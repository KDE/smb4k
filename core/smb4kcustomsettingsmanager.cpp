/*
    Manage custom settings

    SPDX-FileCopyrightText: 2011-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <QDebug>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// KDE includes
#include <KLocalizedString>

using namespace Smb4KGlobal;

class Smb4KCustomSettingsManagerPrivate
{
public:
    QList<CustomSettingsPtr> customSettings;
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

    read();

    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileRemoved, this, &Smb4KCustomSettingsManager::slotProfileRemoved);
    connect(Smb4KProfileManager::self(), &Smb4KProfileManager::profileMigrated, this, &Smb4KCustomSettingsManager::slotProfileMigrated);
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
        CustomSettingsPtr settings = findCustomSettings(share, true);

        if (!settings) {
            settings = CustomSettingsPtr(new Smb4KCustomSettings(share.data()));
            // add() takes care of the profile, we do not need to set it here.
            add(settings);
        }

        // If the options are already in the list, check if the share is
        // always to be remounted. If so, ignore the 'always' argument
        // and leave that option untouched.
        if (settings->remount() != Smb4KCustomSettings::RemountAlways) {
            settings->setRemount(always ? Smb4KCustomSettings::RemountAlways : Smb4KCustomSettings::RemountOnce);
        }

        write();
        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::removeRemount(const SharePtr &share, bool force)
{
    if (share) {
        CustomSettingsPtr settings = findCustomSettings(share, true);

        if (settings) {
            if (settings->remount() == Smb4KCustomSettings::RemountOnce) {
                settings->setRemount(Smb4KCustomSettings::UndefinedRemount);
            } else if (settings->remount() == Smb4KCustomSettings::RemountAlways && force) {
                settings->setRemount(Smb4KCustomSettings::UndefinedRemount);
            }

            if (!settings->hasCustomSettings()) {
                remove(settings);
            }
        }

        write();
        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::clearRemounts(bool force)
{
    QList<CustomSettingsPtr> settingsList = customSettings(false);

    for (const CustomSettingsPtr &settings : qAsConst(settingsList)) {
        if (settings->type() == Share) {
            if (settings->remount() == Smb4KCustomSettings::RemountOnce) {
                settings->setRemount(Smb4KCustomSettings::UndefinedRemount);
            } else if (settings->remount() == Smb4KCustomSettings::RemountAlways && force) {
                settings->setRemount(Smb4KCustomSettings::UndefinedRemount);
            }
        }

        if (!settings->hasCustomSettings()) {
            remove(settings);
        }
    }

    write();
    Q_EMIT updated();
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::sharesToRemount()
{
    QList<CustomSettingsPtr> settingsList = customSettings(false);
    QList<CustomSettingsPtr> remountsList;

    for (const CustomSettingsPtr &settings : qAsConst(settingsList)) {
        if (settings->remount() != Smb4KCustomSettings::UndefinedRemount) {
            remountsList << settings;
        }
    }

    return remountsList;
}

CustomSettingsPtr Smb4KCustomSettingsManager::findCustomSettings(const NetworkItemPtr &networkItem, bool exactMatch)
{
    CustomSettingsPtr settings = findCustomSettings(networkItem->url());

    if (!settings && !exactMatch && networkItem->type() == Share) {
        CustomSettingsPtr hostSettings = findCustomSettings(networkItem->url().adjusted(QUrl::RemovePath | QUrl::StripTrailingSlash));

        if (hostSettings) {
            settings = CustomSettingsPtr(new Smb4KCustomSettings(networkItem.data()));
            settings->update(hostSettings.data());
        }
    }

    return settings;
}

CustomSettingsPtr Smb4KCustomSettingsManager::findCustomSettings(const QUrl &url)
{
    CustomSettingsPtr settings;

    if (url.isValid() && url.scheme() == QStringLiteral("smb")) {
        QList<CustomSettingsPtr> settingsList = customSettings(false);

        for (const CustomSettingsPtr &cs : qAsConst(settingsList)) {
            if (cs->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)
                == url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)) {
                settings = cs;
                break;
            }
        }
    }

    return settings;
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::customSettings(bool withoutRemountOnce) const
{
    QList<CustomSettingsPtr> settingsList;

    for (const CustomSettingsPtr &settings : qAsConst(d->customSettings)) {
        if (Smb4KSettings::useProfiles() && settings->profile() != Smb4KProfileManager::self()->activeProfile()) {
            continue;
        }

        if (settings->hasCustomSettings(withoutRemountOnce)) {
            settingsList << settings;
        }
    }

    return settingsList;
}

void Smb4KCustomSettingsManager::addCustomSettings(const CustomSettingsPtr &settings)
{
    if (settings) {
        add(settings);
        write();
        Q_EMIT updated();
    }
}

void Smb4KCustomSettingsManager::removeCustomSettings(const CustomSettingsPtr &settings)
{
    if (settings) {
        remove(settings);
        write();
        Q_EMIT updated();
    }
}

QList<CustomSettingsPtr> Smb4KCustomSettingsManager::wakeOnLanEntries() const
{
    QList<CustomSettingsPtr> wakeOnLanList;
    QList<CustomSettingsPtr> settingsList = customSettings();

    for (const CustomSettingsPtr &settings : qAsConst(settingsList)) {
        if (!settings->macAddress().isEmpty() && (settings->wakeOnLanSendBeforeNetworkScan() || settings->wakeOnLanSendBeforeMount())) {
            wakeOnLanList << settings;
        }
    }

    return wakeOnLanList;
}

void Smb4KCustomSettingsManager::saveCustomSettings(const QList<CustomSettingsPtr> &settingsList)
{
    QMutableListIterator<CustomSettingsPtr> it(d->customSettings);

    // NOTE: Do not use Smb4KCustomSettingsManager::remove() here to avoid crashes.
    while (it.hasNext()) {
        CustomSettingsPtr settings = it.next();

        if (!Smb4KSettings::useProfiles() || settings->profile() == Smb4KSettings::activeProfile()) {
            it.remove();
            settings.clear();
        }
    }

    for (const CustomSettingsPtr &settings : settingsList) {
        add(settings);
    }

    write();
    Q_EMIT updated();
}

void Smb4KCustomSettingsManager::add(const CustomSettingsPtr &settings)
{
    if (settings->hasCustomSettings()) {
        CustomSettingsPtr knownSettings = findCustomSettings(settings->url());

        if (knownSettings) {
            knownSettings->update(settings.data());
        } else {
            if (settings->profile().isEmpty()) {
                settings->setProfile(Smb4KProfileManager::self()->activeProfile());
            }

            d->customSettings << settings;
        }

        // Propagate the settings to the host's shares if the type is 'Host'
        if (settings->type() == Host) {
            QList<CustomSettingsPtr> customSettingsList = customSettings(true);

            for (const CustomSettingsPtr &cs : qAsConst(customSettingsList)) {
                // Since only the URL is important, do not check for the workgroup.
                // Also, if the workgroup is a DNS-SD domain, it is most likely not
                // a valid SMB workgroup or domain.
                if (cs->type() == Share && cs->hostName() == settings->hostName()) {
                    cs->update(settings.data());
                }
            }
        }
    }
}

void Smb4KCustomSettingsManager::remove(const CustomSettingsPtr &settings)
{
    for (int i = 0; i < d->customSettings.size(); i++) {
        if ((!Smb4KSettings::useProfiles() || Smb4KProfileManager::self()->activeProfile() == d->customSettings.at(i)->profile())
            && d->customSettings.at(i)->url().matches(settings->url(), QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)) {
            d->customSettings.takeAt(i).clear();
            break;
        }
    }
}

void Smb4KCustomSettingsManager::read()
{
    while (!d->customSettings.isEmpty()) {
        d->customSettings.takeFirst().clear();
    }

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
                        CustomSettingsPtr settings = CustomSettingsPtr(new Smb4KCustomSettings());
                        settings->setProfile(xmlReader.attributes().value(QStringLiteral("profile")).toString());

                        if (QString::compare(xmlReader.attributes().value(QStringLiteral("type")).toString(), QStringLiteral("host"), Qt::CaseInsensitive)
                            == 0) {
                            settings->setNetworkItem(new Smb4KHost());
                        } else {
                            settings->setNetworkItem(new Smb4KShare());
                        }

                        while (!(xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("options"))) {
                            xmlReader.readNext();

                            if (xmlReader.isStartElement()) {
                                if (xmlReader.name() == QStringLiteral("workgroup")) {
                                    settings->setWorkgroupName(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("url")) {
                                    QUrl url(xmlReader.readElementText());
                                    settings->setUrl(url);
                                } else if (xmlReader.name() == QStringLiteral("ip")) {
                                    settings->setIpAddress(xmlReader.readElementText());
                                } else if (xmlReader.name() == QStringLiteral("custom")) {
                                    while (!(xmlReader.isEndElement() && xmlReader.name() == QStringLiteral("custom"))) {
                                        xmlReader.readNext();

                                        if (xmlReader.isStartElement()) {
                                            if (xmlReader.name() == QStringLiteral("smb_port")) {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setSmbPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_smb_port")) {
                                                bool ok = false;
                                                bool useSmbPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseSmbPort(useSmbPort);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("kerberos")) {
                                                bool ok = false;
                                                bool useKerberos = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseKerberos(useKerberos);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("mac_address")) {
                                                QString macAddress = xmlReader.readElementText();

                                                QRegularExpression expression(QStringLiteral("..\\:..\\:..\\:..\\:..\\:.."));

                                                if (expression.match(macAddress).hasMatch()) {
                                                    settings->setMacAddress(macAddress);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("wol_send_before_first_scan")) {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setWakeOnLanSendBeforeNetworkScan(send);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("wol_send_before_mount")) {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setWakeOnLanSendBeforeMount(send);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("remount")) {
                                                bool ok = false;
                                                int remount = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setRemount(remount);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_user")) {
                                                bool ok = false;
                                                bool useUser = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseUser(useUser);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("uid")) {
                                                bool ok = false;
                                                int uid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUser user((K_UID)uid);

                                                    if (user.isValid()) {
                                                        settings->setUser(user);
                                                    }
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_group")) {
                                                bool ok = false;
                                                bool useGroup = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseGroup(useGroup);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("gid")) {
                                                bool ok = false;
                                                int gid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUserGroup group((K_GID)gid);

                                                    if (group.isValid()) {
                                                        settings->setGroup(group);
                                                    }
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_file_mode")) {
                                                bool ok = false;
                                                bool useFileMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseFileMode(useFileMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("file_mode")) {
                                                settings->setFileMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == QStringLiteral("use_directory_mode")) {
                                                bool ok = false;
                                                bool useDirectoryMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseDirectoryMode(useDirectoryMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("directory_mode")) {
                                                settings->setDirectoryMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == QStringLiteral("use_client_protocol_versions")) {
                                                bool ok = false;
                                                bool useClientProtocolVersions = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseClientProtocolVersions(useClientProtocolVersions);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("minimal_client_protocol_version")) {
                                                bool ok = false;
                                                int minimalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setMinimalClientProtocolVersion(minimalClientProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("maximal_client_protocol_version")) {
                                                bool ok = false;
                                                int maximalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setMaximalClientProtocolVersion(maximalClientProtocolVersion);
                                                }
                                            }
#if defined(Q_OS_LINUX)
                                            else if (xmlReader.name() == QStringLiteral("cifs_unix_extensions_support")) {
                                                bool ok = false;
                                                bool cifsUnixExtensionsSupported = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setCifsUnixExtensionsSupport(cifsUnixExtensionsSupported);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_filesystem_port")) {
                                                bool ok = false;
                                                bool useFilesystemPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseFileSystemPort(useFilesystemPort);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("filesystem_port")) {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setFileSystemPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_smb_mount_protocol_version")) {
                                                bool ok = false;
                                                bool useMountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseMountProtocolVersion(useMountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("smb_mount_protocol_version")) {
                                                bool ok = false;
                                                int mountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setMountProtocolVersion(mountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_security_mode")) {
                                                bool ok = false;
                                                bool useSecurityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseSecurityMode(useSecurityMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("security_mode")) {
                                                bool ok = false;
                                                int securityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setSecurityMode(securityMode);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("use_write_access")) {
                                                bool ok = false;
                                                bool useWriteAccess = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setUseWriteAccess(useWriteAccess);
                                                }
                                            } else if (xmlReader.name() == QStringLiteral("write_access")) {
                                                bool ok = false;
                                                int writeAccess = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    settings->setWriteAccess(writeAccess);
                                                }
                                            }
#endif
                                        }
                                    }
                                }
                            }
                        }

                        d->customSettings << settings;
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

void Smb4KCustomSettingsManager::write()
{
    QFile xmlFile(dataLocation() + QDir::separator() + QStringLiteral("custom_options.xml"));

    if (d->customSettings.isEmpty()) {
        xmlFile.remove();
        return;
    }

    if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QXmlStreamWriter xmlWriter(&xmlFile);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement(QStringLiteral("custom_options"));
        xmlWriter.writeAttribute(QStringLiteral("version"), QStringLiteral("3.0"));

        for (const CustomSettingsPtr &settings : qAsConst(d->customSettings)) {
            if (settings->hasCustomSettings()) {
                xmlWriter.writeStartElement(QStringLiteral("options"));
                xmlWriter.writeAttribute(QStringLiteral("type"), settings->type() == Host ? QStringLiteral("host") : QStringLiteral("share"));
                xmlWriter.writeAttribute(QStringLiteral("profile"), settings->profile());

                xmlWriter.writeTextElement(QStringLiteral("workgroup"), settings->workgroupName());
                xmlWriter.writeTextElement(QStringLiteral("url"), settings->url().toDisplayString());
                xmlWriter.writeTextElement(QStringLiteral("ip"), settings->ipAddress());

                xmlWriter.writeStartElement(QStringLiteral("custom"));

                QMap<QString, QString> map = settings->customSettings();
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

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomSettingsManager::slotProfileRemoved(const QString &name)
{
    QMutableListIterator<CustomSettingsPtr> it(d->customSettings);

    while (it.hasNext()) {
        CustomSettingsPtr settings = it.next();

        if (name == settings->profile()) {
            it.remove();
        }
    }

    write();
    Q_EMIT updated();
}

void Smb4KCustomSettingsManager::slotProfileMigrated(const QString &oldName, const QString &newName)
{
    for (const CustomSettingsPtr &settings : qAsConst(d->customSettings)) {
        if (oldName == settings->profile()) {
            settings->setProfile(newName);
        }
    }

    write();
    Q_EMIT updated();
}
