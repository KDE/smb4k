/*
    Manage custom options

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcustomsettingsmanager.h"
#include "smb4kcustomsettings.h"
#include "smb4kglobal.h"
#include "smb4khomesshareshandler.h"
#include "smb4khost.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

class Smb4KCustomSettingsManagerPrivate
{
public:
    QList<CustomSettingsPtr> options;
};

class Smb4KCustomSettingsManagerStatic
{
public:
    Smb4KCustomSettingsManager instance;
};

Q_GLOBAL_STATIC(Smb4KCustomSettingsManagerStatic, p);

Smb4KCustomSettingsManager::Smb4KCustomSettingsManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KCustomSettingsManagerPrivate)
{
    // First we need the directory.
    QString path = dataLocation();

    QDir dir;

    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    readCustomSettings();

    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileRemoved, this, &Smb4KCustomSettingsManager::slotProfileRemoved);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileMigrated, this, &Smb4KCustomSettingsManager::slotProfileMigrated);
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Smb4KCustomSettingsManager::slotAboutToQuit);
}

Smb4KCustomSettingsManager::~Smb4KCustomSettingsManager()
{
}

Smb4KCustomSettingsManager *Smb4KCustomSettingsManager::self()
{
    return &p->instance;
}

void Smb4KCustomSettingsManager::addRemount(const SharePtr &share, bool always)
{
    if (share) {
        //
        // Find the right custom options, if they exist
        //
        CustomSettingsPtr options = findCustomSettings(share, true);

        if (options) {
            // If the options are already in the list, check if the share is
            // always to be remounted. If so, ignore the 'always' argument
            // and leave that option untouched.
            if (options->remount() != Smb4KCustomSettings::RemountAlways) {
                options->setRemount(always ? Smb4KCustomSettings::RemountAlways : Smb4KCustomSettings::RemountOnce);
            }
        } else {
            options = CustomSettingsPtr(new Smb4KCustomSettings(share.data()));
            options->setProfile(Smb4KProfileManager::self()->activeProfile());
            options->setRemount(always ? Smb4KCustomSettings::RemountAlways : Smb4KCustomSettings::RemountOnce);
            d->options << options;
        }

        writeCustomSettings();
        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::removeRemount(const SharePtr &share, bool force)
{
    if (share) {
        //
        // Get the remount
        //
        CustomSettingsPtr options = findCustomSettings(share, true);

        //
        // Remove the remount flag and, if there are no more options defined,
        // the options object itself. Save the modified list to the file afterwards.
        //
        if (options) {
            if (options->remount() == Smb4KCustomSettings::RemountOnce) {
                options->setRemount(Smb4KCustomSettings::UndefinedRemount);
            } else if (options->remount() == Smb4KCustomSettings::RemountAlways && force) {
                options->setRemount(Smb4KCustomSettings::UndefinedRemount);
            }

            if (!options->hasOptions()) {
                removeCustomSettings(options, false);
            }
        }

        writeCustomSettings();
        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::clearRemounts(bool force)
{
    //
    // Remove the remount flag and, if there are nomore options defined,
    // also the options object. Write everything to the file afterwards.
    //
    for (const CustomSettingsPtr &options : qAsConst(d->options)) {
        if (options->type() == Share) {
            if (options->remount() == Smb4KCustomSettings::RemountOnce) {
                options->setRemount(Smb4KCustomSettings::UndefinedRemount);
            } else if (options->remount() == Smb4KCustomSettings::RemountAlways && force) {
                options->setRemount(Smb4KCustomSettings::UndefinedRemount);
            }
        }

        if (!options->hasOptions()) {
            removeCustomSettings(options, false);
        }
    }

    writeCustomSettings();
    Q_EMIT updated();
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::sharesToRemount()
{
    //
    // List of relevant custom options
    //
    QList<CustomSettingsPtr> optionsList = customSettings(false);

    //
    // List of remounts
    //
    QList<CustomSettingsPtr> remounts;

    //
    // Get the list of remounts
    //
    for (const CustomSettingsPtr &options : qAsConst(optionsList)) {
        if (options->remount() != Smb4KCustomSettings::UndefinedRemount) {
            remounts << options;
        }
    }

    //
    // Return relevant options
    //
    return remounts;
}

CustomSettingsPtr Smb4KCustomSettingsManager::findCustomSettings(const NetworkItemPtr &networkItem, bool exactMatch)
{
    CustomSettingsPtr options;

    if (exactMatch) {
        options = findCustomSettings(networkItem->url());
    } else {
        if (networkItem->type() == Host) {
            options = findCustomSettings(networkItem->url());
        } else if (networkItem->type() == Share) {
            options = findCustomSettings(networkItem->url());

            // Get the host's custom options, if needed
            if (!options) {
                CustomSettingsPtr shareOptions = CustomSettingsPtr(new Smb4KCustomSettings(networkItem.data()));

                QUrl hostUrl = networkItem->url().adjusted(QUrl::RemovePath);
                CustomSettingsPtr hostOptions = findCustomSettings(hostUrl);

                if (hostOptions) {
                    shareOptions->update(hostOptions.data());
                    options = shareOptions;
                }
            }
        }
    }

    return options;
}

CustomSettingsPtr Smb4KCustomSettingsManager::findCustomSettings(const QUrl &url)
{
    //
    // The options that are to be returned
    //
    CustomSettingsPtr options;

    //
    // Search the options for the given URL
    //
    if (url.isValid() && url.scheme() == QStringLiteral("smb")) {
        //
        // Get the relevant options
        //
        QList<CustomSettingsPtr> optionsList = customSettings(false);

        //
        // Get the options
        //
        for (const CustomSettingsPtr &o : qAsConst(optionsList)) {
            if (o->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)
                == url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)) {
                options = o;
                break;
            }
        }
    }

    //
    // Return the options
    //
    return options;
}

void Smb4KCustomSettingsManager::readCustomSettings()
{
    //
    // Clear the list of options
    //
    while (!d->options.isEmpty()) {
        d->options.takeFirst().clear();
    }

    //
    // Set the XML file
    //
    QFile xmlFile(dataLocation() + QDir::separator() + QStringLiteral("custom_options.xml"));

    if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader xmlReader(&xmlFile);

        while (!xmlReader.atEnd()) {
            xmlReader.readNext();

            if (xmlReader.isStartElement()) {
                if (xmlReader.name() == QStringLiteral("custom_options") && xmlReader.attributes().value(QStringLiteral("version")) != QStringLiteral("3.0")) {
                    xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
                    break;
                } else {
                    if (xmlReader.name() == QStringLiteral("options")) {
                        CustomSettingsPtr options = CustomSettingsPtr(new Smb4KCustomSettings());
                        options->setProfile(xmlReader.attributes().value(QStringLiteral("profile")).toString());

                        //
                        // Initialize the options
                        //
                        if (QString::compare(xmlReader.attributes().value(QStringLiteral("type")).toString(), QStringLiteral("host"), Qt::CaseInsensitive)
                            == 0) {
                            options->setNetworkItem(new Smb4KHost());
                        } else {
                            options->setNetworkItem(new Smb4KShare());
                        }

                        while (!(xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("options"))) {
                            xmlReader.readNext();

                            if (xmlReader.isStartElement()) {
                                if (xmlReader.name() == QStringLiteral("workgroup")) {
                                    options->setWorkgroupName(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("url")) {
                                    QUrl url(xmlReader.readElementText());
                                    options->setUrl(url);
                                } else if (xmlReader.name() == QStringLiteral("ip")) {
                                    options->setIpAddress(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("custom")) {
                                    while (!(xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("custom"))) {
                                        xmlReader.readNext();

                                        if (xmlReader.isStartElement()) {
                                            if (xmlReader.name() == QStringLiteral("smb_port")) {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setSmbPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_smb_port")) {
                                                bool ok = false;
                                                bool useSmbPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseSmbPort(useSmbPort);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("kerberos")) {
                                                bool ok = false;
                                                bool useKerberos = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseKerberos(useKerberos);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("mac_address")) {
                                                QString macAddress = xmlReader.readElementText();

                                                QRegularExpression expression(QStringLiteral("..\\:..\\:..\\:..\\:..\\:.."));

                                                if (expression.match(macAddress).hasMatch()) {
                                                    options->setMACAddress(macAddress);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("wol_send_before_first_scan")) {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setWOLSendBeforeNetworkScan(send);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("wol_send_before_mount")) {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setWOLSendBeforeMount(send);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("remount")) {
                                                bool ok = false;
                                                int remount = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setRemount(remount);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_user")) {
                                                bool ok = false;
                                                bool useUser = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseUser(useUser);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("uid")) {
                                                bool ok = false;
                                                int uid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUser user((K_UID)uid);

                                                    if (user.isValid()) {
                                                        options->setUser(user);
                                                    }
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_group")) {
                                                bool ok = false;
                                                bool useGroup = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseGroup(useGroup);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("gid")) {
                                                bool ok = false;
                                                int gid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUserGroup group((K_GID)gid);

                                                    if (group.isValid()) {
                                                        options->setGroup(group);
                                                    }
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_file_mode")) {
                                                bool ok = false;
                                                bool useFileMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseFileMode(useFileMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("file_mode")) {
                                                options->setFileMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == QStringLiteral("use_directory_mode")) {
                                                bool ok = false;
                                                bool useDirectoryMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseDirectoryMode(useDirectoryMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("directory_mode")) {
                                                options->setDirectoryMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == QStringLiteral("use_client_protocol_versions")) {
                                                bool ok = false;
                                                bool useClientProtocolVersions = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseClientProtocolVersions(useClientProtocolVersions);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("minimal_client_protocol_version")) {
                                                bool ok = false;
                                                int minimalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMinimalClientProtocolVersion(minimalClientProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("maximal_client_protocol_version")) {
                                                bool ok = false;
                                                int maximalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMaximalClientProtocolVersion(maximalClientProtocolVersion);
                                                }
                                            }
#if defined(Q_OS_LINUX)
                                            else if (xmlReader.name() == QStringLiteral("cifs_unix_extensions_support")) {
                                                bool ok = false;
                                                bool cifsUnixExtensionsSupported = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setCifsUnixExtensionsSupport(cifsUnixExtensionsSupported);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_filesystem_port")) {
                                                bool ok = false;
                                                bool useFilesystemPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseFileSystemPort(useFilesystemPort);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("filesystem_port")) {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setFileSystemPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_smb_mount_protocol_version")) {
                                                bool ok = false;
                                                bool useMountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseMountProtocolVersion(useMountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("smb_mount_protocol_version")) {
                                                bool ok = false;
                                                int mountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMountProtocolVersion(mountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_security_mode")) {
                                                bool ok = false;
                                                bool useSecurityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseSecurityMode(useSecurityMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("security_mode")) {
                                                bool ok = false;
                                                int securityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setSecurityMode(securityMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_write_access")) {
                                                bool ok = false;
                                                bool useWriteAccess = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseWriteAccess(useWriteAccess);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("write_access")) {
                                                bool ok = false;
                                                int writeAccess = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setWriteAccess(writeAccess);
                                                }
                                            }
#endif
                                        }
                                    }
                                }

                                continue;
                            } else {
                                continue;
                            }
                        }

                        d->options << options;
                    }
                }
            }
        }

        xmlFile.close();

        if (xmlReader.hasError()) {
            Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
        }
    } else {
        if (xmlFile.exists()) {
            Smb4KNotification::openingFileFailed(xmlFile);
        }
    }
}

void Smb4KCustomSettingsManager::writeCustomSettings()
{
    QFile xmlFile(dataLocation() + QDir::separator() + QStringLiteral("custom_options.xml"));

    if (d->options.isEmpty()) {
        xmlFile.remove();
        return;
    }

    if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QXmlStreamWriter xmlWriter(&xmlFile);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement(QStringLiteral("custom_options"));
        xmlWriter.writeAttribute(QStringLiteral("version"), QStringLiteral("3.0"));

        for (const CustomSettingsPtr &options : qAsConst(d->options)) {
            if (options->hasOptions()) {
                xmlWriter.writeStartElement(QStringLiteral("options"));
                xmlWriter.writeAttribute(QStringLiteral("type"), options->type() == Host ? QStringLiteral("host") : QStringLiteral("share"));
                xmlWriter.writeAttribute(QStringLiteral("profile"), options->profile());

                xmlWriter.writeTextElement(QStringLiteral("workgroup"), options->workgroupName());
                xmlWriter.writeTextElement(QStringLiteral("url"), options->url().toDisplayString());
                xmlWriter.writeTextElement(QStringLiteral("ip"), options->ipAddress());

                xmlWriter.writeStartElement(QStringLiteral("custom"));

                QMap<QString, QString> map = options->customSettings();
                QMapIterator<QString, QString> i(map);

                while (i.hasNext()) {
                    i.next();

                    if (!i.value().isEmpty()) {
                        xmlWriter.writeTextElement(i.key(), i.value());
                    }
                }

                xmlWriter.writeEndElement();
                xmlWriter.writeEndElement();
            }
        }

        xmlWriter.writeEndDocument();
        xmlFile.close();
    } else {
        Smb4KNotification::openingFileFailed(xmlFile);
    }
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::customSettings(bool withoutRemountOnce) const
{
    QList<CustomSettingsPtr> optionsList;

    for (const CustomSettingsPtr &options : qAsConst(d->options)) {
        if (Smb4KSettings::useProfiles() && options->profile() != Smb4KProfileManager::self()->activeProfile()) {
            continue;
        }

        if (options->hasOptions(withoutRemountOnce)) {
            optionsList << options;
        }
    }

    return optionsList;
}

void Smb4KCustomSettingsManager::addCustomSettings(const CustomSettingsPtr &options, bool write)
{
    if (options) {
        CustomSettingsPtr knownOptions = findCustomSettings(options->url());

        if (knownOptions) {
            knownOptions->update(options.data());
        } else {
            if (options->profile().isEmpty()) {
                options->setProfile(Smb4KProfileManager::self()->activeProfile());
            }

            d->options << options;
        }

        //
        // In case the options are defined for a host, propagate them
        // to the options of shares belonging to that host. Overwrite
        // the settings
        //
        if (options->type() == Host) {
            QList<CustomSettingsPtr> allOptions = customSettings(true);

            for (const CustomSettingsPtr &o : qAsConst(allOptions)) {
                // FIXME: Can we remove the check of the workgroup?
                if (o->type() == Share && o->hostName() == options->hostName() && o->workgroupName() == options->workgroupName()) {
                    o->setIpAddress(options->ipAddress());
                    o->setUseUser(options->useUser());
                    o->setUser(options->user());
                    o->setUseGroup(options->useGroup());
                    o->setGroup(options->group());
                    o->setUseFileMode(options->useFileMode());
                    o->setFileMode(options->fileMode());
                    o->setUseDirectoryMode(options->useDirectoryMode());
                    o->setDirectoryMode(options->directoryMode());
#if defined(Q_OS_LINUX)
                    o->setCifsUnixExtensionsSupport(options->cifsUnixExtensionsSupport());
                    o->setUseFileSystemPort(options->useFileSystemPort());
                    o->setFileSystemPort(options->fileSystemPort());
                    o->setUseMountProtocolVersion(options->useMountProtocolVersion());
                    o->setMountProtocolVersion(options->mountProtocolVersion());
                    o->setUseSecurityMode(options->useSecurityMode());
                    o->setSecurityMode(options->securityMode());
                    o->setUseWriteAccess(options->useWriteAccess());
                    o->setWriteAccess(options->writeAccess());
#endif
                    o->setUseSmbPort(options->useSmbPort());
                    o->setSmbPort(options->smbPort());
                    o->setUseKerberos(options->useKerberos());
                    o->setMACAddress(options->macAddress());
                    o->setWOLSendBeforeNetworkScan(options->wolSendBeforeNetworkScan());
                    o->setWOLSendBeforeMount(options->wolSendBeforeMount());
                }
            }
        }

        if (write) {
            writeCustomSettings();
        }

        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::removeCustomSettings(const CustomSettingsPtr &options, bool write)
{
    if (options) {
        for (int i = 0; i < d->options.size(); ++i) {
            if ((!Smb4KSettings::useProfiles() || Smb4KProfileManager::self()->activeProfile() == d->options.at(i)->profile())
                && d->options.at(i)->url().matches(options->url(), QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)) {
                d->options.takeAt(i).clear();
                break;
            }
        }

        if (write) {
            writeCustomSettings();
        }

        Q_EMIT updated();
    }
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::wakeOnLanEntries() const
{
    QList<CustomSettingsPtr> optionsList;
    QList<CustomSettingsPtr> allOptions = customSettings();

    for (const CustomSettingsPtr &options : qAsConst(allOptions)) {
        if (!options->macAddress().isEmpty() && (options->wolSendBeforeNetworkScan() || options->wolSendBeforeMount())) {
            optionsList << options;
        }
    }

    return optionsList;
}

void Smb4KCustomSettingsManager::saveCustomSettings(const QList<CustomSettingsPtr> &optionsList)
{
    QMutableListIterator<CustomSettingsPtr> it(d->options);

    while (it.hasNext()) {
        CustomSettingsPtr options = it.next();
        removeCustomSettings(options);
    }

    for (const CustomSettingsPtr &options : optionsList) {
        addCustomSettings(options, false);
    }

    writeCustomSettings();
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomSettingsManager::slotAboutToQuit()
{
    writeCustomSettings();
}

void Smb4KCustomSettingsManager::slotProfileRemoved(const QString &name)
{
    QMutableListIterator<CustomSettingsPtr> it(d->options);

    while (it.hasNext()) {
        CustomSettingsPtr options = it.next();

        if (name == options->profile()) {
            it.remove();
        }
    }

    writeCustomSettings();
    Q_EMIT updated();
}

void Smb4KCustomSettingsManager::slotProfileMigrated(const QString &oldName, const QString &newName)
{
    for (const CustomSettingsPtr &options : qAsConst(d->options)) {
        if (oldName == options->profile()) {
            options->setProfile(newName);
        }
    }

    writeCustomSettings();
    Q_EMIT updated();
}
