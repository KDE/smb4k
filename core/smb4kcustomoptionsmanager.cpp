/*
    Manage custom options

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcustomoptionsmanager.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager_p.h"
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
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KCustomOptionsManagerStatic, p);

Smb4KCustomOptionsManager::Smb4KCustomOptionsManager(QObject *parent)
    : QObject(parent)
    , d(new Smb4KCustomOptionsManagerPrivate)
{
    // First we need the directory.
    QString path = dataLocation();

    QDir dir;

    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    readCustomOptions();

    // Connections
    connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
}

Smb4KCustomOptionsManager::~Smb4KCustomOptionsManager()
{
}

Smb4KCustomOptionsManager *Smb4KCustomOptionsManager::self()
{
    return &p->instance;
}

void Smb4KCustomOptionsManager::addRemount(const SharePtr &share, bool always)
{
    if (share) {
        //
        // Find the right custom options, if they exist
        //
        OptionsPtr options = findOptions(share, true);

        if (options) {
            // If the options are already in the list, check if the share is
            // always to be remounted. If so, ignore the 'always' argument
            // and leave that option untouched.
            if (options->remount() != Smb4KCustomOptions::RemountAlways) {
                options->setRemount(always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce);
            }
        } else {
            options = OptionsPtr(new Smb4KCustomOptions(share.data()));
            options->setProfile(Smb4KProfileManager::self()->activeProfile());
            options->setRemount(always ? Smb4KCustomOptions::RemountAlways : Smb4KCustomOptions::RemountOnce);
            d->options << options;
        }

        //
        // Write the custom options
        //
        writeCustomOptions();
    }
}

void Smb4KCustomOptionsManager::removeRemount(const SharePtr &share, bool force)
{
    if (share) {
        //
        // Get the remount
        //
        OptionsPtr options = findOptions(share, true);

        //
        // Remove the remount flag and, if there are no more options defined,
        // the options object itself. Save the modified list to the file afterwards.
        //
        if (options) {
            if (options->remount() == Smb4KCustomOptions::RemountOnce) {
                options->setRemount(Smb4KCustomOptions::UndefinedRemount);
            } else if (options->remount() == Smb4KCustomOptions::RemountAlways && force) {
                options->setRemount(Smb4KCustomOptions::UndefinedRemount);
            }

            if (!options->hasOptions()) {
                removeCustomOptions(options, false);
            }
        }

        //
        // Write the options
        //
        writeCustomOptions();
    }
}

void Smb4KCustomOptionsManager::clearRemounts(bool force)
{
    //
    // Remove the remount flag and, if there are nomore options defined,
    // also the options object. Write everything to the file afterwards.
    //
    for (const OptionsPtr &options : qAsConst(d->options)) {
        if (options->type() == Share) {
            if (options->remount() == Smb4KCustomOptions::RemountOnce) {
                options->setRemount(Smb4KCustomOptions::UndefinedRemount);
            } else if (options->remount() == Smb4KCustomOptions::RemountAlways && force) {
                options->setRemount(Smb4KCustomOptions::UndefinedRemount);
            }
        }

        if (!options->hasOptions()) {
            removeCustomOptions(options, false);
        }
    }

    //
    // Write the options
    //
    writeCustomOptions();
}

QList<OptionsPtr> Smb4KCustomOptionsManager::sharesToRemount()
{
    //
    // List of relevant custom options
    //
    QList<OptionsPtr> optionsList = customOptions(false);

    //
    // List of remounts
    //
    QList<OptionsPtr> remounts;

    //
    // Get the list of remounts
    //
    for (const OptionsPtr &options : qAsConst(optionsList)) {
        if (options->remount() != Smb4KCustomOptions::UndefinedRemount) {
            remounts << options;
        }
    }

    //
    // Return relevant options
    //
    return remounts;
}

OptionsPtr Smb4KCustomOptionsManager::findOptions(const NetworkItemPtr &networkItem, bool exactMatch)
{
    OptionsPtr options;

    if (exactMatch) {
        options = findOptions(networkItem->url());
    } else {
        if (networkItem->type() == Host) {
            options = findOptions(networkItem->url());
        } else if (networkItem->type() == Share) {
            options = findOptions(networkItem->url());

            // Get the host's custom options, if needed
            if (!options) {
                OptionsPtr shareOptions = OptionsPtr(new Smb4KCustomOptions(networkItem.data()));

                QUrl hostUrl = networkItem->url().adjusted(QUrl::RemovePath);
                OptionsPtr hostOptions = findOptions(hostUrl);

                if (hostOptions) {
                    shareOptions->update(hostOptions.data());
                    options = shareOptions;
                }
            }
        }
    }

    return options;
}

OptionsPtr Smb4KCustomOptionsManager::findOptions(const QUrl &url)
{
    //
    // The options that are to be returned
    //
    OptionsPtr options;

    //
    // Search the options for the given URL
    //
    if (url.isValid() && url.scheme() == "smb") {
        //
        // Get the relevant options
        //
        QList<OptionsPtr> optionsList = customOptions(false);

        //
        // Get the options
        //
        for (const OptionsPtr &o : qAsConst(optionsList)) {
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

void Smb4KCustomOptionsManager::readCustomOptions()
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
    QFile xmlFile(dataLocation() + QDir::separator() + "custom_options.xml");

    if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QXmlStreamReader xmlReader(&xmlFile);

        while (!xmlReader.atEnd()) {
            xmlReader.readNext();

            if (xmlReader.isStartElement()) {
                if (xmlReader.name() == "custom_options" && xmlReader.attributes().value("version") != "3.0") {
                    xmlReader.raiseError(i18n("The format of %1 is not supported.", xmlFile.fileName()));
                    break;
                } else {
                    if (xmlReader.name() == "options") {
                        OptionsPtr options = OptionsPtr(new Smb4KCustomOptions());
                        options->setProfile(xmlReader.attributes().value("profile").toString());

                        //
                        // Initialize the options
                        //
                        if (QString::compare(xmlReader.attributes().value("type").toString(), "host", Qt::CaseInsensitive) == 0) {
                            options->setNetworkItem(new Smb4KHost());
                        } else {
                            options->setNetworkItem(new Smb4KShare());
                        }

                        while (!(xmlReader.isEndElement() && xmlReader.name() == "options")) {
                            xmlReader.readNext();

                            if (xmlReader.isStartElement()) {
                                if (xmlReader.name() == "workgroup") {
                                    options->setWorkgroupName(xmlReader.readElementText());
                                } else if (xmlReader.name() == "url") {
                                    QUrl url(xmlReader.readElementText());
                                    options->setUrl(url);
                                } else if (xmlReader.name() == "ip") {
                                    options->setIpAddress(xmlReader.readElementText());
                                } else if (xmlReader.name() == "custom") {
                                    while (!(xmlReader.isEndElement() && xmlReader.name() == "custom")) {
                                        xmlReader.readNext();

                                        if (xmlReader.isStartElement()) {
                                            if (xmlReader.name() == "smb_port") {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setSmbPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == "use_smb_port") {
                                                bool ok = false;
                                                bool useSmbPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseSmbPort(useSmbPort);
                                                }
                                            } else if (xmlReader.name() == "kerberos") {
                                                bool ok = false;
                                                bool useKerberos = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseKerberos(useKerberos);
                                                }
                                            } else if (xmlReader.name() == "mac_address") {
                                                QString macAddress = xmlReader.readElementText();

                                                QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");

                                                if (exp.exactMatch(macAddress)) {
                                                    options->setMACAddress(macAddress);
                                                }
                                            } else if (xmlReader.name() == "wol_send_before_first_scan") {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setWOLSendBeforeNetworkScan(send);
                                                }
                                            } else if (xmlReader.name() == "wol_send_before_mount") {
                                                bool ok = false;
                                                bool send = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setWOLSendBeforeMount(send);
                                                }
                                            } else if (xmlReader.name() == "remount") {
                                                bool ok = false;
                                                int remount = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setRemount(remount);
                                                }
                                            } else if (xmlReader.name() == "use_user") {
                                                bool ok = false;
                                                bool useUser = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseUser(useUser);
                                                }
                                            } else if (xmlReader.name() == "uid") {
                                                bool ok = false;
                                                int uid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUser user((K_UID)uid);

                                                    if (user.isValid()) {
                                                        options->setUser(user);
                                                    }
                                                }
                                            } else if (xmlReader.name() == "use_group") {
                                                bool ok = false;
                                                bool useGroup = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseGroup(useGroup);
                                                }
                                            } else if (xmlReader.name() == "gid") {
                                                bool ok = false;
                                                int gid = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    KUserGroup group((K_GID)gid);

                                                    if (group.isValid()) {
                                                        options->setGroup(group);
                                                    }
                                                }
                                            } else if (xmlReader.name() == "use_file_mode") {
                                                bool ok = false;
                                                bool useFileMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseFileMode(useFileMode);
                                                }
                                            } else if (xmlReader.name() == "file_mode") {
                                                options->setFileMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == "use_directory_mode") {
                                                bool ok = false;
                                                bool useDirectoryMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseDirectoryMode(useDirectoryMode);
                                                }
                                            } else if (xmlReader.name() == "directory_mode") {
                                                options->setDirectoryMode(xmlReader.readElementText());
                                            } else if (xmlReader.name() == "use_client_protocol_versions") {
                                                bool ok = false;
                                                bool useClientProtocolVersions = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseClientProtocolVersions(useClientProtocolVersions);
                                                }
                                            } else if (xmlReader.name() == "minimal_client_protocol_version") {
                                                bool ok = false;
                                                int minimalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMinimalClientProtocolVersion(minimalClientProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == "maximal_client_protocol_version") {
                                                bool ok = false;
                                                int maximalClientProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMaximalClientProtocolVersion(maximalClientProtocolVersion);
                                                }
                                            }
#if defined(Q_OS_LINUX)
                                            else if (xmlReader.name() == "cifs_unix_extensions_support") {
                                                bool ok = false;
                                                bool cifsUnixExtensionsSupported = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setCifsUnixExtensionsSupport(cifsUnixExtensionsSupported);
                                                }
                                            } else if (xmlReader.name() == "use_filesystem_port") {
                                                bool ok = false;
                                                bool useFilesystemPort = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseFileSystemPort(useFilesystemPort);
                                                }
                                            } else if (xmlReader.name() == "filesystem_port") {
                                                bool ok = false;
                                                int portNumber = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setFileSystemPort(portNumber);
                                                }
                                            } else if (xmlReader.name() == "use_smb_mount_protocol_version") {
                                                bool ok = false;
                                                bool useMountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseMountProtocolVersion(useMountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == "smb_mount_protocol_version") {
                                                bool ok = false;
                                                int mountProtocolVersion = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setMountProtocolVersion(mountProtocolVersion);
                                                }
                                            } else if (xmlReader.name() == "use_security_mode") {
                                                bool ok = false;
                                                bool useSecurityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseSecurityMode(useSecurityMode);
                                                }
                                            } else if (xmlReader.name() == "security_mode") {
                                                bool ok = false;
                                                int securityMode = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setSecurityMode(securityMode);
                                                }
                                            } else if (xmlReader.name() == "use_write_access") {
                                                bool ok = false;
                                                bool useWriteAccess = xmlReader.readElementText().toInt(&ok);

                                                if (ok) {
                                                    options->setUseWriteAccess(useWriteAccess);
                                                }
                                            } else if (xmlReader.name() == "write_access") {
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

void Smb4KCustomOptionsManager::writeCustomOptions()
{
    QFile xmlFile(dataLocation() + QDir::separator() + "custom_options.xml");

    if (d->options.isEmpty()) {
        xmlFile.remove();
        return;
    }

    bool write = false;

    for (const OptionsPtr &options : qAsConst(d->options)) {
        if (options->isChanged()) {
            write = true;
            break;
        }
    }

    if (!write) {
        return;
    }

    if (xmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QXmlStreamWriter xmlWriter(&xmlFile);
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartDocument();
        xmlWriter.writeStartElement("custom_options");
        xmlWriter.writeAttribute("version", "3.0");

        for (const OptionsPtr &options : qAsConst(d->options)) {
            if (options->hasOptions()) {
                xmlWriter.writeStartElement("options");
                xmlWriter.writeAttribute("type", options->type() == Host ? "host" : "share");
                xmlWriter.writeAttribute("profile", options->profile());

                xmlWriter.writeTextElement("workgroup", options->workgroupName());
                xmlWriter.writeTextElement("url", options->url().toDisplayString());
                xmlWriter.writeTextElement("ip", options->ipAddress());

                xmlWriter.writeStartElement("custom");

                QMap<QString, QString> map = options->customOptions();
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

            options->setChanged(false);
        }

        xmlWriter.writeEndDocument();
        xmlFile.close();
    } else {
        Smb4KNotification::openingFileFailed(xmlFile);
    }
}

QList<OptionsPtr> Smb4KCustomOptionsManager::customOptions(bool withoutRemountOnce) const
{
    //
    // Options list
    //
    QList<OptionsPtr> optionsList;

    //
    // Get this list of options
    //
    for (const OptionsPtr &options : qAsConst(d->options)) {
        if (Smb4KSettings::useProfiles() && options->profile() != Smb4KProfileManager::self()->activeProfile()) {
            continue;
        }

        if (options->hasOptions(withoutRemountOnce)) {
            optionsList << options;
        }
    }

    //
    // Return the list of relevant options
    //
    return optionsList;
}

void Smb4KCustomOptionsManager::openCustomOptionsDialog(const NetworkItemPtr &item)
{
    if (item) {
        OptionsPtr options;

        switch (item->type()) {
        case Host: {
            options = findOptions(item);

            if (!options) {
                options = OptionsPtr(new Smb4KCustomOptions(item.data()));
                options->setProfile(Smb4KProfileManager::self()->activeProfile());
            }

            break;
        }
        case Share: {
            SharePtr share = item.staticCast<Smb4KShare>();

            if (share && !share->isPrinter()) {
                if (share->isHomesShare()) {
                    if (!Smb4KHomesSharesHandler::self()->specifyUser(share, true)) {
                        return;
                    }
                }

                options = findOptions(share);

                if (!options) {
                    options = OptionsPtr(new Smb4KCustomOptions(share.data()));
                    options->setProfile(Smb4KProfileManager::self()->activeProfile());

                    // Get rid of the 'homes' share
                    if (share->isHomesShare()) {
                        options->setUrl(share->homeUrl());
                    }
                }
            }

            break;
        }
        default: {
            break;
        }
        }

        openCustomOptionsDialog(options, true);
    }
}

bool Smb4KCustomOptionsManager::openCustomOptionsDialog(const OptionsPtr &options, bool write)
{
    if (options) {
        QPointer<Smb4KCustomOptionsDialog> dlg = new Smb4KCustomOptionsDialog(options, QApplication::activeWindow());

        if (dlg->exec() == QDialog::Accepted) {
            if (options->hasOptions()) {
                addCustomOptions(options, write);
            } else {
                removeCustomOptions(options, write);
            }
        } else {
            // FIXME: When operating on a list with uncommitted changes, this reset function might cause problems.
            resetCustomOptions();
        }

        delete dlg;

        return options->hasOptions();
    }

    return false;
}

void Smb4KCustomOptionsManager::addCustomOptions(const OptionsPtr &options, bool write)
{
    if (options) {
        OptionsPtr knownOptions = findOptions(options->url());

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
            QList<OptionsPtr> allOptions = customOptions(true);

            for (const OptionsPtr &o : qAsConst(allOptions)) {
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
            writeCustomOptions();
        }
    }
}

void Smb4KCustomOptionsManager::removeCustomOptions(const OptionsPtr &options, bool write)
{
    if (options) {
        //
        // Find the custom options and remove them
        //
        for (int i = 0; i < d->options.size(); ++i) {
            if ((!Smb4KSettings::useProfiles() || Smb4KProfileManager::self()->activeProfile() == d->options.at(i)->profile())
                && d->options.at(i)->url().matches(options->url(), QUrl::RemoveUserInfo | QUrl::RemovePort | QUrl::StripTrailingSlash)) {
                d->options.takeAt(i).clear();
                break;
            }
        }

        //
        // Write the custom options to the file, if desired
        //
        if (write) {
            writeCustomOptions();
        }
    }
}

QList<OptionsPtr> Smb4KCustomOptionsManager::wakeOnLanEntries() const
{
    QList<OptionsPtr> optionsList;
    QList<OptionsPtr> allOptions = customOptions();

    for (const OptionsPtr &options : qAsConst(allOptions)) {
        if (!options->macAddress().isEmpty() && (options->wolSendBeforeNetworkScan() || options->wolSendBeforeMount())) {
            optionsList << options;
        }
    }

    return optionsList;
}

void Smb4KCustomOptionsManager::resetCustomOptions()
{
    readCustomOptions();
}

void Smb4KCustomOptionsManager::saveCustomOptions()
{
    writeCustomOptions();
}

void Smb4KCustomOptionsManager::migrateProfile(const QString &from, const QString &to)
{
    //
    // Replace the old with the new profile
    //
    for (const OptionsPtr &options : qAsConst(d->options)) {
        if (options->profile() == from) {
            options->setProfile(to);
        }
    }

    //
    // Write all custom options to the file.
    //
    writeCustomOptions();
}

void Smb4KCustomOptionsManager::removeProfile(const QString &name)
{
    //
    // Remove all entries belonging to the profile
    //
    QMutableListIterator<OptionsPtr> it(d->options);

    while (it.hasNext()) {
        OptionsPtr options = it.next();

        if (QString::compare(options->profile(), name, Qt::CaseSensitive) == 0) {
            it.remove();
        }
    }

    //
    // Write all custom options to the file.
    //
    writeCustomOptions();
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsManager::slotAboutToQuit()
{
    writeCustomOptions();
}
