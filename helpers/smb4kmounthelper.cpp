/*
    The helper that mounts and unmounts shares.

    SPDX-FileCopyrightText: 2010-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kmounthelper.h"
#include "../core/smb4kglobal.h"

// Qt includes
#include <QDebug>
#include <QDir>
#include <QNetworkInterface>
#include <QProcessEnvironment>
#include <QUrl>

// KDE includes
#include <KAuth/HelperSupport>
#include <KLocalizedString>
#include <KProcess>
#include <KUser>

// system includes
#include <fcntl.h>
#include <sys/stat.h>

using namespace Smb4KGlobal;

KAUTH_HELPER_MAIN("org.kde.smb4k.mounthelper", Smb4KMountHelper);

static const QStringList MOUNT_ARG_WHITELIST{QStringList{
#if defined(Q_OS_LINUX)
    QStringLiteral("domain"),      QStringLiteral("ip"),         QStringLiteral("username"),    QStringLiteral("guest"),
    QStringLiteral("netbiosname"), QStringLiteral("servern"),    QStringLiteral("file_mode"),   QStringLiteral("dir_mode"),
    QStringLiteral("forceuid"),    QStringLiteral("forcegid"),   QStringLiteral("iocharset"),   QStringLiteral("rw"),
    QStringLiteral("ro"),          QStringLiteral("perm"),       QStringLiteral("noperm"),      QStringLiteral("setuids"),
    QStringLiteral("nosetuids"),   QStringLiteral("serverino"),  QStringLiteral("noserverino"), QStringLiteral("cache"),
    QStringLiteral("mapchars"),    QStringLiteral("nomapchars"), QStringLiteral("nobrl"),       QStringLiteral("sec"),
    QStringLiteral("vers")
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QStringLiteral("-E"),
    QStringLiteral("-I"),
    QStringLiteral("-L"),
    QStringLiteral("-N"),
    QStringLiteral("-U"),
    QStringLiteral("-W"),
    QStringLiteral("-f"),
    QStringLiteral("-d")
#endif
}};

static const QStringList UNMOUNT_ARG_WHITELIST{QStringList{
#if defined(Q_OS_LINUX)
    QStringLiteral("-l"),
    QStringLiteral("--lazy")
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QStringLiteral("-f")
#endif
}};

KAuth::ActionReply Smb4KMountHelper::mount(const QVariantMap &args)
{
    ActionReply reply;

    if (!isOnline()) {
        return errorReply(i18n("The computer is not online."));
    }

    QString mountPoint;
    QUrl shareUrl = args[QStringLiteral("mh_url")].toUrl();

    if (auto mp = createMountPoint(shareUrl)) {
        mountPoint = *mp;
    } else {
        return errorReply(i18n("Could not create mount point for share %1.", shareUrl.toDisplayString()));
    }

    const QString mount = findMountExecutable();

    if (mount.isEmpty()) {
        return errorReply(i18n("The mount command could not be found."));
    }

    QStringList mountOptions = args[QStringLiteral("mh_options")].toStringList();

    if (!checkMountArguments(&mountOptions)) {
        return errorReply(i18n("Forbidden mount options were passed."));
    }

    if (args.contains(QStringLiteral("mh_use_ids")) && args[QStringLiteral("mh_use_ids")].toBool()) {
        QString uid = KUser(HelperSupport::callerUid()).userId().toString();
        QString gid = KUser(HelperSupport::callerUid()).groupId().toString();
#if defined(Q_OS_LINUX)
        mountOptions << QStringLiteral("uid=") + uid;
        mountOptions << QStringLiteral("gid=") + gid;
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        mountOptions << QStringLiteral("-u");
        mountOptions << uid;
        mountOptions << QStringLiteral("-g");
        mountOptions << gid;
#endif
    }

    QStringList command;
#if defined(Q_OS_LINUX)
    command << mount;
    command << shareUrl.toString(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::RemovePort);
    command << mountPoint;
    if (!mountOptions.join(QString()).trimmed().isEmpty()) {
        command << QStringLiteral("-o");
        command << mountOptions.join(QStringLiteral(","));
    }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    command << mount;
    if (!mountOptions.join(QString()).trimmed().isEmpty()) {
        command << mountOptions;
    }
    command << shareUrl.toString(QUrl::RemoveScheme | QUrl::RemoveUserInfo | QUrl::RemovePort);
    command << mountPoint;
#endif

    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
#if defined(Q_OS_LINUX)
    proc.setEnv(QStringLiteral("PASSWD"), args[QStringLiteral("mh_url")].toUrl().password(), true);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    // We need this to avoid a translated password prompt.
    proc.setEnv(QStringLiteral("LANG"), QStringLiteral("C"));
#endif
    // If the location of a Kerberos ticket is passed, it needs to
    // be passed to the process environment here.
    if (args.contains(QStringLiteral("mh_krb5ticket"))) {
        auto ticketFd = args[QStringLiteral("mh_krb5ticket")].value<QDBusUnixFileDescriptor>();

        if (!checkFileDescriptor(ticketFd)) {
            return errorReply(i18n("There is something wrong with the provided Kerberos ticket."));
        }

        QString krb5ccFile = QString(QStringLiteral("/proc/self/fd/%1")).arg(ticketFd.fileDescriptor());
        proc.setEnv(QStringLiteral("KRB5CCNAME"), krb5ccFile);
    }

    proc.setProgram(command);
    proc.start();

    if (proc.waitForStarted(-1)) {
        int timeout = 0;

        while (proc.state() != KProcess::NotRunning) {
            // We want to be able to terminate the process from outside.
            // Thus, we implement a loop that checks periodically, if we
            // need to kill the process. It is non-blocking.
            if (HelperSupport::isStopped() || timeout == 30000) {
                proc.kill();
                break;
            }
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
            // Check if there is a password prompt. If there is one, pass
            // the password to it.
            QByteArray out = proc.readAllStandardError();

            if (out.startsWith("Password")) {
                proc.write(args[QStringLiteral("mh_url")].toUrl().password().toUtf8().data());
                proc.write("\r");
            }
#endif
            timeout += 10;
            wait(10);
        }

        if (proc.exitStatus() == KProcess::NormalExit) {
            QString stdErr = QString::fromUtf8(proc.readAllStandardError());
            reply.addData(QStringLiteral("mh_error_message"), stdErr.trimmed());

            if (stdErr.isEmpty()) {
                reply.addData(QStringLiteral("mh_mountpoint"), mountPoint);
            }
        }
    } else {
        return errorReply(i18n("The mount process could not be started."));
    }

    return reply;
}

KAuth::ActionReply Smb4KMountHelper::unmount(const QVariantMap &args)
{
    ActionReply reply;

    QString mountPoint = args[QStringLiteral("mh_mountpoint")].toString();

    if (!isMountPointAllowed(mountPoint)) {
        return errorReply(i18n("The mountpoint %1 is illegal.", args[QStringLiteral("mh_mountpoint")].toString()));
    }

    const QString umount = findUmountExecutable();

    if (umount.isEmpty()) {
        return errorReply(i18n("The umount command could not be found."));
    }

    QStringList unmountOptions = args[QStringLiteral("mh_options")].toStringList();

    if (!checkUnmountArguments(&unmountOptions)) {
        return errorReply(i18n("Forbidden unmount options were passed."));
    }

    QStringList command;
    command << umount;
    command << unmountOptions;
    command << QDir(mountPoint).canonicalPath();

    KProcess proc(this);
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    proc.setProgram(command);

    // Depending on the online state, use a different behavior for unmounting.
    //
    // Extensive tests have shown that - when offline - unmounting does not
    // work properly when the process is not detached. Thus, detach it when
    // the system is offline.
    if (isOnline()) {
        proc.start();

        if (proc.waitForStarted(-1)) {
            // We want to be able to terminate the process from outside.
            // Thus, we implement a loop that checks periodically, if we
            // need to kill the process. It is non-blocking.
            int timeout = 0;

            while (proc.state() != KProcess::NotRunning) {
                if (HelperSupport::isStopped() || timeout == 30000) {
                    proc.kill();
                    break;
                }

                timeout += 10;
                wait(10);
            }

            if (proc.exitStatus() == KProcess::NormalExit) {
                QString stdErr = QString::fromUtf8(proc.readAllStandardError());
                reply.addData(QStringLiteral("mh_error_message"), stdErr.trimmed());
            }
        } else {
            return errorReply(i18n("The unmount process could not be started."));
        }
    } else {
        proc.startDetached();
    }

    removeMountPoint(QDir(mountPoint).canonicalPath());

    return reply;
}

bool Smb4KMountHelper::isOnline() const
{
    // FIXME: Do not allow virtual networks
    bool online = false;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : std::as_const(interfaces)) {
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning) {
            online = true;
            break;
        }
    }

    return online;
}

bool Smb4KMountHelper::checkMountArguments(QStringList *argList) const
{
    QStringListIterator it(*argList);

    while (it.hasNext()) {
        QString entry = it.next();
#if defined(Q_OS_LINUX)
        QString arg;

        // Do not allow commata in an entry. We do not want a
        // malicious user to be able to inject arbitrary options.
        if (entry.contains(QStringLiteral(","))) {
            return false;
        }

        if (entry.contains(QStringLiteral("="))) {
            arg = entry.section(QStringLiteral("="), 1, 1);
            entry = entry.section(QStringLiteral("="), 0, 0);
        }

        // If the option is not whitelisted, return here
        if (!MOUNT_ARG_WHITELIST.contains(entry)) {
            return false;
        }

        // Do not allow setuid root bits or the like
        if (entry == QStringLiteral("file_mode") || entry == QStringLiteral("dir_mode")) {
            // We only take an octal number with 3 or 4 digits
            if (arg.size() < 3 || arg.size() > 4) {
                return false;
            }

            // If the argument is not an octal number, return here
            bool ok = false;
            (void)arg.toInt(&ok, 8);

            if (!ok) {
                return false;
            }

            // We only accept a '0' as first digit in a 4 digit octal number
            if (arg.size() == 4 && arg[0].toLatin1() != '0') {
                return false;
            }
        }
#elif defined(Q_OS_FREEBSD) | defined(Q_OS_NETBSD)
        if (!entry.startsWith(QStringLiteral("-"))) {
            // These can only be arguments of the options
            continue;
        }

        // If the option is not whitelisted, return here
        if (!MOUNT_ARG_WHITELIST.contains(entry)) {
            return false;
        }

        // Do not allow setuid root bits or the like
        if (entry == QStringLiteral("-f") || entry == QStringLiteral("-d")) {
            QString arg = it.peekNext();

            // We only take an octal number with 3 or 4 digits
            if (arg.size() < 3 || arg.size() > 4) {
                return false;
            }

            // If the argument is not an octal number, return here
            bool ok = false;
            (void)arg.toInt(&ok, 8);

            if (!ok) {
                return false;
            }

            // We only accept a '0' as first digit in a 4 digit octal number
            if (arg.size() == 4 && arg[0].toLatin1() != '0') {
                return false;
            }
        }
#endif
    }

    return true;
}

bool Smb4KMountHelper::checkUnmountArguments(QStringList *argList) const
{
    QStringListIterator it(*argList);

    while (it.hasNext()) {
        QString entry = it.next();

        if (!UNMOUNT_ARG_WHITELIST.contains(entry)) {
            return false;
        }
    }

    return true;
}

QString Smb4KMountHelper::mountPrefix() const
{
    return (QDir::cleanPath(QStringLiteral("/var/run")) + QDir::separator() + QStringLiteral("smb4k"));
}

ActionReply Smb4KMountHelper::errorReply(const QString &message) const
{
    ActionReply reply = ActionReply::HelperErrorReply();
    reply.setErrorDescription(message);
    return reply;
}

std::optional<QString> Smb4KMountHelper::createMountPoint(const QUrl &url) const
{
    QDir dir(mountPrefix());

    if (!dir.exists()) {
        if (!dir.mkdir(dir.path(),
                       QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup | QFileDevice::ExeGroup
                           | QFileDevice::ReadOther | QFileDevice::ExeOther)) {
            return std::nullopt;
        }
    }

    // Check that the other path components are valid.
    // NOTE: The host component can not contain any slash characters that would
    // convert into path delimiters. This is ensured by the QUrl implementation.
    QString hostComponent = url.host();
    QString pathComponent =
        (url.adjusted(QUrl::StripTrailingSlash).path().startsWith(QDir::separator()) ? url.adjusted(QUrl::StripTrailingSlash).path().removeFirst()
                                                                                     : url.adjusted(QUrl::StripTrailingSlash).path());

    // Do not allow an empty host name
    if (hostComponent.isEmpty()) {
        return std::nullopt;
    }

    // Do not allow empty paths, paths containing any '..' or that have more
    // than one level (= contains slash characters).
    if (pathComponent.isEmpty() || pathComponent.contains(QStringLiteral("..")) || pathComponent.contains(QDir::separator())) {
        return std::nullopt;
    }

    QStringList pathComponents;
    pathComponents << KUser(HelperSupport::callerUid()).loginName();
    pathComponents << hostComponent;
    pathComponents << pathComponent;

    for (const QString &component : std::as_const(pathComponents)) {
        dir.setPath(dir.canonicalPath() + QDir::separator() + component);

        if (!dir.exists()) {
            if (!dir.mkdir(dir.path(),
                           QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner | QFileDevice::ReadGroup | QFileDevice::ExeGroup
                               | QFileDevice::ReadOther | QFileDevice::ExeOther)) {
                return std::nullopt;
            }
        }
    }

    return dir.canonicalPath();
}

void Smb4KMountHelper::removeMountPoint(const QString &mountPoint) const
{
    if (isMountPointAllowed(mountPoint)) {
        QDir dir;
        dir.rmpath(mountPoint);
    }
}

bool Smb4KMountHelper::isMountPointAllowed(const QString &mountPoint) const
{
    QString canonicalMountPoint = QDir(mountPoint).canonicalPath();
    QString canonicalUserDirectory = QDir(mountPrefix()).canonicalPath() + QDir::separator() + KUser(HelperSupport::callerUid()).loginName();

    if (!canonicalMountPoint.startsWith(canonicalUserDirectory)) {
        return false;
    }

    return true;
}

bool Smb4KMountHelper::checkFileDescriptor(const QDBusUnixFileDescriptor &dbusFd)
{
    if (!dbusFd.isValid()) {
        return false;
    }

    // Check if the file descriptor actually points to a regular file
    struct stat s;

    if (fstat(dbusFd.fileDescriptor(), &s)) {
        return false;
    }

    if (!S_ISREG(s.st_mode)) {
        return false;
    }

    int flags = fcntl(dbusFd.fileDescriptor(), F_GETFL);
    if ((flags & (O_PATH | O_DIRECTORY)) != 0) {
        return false;
    }

    return true;
}
