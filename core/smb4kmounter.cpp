/*
    The core class that mounts the shares.

    SPDX-FileCopyrightText: 2003-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Application specific includes
#include "smb4kmounter.h"
#include "smb4kbookmark.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kcredentialsmanager.h"
#include "smb4kcustomsettings.h"
#include "smb4kcustomsettingsmanager.h"
#include "smb4khardwareinterface.h"
#include "smb4khomesshareshandler.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QApplication>
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
#include <QApplicationStatic>
#else
#include <qapplicationstatic.h>
#endif
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHostInfo>
#include <QPointer>
#include <QStorageInfo>
#include <QTimer>
#include <QUdpSocket>

// KDE includes
#include <KAuth/ExecuteJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMountPoint>
#include <KShell>
#include <KUser>
#include <kauth_version.h>

using namespace Smb4KGlobal;

#define TIMEOUT 50

class Smb4KMounterPrivate
{
public:
    int remountTimeout;
    int remountAttempts;
    int timerId;
    int checkTimeout;
    QList<SharePtr> newlyMounted;
    QList<SharePtr> newlyUnmounted;
    QList<SharePtr> retries;
    QList<SharePtr> remounts;
    bool detectAllShares;
    bool longActionRunning;
    QStorageInfo storageInfo;
};

class Smb4KMounterStatic
{
public:
    Smb4KMounter instance;
};

Q_APPLICATION_STATIC(Smb4KMounterStatic, p);

Smb4KMounter::Smb4KMounter(QObject *parent)
    : KCompositeJob(parent)
    , d(new Smb4KMounterPrivate)
{
    setAutoDelete(false);

    d->timerId = -1;
    d->remountTimeout = 0;
    d->remountAttempts = 0;
    d->checkTimeout = 0;
    d->longActionRunning = false;
    d->detectAllShares = Smb4KMountSettings::detectAllShares();

    //
    // Connections
    //
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::aboutToChangeProfile, this, &Smb4KMounter::slotAboutToChangeProfile);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::activeProfileChanged, this, &Smb4KMounter::slotActiveProfileChanged);
    connect(Smb4KCredentialsManager::self(), &Smb4KCredentialsManager::credentialsUpdated, this, &Smb4KMounter::slotCredentialsUpdated);
    connect(Smb4KMountSettings::self(), &Smb4KMountSettings::configChanged, this, &Smb4KMounter::slotConfigChanged);

    connect(Smb4KHardwareInterface::self(), &Smb4KHardwareInterface::onlineStateChanged, this, &Smb4KMounter::slotOnlineStateChanged);
    connect(Smb4KHardwareInterface::self(), &Smb4KHardwareInterface::networkShareAdded, this, &Smb4KMounter::slotShareMounted);
    connect(Smb4KHardwareInterface::self(), &Smb4KHardwareInterface::networkShareRemoved, this, &Smb4KMounter::slotShareUnmounted);

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Smb4KMounter::slotAboutToQuit);
}

Smb4KMounter::~Smb4KMounter()
{
    while (!d->newlyMounted.isEmpty()) {
        d->newlyMounted.takeFirst().clear();
    }

    while (!d->newlyUnmounted.isEmpty()) {
        d->newlyUnmounted.takeFirst().clear();
    }

    while (!d->retries.isEmpty()) {
        d->retries.takeFirst().clear();
    }

    while (!d->remounts.isEmpty()) {
        d->remounts.takeFirst().clear();
    }
}

Smb4KMounter *Smb4KMounter::self()
{
    return &p->instance;
}

void Smb4KMounter::abort()
{
    if (!QCoreApplication::closingDown()) {
        QListIterator<KJob *> it(subjobs());

        while (it.hasNext()) {
            it.next()->kill(KJob::EmitResult);
        }
    }
}

bool Smb4KMounter::isRunning()
{
    return (hasSubjobs() || d->longActionRunning);
}

void Smb4KMounter::triggerRemounts(bool fillList)
{
    if (d->remounts.isEmpty() && !fillList) {
        return;
    }

    if (fillList) {
        QList<CustomSettingsPtr> options = Smb4KCustomSettingsManager::self()->sharesToRemount();

        for (const CustomSettingsPtr &option : std::as_const(options)) {
            if (option->remount() == Smb4KCustomSettings::RemountOnce && !Smb4KMountSettings::remountShares()) {
                continue;
            }

            QList<SharePtr> mountedShares = findShareByUrl(option->url());
            bool remountShare = true;

            for (const SharePtr &share : std::as_const(mountedShares)) {
                if (!share->isForeign()) {
                    remountShare = false;
                    break;
                }
            }

            if (remountShare) {
                bool insertShare = true;

                for (const SharePtr &share : std::as_const(d->remounts)) {
                    if (QString::compare(share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                         option->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort))
                        == 0) {
                        insertShare = false;
                        break;
                    }
                }

                if (insertShare) {
                    SharePtr share = SharePtr(new Smb4KShare());
                    share->setUrl(option->url());
                    share->setWorkgroupName(option->workgroupName());
                    share->setHostIpAddress(option->ipAddress());

                    if (share->url().isValid() && !share->url().isEmpty()) {
                        d->remounts << share;
                    }
                }
            }
        }
    }

    mountShares(d->remounts);
    d->remountAttempts++;
}

void Smb4KMounter::mountShare(const SharePtr &share)
{
    if (share) {
        //
        // Check that the URL is valid
        //
        if (!share->url().isValid()) {
            Smb4KNotification::invalidURLPassed();
            return;
        }

        //
        // Check if the share has already been mounted. If it is already present,
        // do not process it and return.
        //
        QUrl url;

        if (share->isHomesShare()) {
            url = share->homeUrl();
        } else {
            url = share->url();
        }

        QList<SharePtr> mountedShares = findShareByUrl(url);
        bool isMounted = false;

        for (const SharePtr &s : std::as_const(mountedShares)) {
            if (!s->isForeign()) {
                isMounted = true;
                break;
            }
        }

        if (isMounted) {
            return;
        }

        //
        // Wake-On-LAN: Wake up the host before mounting
        //
        if (Smb4KSettings::enableWakeOnLAN()) {
            CustomSettingsPtr customSettings = Smb4KCustomSettingsManager::self()->findCustomSettings(share->url().resolved(QUrl(QStringLiteral(".."))));

            if (customSettings && customSettings->wakeOnLanSendBeforeMount()) {
                Q_EMIT aboutToStart(WakeUp);

                QUdpSocket *socket = new QUdpSocket(this);
                QHostAddress addr;

                // Use the host's IP address directly from the share object.
                if (share->hasHostIpAddress()) {
                    addr.setAddress(share->hostIpAddress());
                } else {
                    addr.setAddress(QStringLiteral("255.255.255.255"));
                }

                // Construct magic sequence
                QByteArray sequence;

                // 6 times 0xFF
                for (int j = 0; j < 6; ++j) {
                    sequence.append(QChar(0xFF).toLatin1());
                }

                // 16 times the MAC address
                QStringList parts = customSettings->macAddress().split(QStringLiteral(":"), Qt::SkipEmptyParts);

                for (int j = 0; j < 16; ++j) {
                    for (int k = 0; k < parts.size(); ++k) {
                        QString item = QStringLiteral("0x") + parts.at(k);
                        sequence.append(QChar(item.toInt(nullptr, 16)).toLatin1());
                    }
                }

                socket->writeDatagram(sequence, addr, 9);

                delete socket;

                // Wait the defined time
                int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
                int i = 0;

                while (i++ < stop) {
                    wait(250);
                }

                Q_EMIT finished(WakeUp);
            }
        }

        //
        // Create the mountpoint
        //
        QString mountpoint;
        mountpoint += Smb4KMountSettings::mountPrefix().path();
        mountpoint += QDir::separator();
        mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
        mountpoint += QDir::separator();

        if (!share->isHomesShare()) {
            mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
        } else {
            mountpoint += (Smb4KMountSettings::forceLowerCaseSubdirs() ? share->userName().toLower() : share->userName());
        }

        // Get the permissions that should be used for creating the
        // mount prefix and all its subdirectories.
        // Please note that the actual permissions of the mount points
        // are determined by the mount utility.
        QFile::Permissions permissions;
        QUrl parentDirectory;

        if (QFile::exists(Smb4KMountSettings::mountPrefix().path())) {
            parentDirectory = Smb4KMountSettings::mountPrefix();
        } else {
            QUrl u = Smb4KMountSettings::mountPrefix();
            parentDirectory = u.resolved(QUrl(QStringLiteral("..")));
            ;
        }

        QFile f(parentDirectory.path());
        permissions = f.permissions();

        QDir dir(mountpoint);

        if (!dir.mkpath(dir.path())) {
            share->setPath(QStringLiteral(""));
            Smb4KNotification::mkdirFailed(dir);
            return;
        } else {
            QUrl u = QUrl::fromLocalFile(dir.path());

            while (!parentDirectory.matches(u, QUrl::StripTrailingSlash)) {
                QFile(u.path()).setPermissions(permissions);
                u = u.resolved(QUrl(QStringLiteral("..")));
            }
        }

        share->setPath(QDir::cleanPath(mountpoint));

        //
        // Get the authentication information
        //
        Smb4KCredentialsManager::self()->readLoginCredentials(share);

        //
        // Mount arguments
        //
        QVariantMap args;

        if (!fillMountActionArgs(share, args)) {
            return;
        }

        //
        // Emit the aboutToStart() signal
        //
        Q_EMIT aboutToStart(MountShare);

        //
        // Create the mount action
        //
        KAuth::Action mountAction(QStringLiteral("org.kde.smb4k.mounthelper.mount"));
        mountAction.setHelperId(QStringLiteral("org.kde.smb4k.mounthelper"));
        mountAction.setArguments(args);

        KAuth::ExecuteJob *job = mountAction.execute();

        //
        // Modify the cursor, if necessary.
        //
        if (!hasSubjobs()) {
            QApplication::setOverrideCursor(Qt::BusyCursor);
        }

        //
        // Add the job
        //
        addSubjob(job);

        //
        // Start the job and process the returned result.
        //
        bool success = job->exec();

        if (success) {
            int errorCode = job->error();

            if (errorCode == 0) {
                // Get the error message
                QString errorMsg = job->data().value(QStringLiteral("mh_error_message")).toString();

                if (!errorMsg.isEmpty()) {
#if defined(Q_OS_LINUX)
                    if (errorMsg.contains(QStringLiteral("mount error 13"))
                        || errorMsg.contains(QStringLiteral("mount error(13)")) /* authentication error */) {
                        d->retries << share;
                        Q_EMIT requestCredentials(share);
                    } else if (errorMsg.contains(QStringLiteral("Unable to find suitable address."))) {
                        // Swallow this
                    } else {
                        Smb4KNotification::mountingFailed(share, errorMsg);
                    }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
                    if (errorMsg.contains(QStringLiteral("Authentication error")) || errorMsg.contains(QStringLiteral("Permission denied"))) {
                        d->retries << share;
                        Q_EMIT requestCredentials(share);
                    } else {
                        Smb4KNotification::mountingFailed(share, errorMsg);
                    }
#else
                    qWarning() << "Smb4KMounter::slotMountJobFinished(): Error handling not implemented!";
                    Smb4KNotification::mountingFailed(share, errorMsg);
#endif
                }
            } else {
                Smb4KNotification::actionFailed(errorCode);
            }
        } else {
            // FIXME: Report that the action could not be started
        }

        //
        // Remove the job from the job list
        //
        removeSubjob(job);

        //
        // Reset the busy cursor
        //
        if (!hasSubjobs()) {
            QApplication::restoreOverrideCursor();
        }

        //
        // Emit the finished() signal
        //
        Q_EMIT finished(MountShare);
    }
}

void Smb4KMounter::mountShares(const QList<SharePtr> &shares)
{
    d->longActionRunning = true;

    for (const SharePtr &share : shares) {
        mountShare(share);
    }

    d->longActionRunning = false;

    if (Smb4KHardwareInterface::self()->initialImportDone()) {
        if (d->newlyMounted.size() > 1) {
            Smb4KNotification::sharesMounted(d->newlyMounted.size());
        } else {
            if (d->newlyMounted.size() == 1) {
                Smb4KNotification::shareMounted(d->newlyMounted.first());
            }
        }
    }

    while (!d->newlyMounted.isEmpty()) {
        d->newlyMounted.takeFirst().clear();
    }
}

void Smb4KMounter::unmountShare(const SharePtr &share, bool silent)
{
    Q_ASSERT(share);

    if (share) {
        //
        // Check that the URL is valid.
        //
        if (!share->url().isValid()) {
            Smb4KNotification::invalidURLPassed();
            return;
        }

        //
        // Handle foreign shares according to the settings
        //
        if (share->isForeign()) {
            if (!Smb4KMountSettings::unmountForeignShares()) {
                if (!silent) {
                    Smb4KNotification::unmountingNotAllowed(share);
                }

                return;
            } else {
                if (!silent) {
                    if (KMessageBox::warningTwoActions(QApplication::activeWindow(),
                                                       i18n("<p>The share <b>%1</b> is mounted to <br><b>%2</b> and owned by user <b>%3</b>.</p>"
                                                            "<p>Do you really want to unmount it?</p>",
                                                            share->displayString(),
                                                            share->path(),
                                                            share->user().loginName()),
                                                       i18n("Foreign Share"),
                                                       KStandardGuiItem::ok(),
                                                       KStandardGuiItem::cancel())
                        == KMessageBox::SecondaryAction) {
                        return;
                    }
                } else {
                    // Without the confirmation of the user, we are not
                    // unmounting a foreign share!
                    return;
                }
            }
        }

        //
        // Force the unmounting of the share either if the system went offline
        // or if the user chose to forcibly unmount inaccessible shares (Linux only).
        //
        bool force = false;

        if (Smb4KHardwareInterface::self()->isOnline()) {
#if defined(Q_OS_LINUX)
            if (share->isInaccessible()) {
                force = Smb4KMountSettings::forceUnmountInaccessible();
            }
#endif
        } else {
            force = true;
        }

        //
        // Unmount arguments
        //
        QVariantMap args;

        if (!fillUnmountActionArgs(share, force, silent, args)) {
            return;
        }

        //
        // Emit the aboutToStart() signal
        //
        Q_EMIT aboutToStart(UnmountShare);

        //
        // Create the unmount action
        //
        KAuth::Action unmountAction(QStringLiteral("org.kde.smb4k.mounthelper.unmount"));
        unmountAction.setHelperId(QStringLiteral("org.kde.smb4k.mounthelper"));
        unmountAction.setArguments(args);

        KAuth::ExecuteJob *job = unmountAction.execute();

        //
        // Modify the cursor, if necessary.
        //
        if (!hasSubjobs()) {
            QApplication::setOverrideCursor(Qt::BusyCursor);
        }

        //
        // Add the job
        //
        addSubjob(job);

        //
        // Start the job and process the returned result.
        //
        bool success = job->exec();

        if (success) {
            int errorCode = job->error();

            if (errorCode == 0) {
                // Get the error message
                QString errorMsg = job->data().value(QStringLiteral("mh_error_message")).toString();

                if (!errorMsg.isEmpty()) {
                    // No error handling needed, just report the error message.
                    Smb4KNotification::unmountingFailed(share, errorMsg);
                }
            } else {
                Smb4KNotification::actionFailed(errorCode);
            }
        } else {
            // FIXME: Report that the action could not be started
        }

        //
        // Remove the job from the job list
        //
        removeSubjob(job);

        //
        // Reset the busy cursor
        //
        if (!hasSubjobs()) {
            QApplication::restoreOverrideCursor();
        }

        //
        // Emit the finished() signal
        //
        Q_EMIT finished(UnmountShare);
    }
}

void Smb4KMounter::unmountShares(const QList<SharePtr> &shares, bool silent)
{
    d->longActionRunning = true;

    Smb4KHardwareInterface::self()->inhibit();

    for (const SharePtr &share : shares) {
        unmountShare(share, silent);
    }

    Smb4KHardwareInterface::self()->uninhibit();

    d->longActionRunning = false;

    if (d->newlyUnmounted.size() > 1) {
        Smb4KNotification::sharesUnmounted(d->newlyUnmounted.size());
    } else {
        if (d->newlyUnmounted.size() == 1) {
            Smb4KNotification::shareUnmounted(shares.first());
        }
    }

    while (!d->newlyUnmounted.isEmpty()) {
        d->newlyUnmounted.takeFirst().clear();
    }
}

void Smb4KMounter::unmountAllShares(bool silent)
{
    unmountShares(mountedSharesList(), silent);
}

void Smb4KMounter::start()
{
    if (Smb4KHardwareInterface::self()->isOnline()) {
        QTimer::singleShot(50, this, SLOT(slotStartJobs()));
    }
}

void Smb4KMounter::saveSharesForRemount()
{
    for (const SharePtr &share : mountedSharesList()) {
        if (!share->isForeign()) {
            Smb4KCustomSettingsManager::self()->addRemount(share, false);
        } else {
            Smb4KCustomSettingsManager::self()->removeRemount(share, false);
        }
    }

    while (!d->remounts.isEmpty()) {
        SharePtr share = d->remounts.takeFirst();
        Smb4KCustomSettingsManager::self()->addRemount(share, false);
        share.clear();
    }
}

void Smb4KMounter::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (!isRunning() && Smb4KHardwareInterface::self()->isOnline()) {
        //
        // Try to remount shares
        //
        if (d->remountAttempts < Smb4KMountSettings::remountAttempts() && Smb4KHardwareInterface::self()->initialImportDone()) {
            if (d->remountAttempts == 0) {
                triggerRemounts(true);
            }

            if ((60000 * Smb4KMountSettings::remountInterval()) < d->remountTimeout) {
                triggerRemounts(false);
                d->remountTimeout = -TIMEOUT;
            }

            d->remountTimeout += TIMEOUT;
        }

        //
        // Check the size, accessibility, etc. of the shares
        //
        if (d->checkTimeout >= 2500) {
            for (const SharePtr &share : mountedSharesList()) {
                check(share);
                Q_EMIT updated(share);
            }

            d->checkTimeout = 0;
        } else {
            d->checkTimeout += TIMEOUT;
        }
    }
}

#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &share, QVariantMap &map)
{
    //
    // Find the mount executable
    //
    const QString mount = findMountExecutable();

    if (!mount.isEmpty()) {
        map.insert(QStringLiteral("mh_command"), mount);
    } else {
        Smb4KNotification::commandNotFound(QStringLiteral("mount.cifs"));
        return false;
    }

    //
    // Global and custom options
    //
    CustomSettingsPtr options = Smb4KCustomSettingsManager::self()->findCustomSettings(share);

    //
    // Pass the remote file system port to the URL
    //
    if (options) {
        if (options->useFileSystemPort()) {
            share->setPort(options->fileSystemPort());
        }
    } else {
        if (Smb4KMountSettings::useRemoteFileSystemPort()) {
            share->setPort(Smb4KMountSettings::remoteFileSystemPort());
        }
    }

    //
    // List of arguments passed via "-o ..." to the mount command
    //
    QStringList argumentsList;

    //
    // Workgroup or domain
    //
    // Do not use this, if the domain is a DNS domain.
    //
    WorkgroupPtr workgroup = findWorkgroup(share->workgroupName());

    if ((workgroup && !workgroup->dnsDiscovered()) || (!workgroup && !share->workgroupName().trimmed().isEmpty())) {
        argumentsList << QStringLiteral("domain=") + KShell::quoteArg(share->workgroupName());
    }

    //
    // Host IP address
    //
    if (share->hasHostIpAddress()) {
        argumentsList << QStringLiteral("ip=") + share->hostIpAddress();
    }

    //
    // User name (login)
    //
    if (!share->userName().isEmpty()) {
        argumentsList << QStringLiteral("username=") + share->userName();
    } else {
        argumentsList << QStringLiteral("guest");
    }

    //
    // Client's and server's NetBIOS name
    //
    // According to the manual page, this is only needed when port 139
    // is used. So, we only pass the NetBIOS name in that case.
    //
    bool setNetbiosNames = false;

    if (options) {
        setNetbiosNames = (options->useFileSystemPort() && options->fileSystemPort() == 139);
    } else {
        setNetbiosNames = (Smb4KMountSettings::useRemoteFileSystemPort() && Smb4KMountSettings::remoteFileSystemPort() == 139);
    }

    if (setNetbiosNames) {
        // The client's NetBIOS name. If that is empty, fall back to the local host name.
        if (!machineNetbiosName().isEmpty()) {
            argumentsList << QStringLiteral("netbiosname=") + KShell::quoteArg(machineNetbiosName());
        } else {
            argumentsList << QStringLiteral("netbiosname=") + KShell::quoteArg(QHostInfo::localHostName());
        }

        // The server's NetBIOS name
        argumentsList << QStringLiteral("servern=") + KShell::quoteArg(share->hostName());
    }

    //
    // CIFS Unix extensions support
    //
    // This sets the uid, gid, file_mode and dir_mode arguments, if necessary.
    //
    bool useCifsUnixExtensionsSupport = false;
    QString userString, groupString, fileModeString, directoryModeString;

    if (options) {
        useCifsUnixExtensionsSupport = options->cifsUnixExtensionsSupport();
        userString = options->useUser() ? options->user().userId().toString() : QString();
        groupString = options->useGroup() ? options->group().groupId().toString() : QString();
        fileModeString = options->useFileMode() ? options->fileMode() : QString();
        directoryModeString = options->useDirectoryMode() ? options->directoryMode() : QString();
    } else {
        useCifsUnixExtensionsSupport = Smb4KMountSettings::cifsUnixExtensionsSupport();
        userString = Smb4KMountSettings::useUserId() ? Smb4KMountSettings::userId() : QString();
        groupString = Smb4KMountSettings::useGroupId() ? Smb4KMountSettings::groupId() : QString();
        fileModeString = Smb4KMountSettings::useFileMode() ? Smb4KMountSettings::fileMode() : QString();
        directoryModeString = Smb4KMountSettings::useDirectoryMode() ? Smb4KMountSettings::directoryMode() : QString();
    }

    if (!useCifsUnixExtensionsSupport) {
        // User id
        if (!userString.isEmpty()) {
            argumentsList << QStringLiteral("uid=") + userString;
        }

        // Group id
        if (!groupString.isEmpty()) {
            argumentsList << QStringLiteral("gid=") + groupString;
        }

        // File mode
        if (!fileModeString.isEmpty()) {
            argumentsList << QStringLiteral("file_mode=") + fileModeString;
        }

        // Directory mode
        if (!directoryModeString.isEmpty()) {
            argumentsList << QStringLiteral("dir_mode=") + directoryModeString;
        }
    }

    //
    // Force user id
    //
    // FIXME: The manual page is not clear about this: Is this option only useful when the uid=...
    // argument is given? If so, this should be moved into the 'User id' code block above.
    //
    if (Smb4KMountSettings::forceUID()) {
        argumentsList << QStringLiteral("forceuid");
    }

    //
    // Force group id
    //
    // FIXME: The manual page is not clear about this: Is this option only useful when the gid=...
    // argument is given? If so, this should be moved into the 'Group id' code block above.
    //
    if (Smb4KMountSettings::forceGID()) {
        argumentsList << QStringLiteral("forcegid");
    }

    //
    // Client character set
    //
    if (Smb4KMountSettings::useClientCharset()) {
        switch (Smb4KMountSettings::clientCharset()) {
        case Smb4KMountSettings::EnumClientCharset::default_charset: {
            break;
        }
        default: {
            argumentsList << QStringLiteral("iocharset=")
                    + Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label;
            break;
        }
        }
    }

    //
    // File system port
    //
    if (options) {
        if (options->useFileSystemPort()) {
            argumentsList << QStringLiteral("port=") + QString::number(options->fileSystemPort());
        }
    } else {
        if (Smb4KMountSettings::useRemoteFileSystemPort()) {
            argumentsList << QStringLiteral("port=") + QString::number(Smb4KMountSettings::remoteFileSystemPort());
        }
    }

    //
    // Write access
    //
    bool useWriteAccess = false;
    int writeAccess = -1;

    if (options) {
        useWriteAccess = options->useWriteAccess();
        writeAccess = options->writeAccess();
    } else {
        useWriteAccess = Smb4KMountSettings::useWriteAccess();
        writeAccess = Smb4KMountSettings::writeAccess();
    }

    if (useWriteAccess) {
        switch (writeAccess) {
        case Smb4KMountSettings::EnumWriteAccess::ReadWrite: {
            argumentsList << QStringLiteral("rw");
            break;
        }
        case Smb4KMountSettings::EnumWriteAccess::ReadOnly: {
            argumentsList << QStringLiteral("ro");
            break;
        }
        default: {
            break;
        }
        }
    }

    //
    // Permission checks
    //
    if (Smb4KMountSettings::permissionChecks()) {
        argumentsList << QStringLiteral("perm");
    } else {
        argumentsList << QStringLiteral("noperm");
    }

    //
    // Client controls ids
    //
    if (Smb4KMountSettings::clientControlsIDs()) {
        argumentsList << QStringLiteral("setuids");
    } else {
        argumentsList << QStringLiteral("nosetuids");
    }

    //
    // Server inode numbers
    //
    if (Smb4KMountSettings::serverInodeNumbers()) {
        argumentsList << QStringLiteral("serverino");
    } else {
        argumentsList << QStringLiteral("noserverino");
    }

    //
    // Cache mode
    //
    if (Smb4KMountSettings::useCacheMode()) {
        switch (Smb4KMountSettings::cacheMode()) {
        case Smb4KMountSettings::EnumCacheMode::None: {
            argumentsList << QStringLiteral("cache=none");
            break;
        }
        case Smb4KMountSettings::EnumCacheMode::Strict: {
            argumentsList << QStringLiteral("cache=strict");
            break;
        }
        case Smb4KMountSettings::EnumCacheMode::Loose: {
            argumentsList << QStringLiteral("cache=loose");
            break;
        }
        default: {
            break;
        }
        }
    }

    //
    // Translate reserved characters
    //
    if (Smb4KMountSettings::translateReservedChars()) {
        argumentsList << QStringLiteral("mapchars");
    } else {
        argumentsList << QStringLiteral("nomapchars");
    }

    //
    // Locking
    //
    if (Smb4KMountSettings::noLocking()) {
        argumentsList << QStringLiteral("nobrl");
    }

    //
    // Security mode
    //
    bool useSecurityMode = false;
    int securityMode = -1;

    if (options) {
        useSecurityMode = options->useSecurityMode();
        securityMode = options->securityMode();
    } else {
        useSecurityMode = Smb4KMountSettings::useSecurityMode();
        securityMode = Smb4KMountSettings::securityMode();
    }

    if (useSecurityMode) {
        switch (securityMode) {
        case Smb4KMountSettings::EnumSecurityMode::None: {
            argumentsList << QStringLiteral("sec=none");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5: {
            argumentsList << QStringLiteral("sec=krb5");
            argumentsList << QStringLiteral("cruid=") + KUser(KUser::UseRealUserID).userId().toString();
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Krb5i: {
            argumentsList << QStringLiteral("sec=krb5i");
            argumentsList << QStringLiteral("cruid=") + KUser(KUser::UseRealUserID).userId().toString();
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlm: {
            argumentsList << QStringLiteral("sec=ntlm");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmi: {
            argumentsList << QStringLiteral("sec=ntlmi");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2: {
            argumentsList << QStringLiteral("sec=ntlmv2");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmv2i: {
            argumentsList << QStringLiteral("sec=ntlmv2i");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmssp: {
            argumentsList << QStringLiteral("sec=ntlmssp");
            break;
        }
        case Smb4KMountSettings::EnumSecurityMode::Ntlmsspi: {
            argumentsList << QStringLiteral("sec=ntlmsspi");
            break;
        }
        default: {
            // Smb4KSettings::EnumSecurityMode::Default,
            break;
        }
        }
    }

    //
    // SMB protocol version
    //
    bool useMountProtocolVersion = false;
    int mountProtocolVersion = -1;

    if (options) {
        useMountProtocolVersion = options->useMountProtocolVersion();
        mountProtocolVersion = options->mountProtocolVersion();
    } else {
        useMountProtocolVersion = Smb4KMountSettings::useSmbProtocolVersion();
        mountProtocolVersion = Smb4KMountSettings::smbProtocolVersion();
    }

    if (useMountProtocolVersion) {
        switch (mountProtocolVersion) {
        case Smb4KMountSettings::EnumSmbProtocolVersion::OnePointZero: {
            argumentsList << QStringLiteral("vers=1.0");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointZero: {
            argumentsList << QStringLiteral("vers=2.0");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::TwoPointOne: {
            argumentsList << QStringLiteral("vers=2.1");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZero: {
            argumentsList << QStringLiteral("vers=3.0");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointZeroPointTwo: {
            argumentsList << QStringLiteral("vers=3.0.2");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::ThreePointOnePointOne: {
            argumentsList << QStringLiteral("vers=3.1.1");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::ThreeAndAbove: {
            argumentsList << QStringLiteral("vers=3");
            break;
        }
        case Smb4KMountSettings::EnumSmbProtocolVersion::Default: {
            argumentsList << QStringLiteral("vers=default");
            break;
        }
        default: {
            break;
        }
        }
    }

    //
    // Mount options provided by the user
    //
    if (Smb4KMountSettings::useCustomCifsOptions()) {
        if (!Smb4KMountSettings::customCIFSOptions().isEmpty()) {
            // SECURITY: Only pass those arguments to mount.cifs that do not pose
            // a potential security risk and that have not already been defined.
            //
            // This is, among others, the proper fix to the security issue reported
            // by Heiner Markert (aka CVE-2014-2581).
            QStringList allowedArgs = allowedMountArguments();
            QStringList list = Smb4KMountSettings::customCIFSOptions().split(QStringLiteral(","), Qt::SkipEmptyParts);
            QMutableStringListIterator it(list);

            while (it.hasNext()) {
                QString arg = it.next().section(QStringLiteral("="), 0, 0);

                if (!allowedArgs.contains(arg)) {
                    it.remove();
                }

                argumentsList += list;
            }
        }
    }

    //
    // Insert the mount options into the map
    //
    QStringList mh_options;
    mh_options << QStringLiteral("-o");
    mh_options << argumentsList.join(QStringLiteral(","));
    map.insert(QStringLiteral("mh_options"), mh_options);

    //
    // Insert the mountpoint into the map
    //
    map.insert(QStringLiteral("mh_mountpoint"), share->canonicalPath());

    //
    // Insert information about the share and its URL into the map
    //
    if (!share->isHomesShare()) {
        map.insert(QStringLiteral("mh_url"), share->url());
    } else {
        map.insert(QStringLiteral("mh_url"), share->homeUrl());
        map.insert(QStringLiteral("mh_homes_url"), share->url());
    }

    //
    // Location of the Kerberos ticket
    //
    // The path to the Kerberos ticket is stored - if it exists - in the
    // KRB5CCNAME environment variable. By default, the ticket is located
    // at /tmp/krb5cc_[uid]. So, if the environment variable does not exist,
    // but the cache file is there, try to use it.
    //
    if (QProcessEnvironment::systemEnvironment().contains(QStringLiteral("KRB5CCNAME"))) {
        map.insert(QStringLiteral("mh_krb5ticket"), QProcessEnvironment::systemEnvironment().value(QStringLiteral("KRB5CCNAME"), QStringLiteral("")));
    } else {
        QString ticket = QStringLiteral("/tmp/krb5cc_") + KUser(KUser::UseRealUserID).userId().toString();

        if (QFile::exists(ticket)) {
            QString fileEntry = QStringLiteral("FILE:") + ticket;
            map.insert(QStringLiteral("mh_krb5ticket"), fileEntry);
        }
    }

    return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &share, QVariantMap &map)
{
    //
    // Find the mount executable
    //
    const QString mount = findMountExecutable();

    if (!mount.isEmpty()) {
        map.insert(QStringLiteral("mh_command"), mount);
    } else {
        Smb4KNotification::commandNotFound(QStringLiteral("mount_smbfs"));
        return false;
    }

    //
    // Global and custom options
    //
    CustomSettingsPtr options = Smb4KCustomSettingsManager::self()->findCustomSettings(share);

    //
    // List of arguments
    //
    QStringList argumentsList;

    //
    // Workgroup or domain
    //
    // Do not use this, if the domain is a DNS domain.
    //
    WorkgroupPtr workgroup = findWorkgroup(share->workgroupName());

    if ((workgroup && !workgroup->dnsDiscovered()) || (!workgroup && !share->workgroupName().trimmed().isEmpty())) {
        argumentsList << QStringLiteral("-W");
        argumentsList << KShell::quoteArg(share->workgroupName());
    }

    //
    // IP address
    //
    if (!share->hostIpAddress().isEmpty()) {
        argumentsList << QStringLiteral("-I");
        argumentsList << share->hostIpAddress();
    }

    //
    // User Id
    //
    if (options) {
        if (options->useUser()) {
            argumentsList << QStringLiteral("-u");
            argumentsList << options->user().userId().toString();
        }
    } else {
        if (Smb4KMountSettings::useUserId()) {
            argumentsList << QStringLiteral("-u");
            argumentsList << Smb4KMountSettings::userId();
        }
    }

    //
    // Group Id
    //
    if (options) {
        if (options->useGroup()) {
            argumentsList << QStringLiteral("-g");
            argumentsList << options->group().groupId().toString();
        }
    } else {
        if (Smb4KMountSettings::useGroupId()) {
            argumentsList << QStringLiteral("-g");
            argumentsList << Smb4KMountSettings::groupId();
        }
    }

    if (Smb4KMountSettings::useCharacterSets()) {
        // Client character set
        QString clientCharset, serverCharset;

        switch (Smb4KMountSettings::clientCharset()) {
        case Smb4KMountSettings::EnumClientCharset::default_charset: {
            break;
        }
        default: {
            clientCharset = Smb4KMountSettings::self()->clientCharsetItem()->choices().value(Smb4KMountSettings::clientCharset()).label;
            break;
        }
        }

        // Server character set
        switch (Smb4KMountSettings::serverCodepage()) {
        case Smb4KMountSettings::EnumServerCodepage::default_codepage: {
            break;
        }
        default: {
            serverCharset = Smb4KMountSettings::self()->serverCodepageItem()->choices().value(Smb4KMountSettings::serverCodepage()).label;
            break;
        }
        }

        if (!clientCharset.isEmpty() && !serverCharset.isEmpty()) {
            argumentsList << QStringLiteral("-E");
            argumentsList << clientCharset + QStringLiteral(":") + serverCharset;
        }
    }

    //
    // File mode
    //
    if (options) {
        if (options->useFileMode()) {
            argumentsList << QStringLiteral("-f");
            argumentsList << options->fileMode();
        }
    } else {
        if (Smb4KMountSettings::useFileMode()) {
            argumentsList << QStringLiteral("-f");
            argumentsList << Smb4KMountSettings::fileMode();
        }
    }

    //
    // Directory mode
    //
    if (options) {
        if (options->useDirectoryMode()) {
            argumentsList << QStringLiteral("-d");
            argumentsList << options->directoryMode();
        }
    } else {
        if (Smb4KMountSettings::useDirectoryMode()) {
            argumentsList << QStringLiteral("-d");
            argumentsList << Smb4KMountSettings::directoryMode();
        }
    }

    //
    // User name (login)
    //
    if (!share->userName().isEmpty()) {
        argumentsList << QStringLiteral("-U");
        argumentsList << share->userName();
    } else {
        argumentsList << QStringLiteral("-N");
    }

    //
    // Insert the mount options into the map
    //
    map.insert(QStringLiteral("mh_options"), argumentsList);

    //
    // Insert the mountpoint into the map
    //
    map.insert(QStringLiteral("mh_mountpoint"), share->canonicalPath());

    //
    // Insert information about the share and its URL into the map
    //
    if (!share->isHomesShare()) {
        map.insert(QStringLiteral("mh_url"), share->url());
    } else {
        map.insert(QStringLiteral("mh_url"), share->homeUrl());
        map.insert(QStringLiteral("mh_homes_url"), share->url());
    }

    return true;
}
#else
//
// Dummy
//
bool Smb4KMounter::fillMountActionArgs(const SharePtr &, QVariantMap &)
{
    qWarning() << "Smb4KMounter::fillMountActionArgs() is not implemented!";
    qWarning() << "Mounting under this operating system is not supported...";
    return false;
}
#endif

#if defined(Q_OS_LINUX)
//
// Linux arguments
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &share, bool force, bool silent, QVariantMap &map)
{
    //
    // The umount program
    //
    const QString umount = findUmountExecutable();

    if (umount.isEmpty() && !silent) {
        Smb4KNotification::commandNotFound(QStringLiteral("umount"));
        return false;
    }

    //
    // The options
    //
    QStringList options;

    if (force) {
        options << QStringLiteral("-l"); // lazy unmount
    }

    //
    // Insert data into the map
    //
    map.insert(QStringLiteral("mh_command"), umount);
    map.insert(QStringLiteral("mh_url"), share->url());

    if (!share->isInaccessible() && Smb4KHardwareInterface::self()->isOnline()) {
        map.insert(QStringLiteral("mh_mountpoint"), share->canonicalPath());
    } else {
        map.insert(QStringLiteral("mh_mountpoint"), share->path());
    }

    map.insert(QStringLiteral("mh_options"), options);

    return true;
}
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
//
// FreeBSD and NetBSD arguments
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &share, bool force, bool silent, QVariantMap &map)
{
    //
    // The umount program
    //
    const QString umount = findUmountExecutable();

    if (umount.isEmpty() && !silent) {
        Smb4KNotification::commandNotFound(QStringLiteral("umount"));
        return false;
    }

    //
    // The options
    //
    QStringList options;

    if (force) {
        options << QStringLiteral("-f");
    }

    //
    // Insert data into the map
    //
    map.insert(QStringLiteral("mh_command"), umount);
    map.insert(QStringLiteral("mh_url"), share->url());

    if (!share->isInaccessible() && Smb4KHardwareInterface::self()->isOnline()) {
        map.insert(QStringLiteral("mh_mountpoint"), share->canonicalPath());
    } else {
        map.insert(QStringLiteral("mh_mountpoint"), share->path());
    }

    map.insert(QStringLiteral("mh_options"), options);

    return true;
}
#else
//
// Dummy
//
bool Smb4KMounter::fillUnmountActionArgs(const SharePtr &, bool, bool, QVariantMap &)
{
    qWarning() << "Smb4KMounter::fillUnmountActionArgs() is not implemented!";
    qWarning() << "Unmounting under this operating system is not supported...";
    return false;
}
#endif

void Smb4KMounter::check(const SharePtr &share)
{
    d->storageInfo.setPath(share->path());

    if (d->storageInfo.isValid() && d->storageInfo.isReady()) {
        // Accessibility
        share->setInaccessible(false);

        // Size information
        share->setFreeDiskSpace(d->storageInfo.bytesAvailable()); // Bytes available to the user, might be less than bytesFree()
        share->setTotalDiskSpace(d->storageInfo.bytesTotal());

        // Get the owner an group, if possible.
        QFileInfo fileInfo(share->path());
        fileInfo.setCaching(false);

        if (fileInfo.exists()) {
            share->setUser(KUser(static_cast<K_UID>(fileInfo.ownerId())));
            share->setGroup(KUserGroup(static_cast<K_GID>(fileInfo.groupId())));
        } else {
            share->setUser(KUser(KUser::UseRealUserID));
            share->setGroup(KUserGroup(KUser::UseRealUserID));
        }
    } else {
        share->setInaccessible(true);
        share->setFreeDiskSpace(0);
        share->setTotalDiskSpace(0);
        share->setUser(KUser(KUser::UseRealUserID));
        share->setGroup(KUserGroup(KUser::UseRealUserID));
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMounter::slotStartJobs()
{
    if (d->timerId == -1) {
        d->timerId = startTimer(TIMEOUT);
    }
}

void Smb4KMounter::slotAboutToQuit()
{
    //
    // Abort any actions
    //
    abort();

    //
    // Check if the user wants to remount shares and save the
    // shares for remount if so.
    //
    if (Smb4KMountSettings::remountShares()) {
        saveSharesForRemount();
    }

    //
    // Unmount the shares if the user chose to do so.
    //
    if (Smb4KMountSettings::unmountSharesOnExit()) {
        unmountAllShares(true);
    }

    //
    // Clean up the mount prefix.
    //
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);

    QDir dir;
    dir.cd(Smb4KMountSettings::mountPrefix().path());
    QStringList hostDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    QStringList mountPointList;

    for (const QString &hostDir : std::as_const(hostDirs)) {
        dir.cd(hostDir);

        QStringList shareDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);

        for (const QString &shareDir : std::as_const(shareDirs)) {
            dir.cd(shareDir);
            mountPointList << dir.absolutePath();
            dir.cdUp();
        }

        dir.cdUp();
    }

    // Remove those mountpoints where a share is actually mounted.
    for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : std::as_const(mountPoints)) {
        mountPointList.removeOne(mountPoint->mountPoint());
    }

    // Remove the empty mountpoints.
    for (const QString &mountPoint : std::as_const(mountPointList)) {
        dir.cd(mountPoint);
        dir.rmdir(dir.canonicalPath());

        if (dir.cdUp()) {
            dir.rmdir(dir.canonicalPath());
        }
    }
}

void Smb4KMounter::slotOnlineStateChanged(bool online)
{
    if (online) {
        slotStartJobs();
    } else {
        abort();
        saveSharesForRemount();

        // FIXME: Do we need this at all?
        for (const SharePtr &share : mountedSharesList()) {
            share->setInaccessible(true);
        }

        unmountAllShares(true);

        d->remountAttempts = 0;
        d->remountTimeout = 0;
    }
}

void Smb4KMounter::slotAboutToChangeProfile()
{
    if (Smb4KMountSettings::remountShares()) {
        saveSharesForRemount();
    }
}

void Smb4KMounter::slotActiveProfileChanged(const QString &newProfile)
{
    Q_UNUSED(newProfile);

    // Stop the timer.
    killTimer(d->timerId);

    abort();

    // Clear all remounts.
    while (!d->remounts.isEmpty()) {
        d->remounts.takeFirst().clear();
    }

    // Clear all retries.
    while (!d->retries.isEmpty()) {
        d->retries.takeFirst().clear();
    }

    // Unmount all shares
    unmountAllShares(true);

    // Reset some variables.
    d->remountTimeout = 0;
    d->remountAttempts = 0;

    // Restart the timer
    d->timerId = startTimer(TIMEOUT);
}

void Smb4KMounter::slotConfigChanged()
{
    if (d->detectAllShares != Smb4KMountSettings::detectAllShares()) {
        d->detectAllShares = Smb4KMountSettings::detectAllShares();

        QListIterator<SharePtr> it(mountedSharesList());

        while (it.hasNext()) {
            SharePtr share = it.next();

            if (share->isForeign() && !Smb4KMountSettings::detectAllShares()) {
                if (removeMountedShare(share, true)) {
                    Q_EMIT unmounted(share);
                    share.clear();
                }
            }
        }

        // Add all shares that need to be added
        QStringList mountPoints = Smb4KHardwareInterface::self()->allMountPoints();

        for (const QString &mountPoint : std::as_const(mountPoints)) {
            if (!findShareByPath(mountPoint)) {
                KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);
                KMountPoint::Ptr mp = mountPoints.findByPath(mountPoint);

                SharePtr share = SharePtr(new Smb4KShare());
                share->setUrl(QUrl(mp->mountedFrom()));
                share->setPath(mp->mountPoint());
                share->setMounted(true);

                QStringList mountOptions = mp->mountOptions();

                for (const QString &option : std::as_const(mountOptions)) {
                    if (option.startsWith(QStringLiteral("domain=")) || option.startsWith(QStringLiteral("workgroup="))) {
                        share->setWorkgroupName(option.section(QStringLiteral("="), 1, 1).trimmed());
                    } else if (option.startsWith(QStringLiteral("addr="))) {
                        share->setHostIpAddress(option.section(QStringLiteral("="), 1, 1).trimmed());
                    } else if (option.startsWith(QStringLiteral("username=")) || option.startsWith(QStringLiteral("user="))) {
                        share->setUserName(option.section(QStringLiteral("="), 1, 1).trimmed());
                    }
                }

                // Work around empty usernames
                if (share->userName().isEmpty()) {
                    share->setUserName(QStringLiteral("guest"));
                }

                check(share);

                if (share->path().startsWith(Smb4KMountSettings::mountPrefix().path())
                    || (!share->isInaccessible() && share->canonicalPath().startsWith(QDir(Smb4KMountSettings::mountPrefix().path()).canonicalPath()))) {
                    share->setForeign(false);
                } else if (share->path().startsWith(QDir::homePath())
                           || (!share->isInaccessible() && share->canonicalPath().startsWith(QDir::home().canonicalPath()))) {
                    share->setForeign(false);
                } else if (share->user().userId() == KUser(KUser::UseRealUserID).userId()
                           && share->group().groupId() == KUserGroup(KUser::UseRealUserID).groupId()) {
                    share->setForeign(false);
                } else {
                    share->setForeign(true);
                }

                if (!share->isForeign() || Smb4KMountSettings::detectAllShares()) {
                    if (addMountedShare(share)) {
                        Q_EMIT mounted(share);
                    }
                }
            }
        }

        Q_EMIT mountedSharesListChanged();
    }
}

void Smb4KMounter::slotCredentialsUpdated(const QUrl &url)
{
    if (!url.isEmpty() && !d->retries.isEmpty()) {
        for (int i = 0; i < d->retries.size(); i++) {
            QUrl parentUrl = d->retries[i]->url().resolved(QUrl(QStringLiteral(".."))).adjusted(QUrl::StripTrailingSlash);

            if (QString::compare(d->retries[i]->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                    == 0
                || QString::compare(parentUrl.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    Qt::CaseInsensitive)
                    == 0) {
                SharePtr share = d->retries.takeAt(i);
                share->setUserName(url.userName());
                share->setPassword(url.password());

                mountShare(share);
            }
        }
    }
}

void Smb4KMounter::slotShareMounted(const QString &mountPoint)
{
    Q_ASSERT(!mountPoint.isEmpty());

    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);
    KMountPoint::Ptr mp = mountPoints.findByPath(mountPoint);

    SharePtr share = SharePtr(new Smb4KShare());
    share->setUrl(QUrl(mp->mountedFrom()));
    share->setPath(mp->mountPoint());
    share->setMounted(true);

    QStringList mountOptions = mp->mountOptions();

    for (const QString &option : std::as_const(mountOptions)) {
        if (option.startsWith(QStringLiteral("domain=")) || option.startsWith(QStringLiteral("workgroup="))) {
            share->setWorkgroupName(option.section(QStringLiteral("="), 1, 1).trimmed());
        } else if (option.startsWith(QStringLiteral("addr="))) {
            share->setHostIpAddress(option.section(QStringLiteral("="), 1, 1).trimmed());
        } else if (option.startsWith(QStringLiteral("username=")) || option.startsWith(QStringLiteral("user="))) {
            share->setUserName(option.section(QStringLiteral("="), 1, 1).trimmed());
        }
    }

    // Work around empty usernames
    if (share->userName().isEmpty()) {
        share->setUserName(QStringLiteral("guest"));
    }

    check(share);

    if (share->path().startsWith(Smb4KMountSettings::mountPrefix().path())
        || (!share->isInaccessible() && share->canonicalPath().startsWith(QDir(Smb4KMountSettings::mountPrefix().path()).canonicalPath()))) {
        share->setForeign(false);
    } else if (share->path().startsWith(QDir::homePath()) || (!share->isInaccessible() && share->canonicalPath().startsWith(QDir::home().canonicalPath()))) {
        share->setForeign(false);
    } else if (share->user().userId() == KUser(KUser::UseRealUserID).userId() && share->group().groupId() == KUserGroup(KUser::UseRealUserID).groupId()) {
        share->setForeign(false);
    } else {
        share->setForeign(true);
    }

    if (!share->isForeign() || Smb4KMountSettings::detectAllShares()) {
        if (addMountedShare(share)) {
            // Remove share from the remounts
            QMutableListIterator<SharePtr> s(d->remounts);

            while (s.hasNext()) {
                SharePtr remount = s.next();

                if (!share->isForeign()
                    && QString::compare(remount->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                        share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                        Qt::CaseInsensitive)
                        == 0) {
                    Smb4KCustomSettingsManager::self()->removeRemount(remount);
                    s.remove();
                    break;
                }
            }

            Q_EMIT mounted(share);

            if (d->longActionRunning) {
                d->newlyMounted << share;
                // Notification is handled in Smb4KMounter::mountShares()
            } else {
                if (Smb4KHardwareInterface::self()->initialImportDone()) {
                    Smb4KNotification::shareMounted(share);
                }

                while (!d->newlyMounted.isEmpty()) {
                    d->newlyMounted.takeFirst().clear();
                }
            }

            Q_EMIT mountedSharesListChanged();
        }
    }
}

void Smb4KMounter::slotShareUnmounted(const QString &mountPoint)
{
    Q_ASSERT(!mountPoint.isEmpty());

    SharePtr share = findShareByPath(mountPoint);

    if (!share->isForeign()) {
        QDir dir;
        dir.cd(share->canonicalPath());
        dir.rmdir(dir.canonicalPath());

        if (dir.cdUp()) {
            dir.rmdir(dir.canonicalPath());
        }
    }

    share->setMounted(false);

    if (removeMountedShare(share, d->longActionRunning)) {
        Q_EMIT unmounted(share);
    }

    if (d->longActionRunning) {
        d->newlyUnmounted << share;
        // Notification is handled in Smb4KMounter::unmountShares()
    } else {
        Smb4KNotification::shareUnmounted(share);

        while (!d->newlyUnmounted.isEmpty()) {
            d->newlyUnmounted.takeFirst().clear();
        }
    }

    Q_EMIT mountedSharesListChanged();
}
