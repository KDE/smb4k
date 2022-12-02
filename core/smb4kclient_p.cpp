/*
    Private classes for the SMB client

    SPDX-FileCopyrightText: 2018-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kclient_p.h"
#include "smb4kcustomoptions.h"
#include "smb4kcustomoptionsmanager.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"
#include "smb4kwalletmanager.h"

// System includes
#include <errno.h>
#include <sys/stat.h>

// Qt includes
#include <QAbstractSocket>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QHostInfo>
#include <QLabel>
#include <QListWidget>
#include <QNetworkInterface>
#include <QPrinter>
#include <QPushButton>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTextDocument>
#include <QToolBar>
#include <QUuid>
#include <QVBoxLayout>
#include <QWindow>

// KDE includes
#include <KConfigGui/KWindowConfig>
#include <KI18n/KLocalizedString>
#include <KIO/Global>
#include <KIOCore/KFileItem>
#include <KIOWidgets/KUrlComboBox>
#include <KIOWidgets/KUrlRequester>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KDualAction>

#ifdef USE_WS_DISCOVERY
#include <KDSoapClient/KDQName>
#include <KDSoapClient/KDSoapClientInterface>
#include <KDSoapClient/KDSoapMessage>
#include <KDSoapClient/KDSoapMessageAddressingProperties>
#include <WSDiscoveryTargetService>
#endif

#define SMBC_DEBUG 0

using namespace Smb4KGlobal;

//
// Client base job
//

Smb4KClientBaseJob::Smb4KClientBaseJob(QObject *parent)
    : KJob(parent)
    , m_process(Smb4KGlobal::NoProcess)
{
    pProcess = &m_process;
    pNetworkItem = &m_networkItem;
    pWorkgroups = &m_workgroups;
    pHosts = &m_hosts;
    pShares = &m_shares;
    pFiles = &m_files;
}

Smb4KClientBaseJob::~Smb4KClientBaseJob()
{
    while (!m_workgroups.isEmpty()) {
        m_workgroups.takeFirst().clear();
    }

    while (!m_hosts.isEmpty()) {
        m_hosts.takeFirst().clear();
    }

    while (!m_shares.isEmpty()) {
        m_shares.takeFirst().clear();
    }

    while (!m_files.isEmpty()) {
        m_files.takeFirst().clear();
    }
}

void Smb4KClientBaseJob::setProcess(Smb4KGlobal::Process process)
{
    m_process = process;
}

Smb4KGlobal::Process Smb4KClientBaseJob::process() const
{
    return m_process;
}

void Smb4KClientBaseJob::setNetworkItem(NetworkItemPtr networkItem)
{
    m_networkItem = networkItem;
}

NetworkItemPtr Smb4KClientBaseJob::networkItem() const
{
    return m_networkItem;
}

QList<WorkgroupPtr> Smb4KClientBaseJob::workgroups()
{
    return m_workgroups;
}

QList<HostPtr> Smb4KClientBaseJob::hosts()
{
    return m_hosts;
}

QList<SharePtr> Smb4KClientBaseJob::shares()
{
    return m_shares;
}

QList<FilePtr> Smb4KClientBaseJob::files()
{
    return m_files;
}

QHostAddress Smb4KClientBaseJob::lookupIpAddress(const QString &name)
{
    //
    // The IP address object
    //
    QHostAddress ipAddress;

    //
    // Get the IP address
    //
    // If the IP address is not to be determined for the local machine, we can use QHostInfo to
    // determine it. Otherwise we need to use QNetworkInterface for it.
    //
    if (name.toUpper() == QHostInfo::localHostName().toUpper() || name.toUpper() == machineNetbiosName().toUpper()) {
        // FIXME: Do we need to honor 'interfaces' here?
        QList<QHostAddress> addresses = QNetworkInterface::allAddresses();

        // Get the IP address for the host. For the time being, prefer the
        // IPv4 address over the IPv6 address.
        for (const QHostAddress &addr : qAsConst(addresses)) {
            // We only use global addresses.
            if (addr.isGlobal()) {
                if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                    ipAddress = addr;
                    break;
                } else if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
                    // FIXME: Use the right address here.
                    ipAddress = addr;
                }
            }
        }
    } else {
        // Get the IP address
        QHostInfo hostInfo = QHostInfo::fromName(name);

        if (hostInfo.error() == QHostInfo::NoError) {
            QList<QHostAddress> addresses = hostInfo.addresses();

            // Get the IP address for the host. For the time being, prefer the
            // IPv4 address over the IPv6 address.
            for (const QHostAddress &addr : qAsConst(addresses)) {
                // We only use global addresses.
                if (addr.isGlobal()) {
                    if (addr.protocol() == QAbstractSocket::IPv4Protocol) {
                        ipAddress = addr;
                        break;
                    } else if (addr.protocol() == QAbstractSocket::IPv6Protocol) {
                        // FIXME: Use the right address here.
                        ipAddress = addr;
                    }
                }
            }
        }
    }

    return ipAddress;
}

//
// Authentication function for libsmbclient
//
static void get_auth_data_with_context_fn(SMBCCTX *context,
                                          const char *server,
                                          const char *share,
                                          char *workgroup,
                                          int maxLenWorkgroup,
                                          char *username,
                                          int maxLenUsername,
                                          char *password,
                                          int maxLenPassword)
{
    if (context != nullptr) {
        Smb4KClientJob *job = static_cast<Smb4KClientJob *>(smbc_getOptionUserData(context));

        if (job) {
            job->get_auth_data_fn(server, share, workgroup, maxLenWorkgroup, username, maxLenUsername, password, maxLenPassword);
        }
    }
}

//
// Client job
//
Smb4KClientJob::Smb4KClientJob(QObject *parent)
    : Smb4KClientBaseJob(parent)
{
}

Smb4KClientJob::~Smb4KClientJob()
{
}

void Smb4KClientJob::start()
{
    QTimer::singleShot(50, this, SLOT(slotStartJob()));
    connect(this, &KJob::finished, this, &Smb4KClientJob::slotFinishJob);
}

void Smb4KClientJob::setPrintFileItem(const KFileItem &item)
{
    m_fileItem = item;
}

KFileItem Smb4KClientJob::printFileItem() const
{
    return m_fileItem;
}

void Smb4KClientJob::setPrintCopies(int copies)
{
    m_copies = copies;
}

int Smb4KClientJob::printCopies() const
{
    return m_copies;
}

void Smb4KClientJob::get_auth_data_fn(const char *server,
                                      const char * /*share*/,
                                      char *workgroup,
                                      int /*maxLenWorkgroup*/,
                                      char *username,
                                      int maxLenUsername,
                                      char *password,
                                      int maxLenPassword)
{
    //
    // Authentication
    //
    switch ((*pNetworkItem)->type()) {
    case Network: {
        //
        // No authentication needed
        //
        break;
    }
    case Workgroup: {
        //
        // Only request authentication data, if the master browsers require
        // authentication data.
        //
        if (Smb4KSettings::masterBrowsersRequireAuth()) {
            if (QString::fromUtf8(server).toUpper() != QString::fromUtf8(workgroup).toUpper()) {
                //
                // This is the master browser. Create a host object for it.
                //
                HostPtr masterBrowser = HostPtr(new Smb4KHost());
                masterBrowser->setWorkgroupName(QString::fromUtf8(workgroup));
                masterBrowser->setHostName(QString::fromUtf8(server));

                //
                // Get the authentication data
                //
                Smb4KWalletManager::self()->readLoginCredentials(masterBrowser);

                //
                // Copy the authentication data
                //
                if (masterBrowser->hasUserInfo()) {
                    qstrncpy(username, masterBrowser->userName().toUtf8().data(), maxLenUsername);
                    qstrncpy(password, masterBrowser->password().toUtf8().data(), maxLenPassword);
                }
            }
        }

        break;
    }
    case Host: {
        //
        // The host object
        //
        HostPtr host = (*pNetworkItem).staticCast<Smb4KHost>();

        //
        // Get the authentication data
        //
        Smb4KWalletManager::self()->readLoginCredentials(host);

        //
        // Copy the authentication data
        //
        if (host->hasUserInfo()) {
            qstrncpy(username, host->userName().toUtf8().data(), maxLenUsername);
            qstrncpy(password, host->password().toUtf8().data(), maxLenPassword);
        }

        break;
    }
    case Share: {
        //
        // The share object
        //
        SharePtr share = (*pNetworkItem).staticCast<Smb4KShare>();

        //
        // Get the authentication data
        //
        Smb4KWalletManager::self()->readLoginCredentials(share);

        //
        // Copy the authentication data
        //
        if (share->hasUserInfo()) {
            qstrncpy(username, share->userName().toUtf8().data(), maxLenUsername);
            qstrncpy(password, share->password().toUtf8().data(), maxLenPassword);
        }

        break;
    }
    case Directory: {
        //
        // The file object
        //
        FilePtr file = (*pNetworkItem).staticCast<Smb4KFile>();

        //
        // Create a share object
        //
        SharePtr share = SharePtr(new Smb4KShare());
        share->setWorkgroupName(file->workgroupName());
        share->setHostName(file->hostName());
        share->setShareName(file->shareName());
        share->setUserName(file->userName());
        share->setPassword(file->password());

        //
        // Get the authentication data
        //
        Smb4KWalletManager::self()->readLoginCredentials(share);

        //
        // Copy the authentication data
        //
        if (share->hasUserInfo()) {
            qstrncpy(username, share->userName().toUtf8().data(), maxLenUsername);
            qstrncpy(password, share->password().toUtf8().data(), maxLenPassword);
        }

        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KClientJob::initClientLibrary()
{
    //
    // Get new context
    //
    m_context = smbc_new_context();

    if (!m_context) {
        int errorCode = errno;

        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));

        emitResult();
        return;
    }

    //
    // Init the context
    //
    m_context = smbc_init_context(m_context);

    if (!m_context) {
        int errorCode = errno;

        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));
        emitResult();
        return;
    }

    //
    // Get the custom options
    //
    OptionsPtr options = Smb4KCustomOptionsManager::self()->findOptions(*pNetworkItem);

    //
    // Set debug level
    //
    smbc_setDebug(m_context, SMBC_DEBUG);

    //
    // Set the NetBIOS name and the workgroup to make connections
    //
    switch ((*pNetworkItem)->type()) {
    case Network: {
        //
        // We do not know about the servers and the domains/workgroups
        // present. So, do not set anything and use the default.
        //
        break;
    }
    case Workgroup: {
        WorkgroupPtr workgroup = (*pNetworkItem).staticCast<Smb4KWorkgroup>();

        //
        // Only set the NetBIOS name, if the workgroup entry has a master browser
        //
        if (workgroup->hasMasterBrowser()) {
            smbc_setNetbiosName(m_context, workgroup->masterBrowserName().toUtf8().data());
        }

        //
        // In case this domain/workgroup was discovered by the DNS-SD service, the workgroup/domain
        // might not be identical with the one defined in the network neighborhood. Thus, only set
        // the workgroup if no DNS-SD discovery was used.
        //
        if (!workgroup->dnsDiscovered()) {
            smbc_setWorkgroup(m_context, workgroup->workgroupName().toUtf8().data());
        }

        break;
    }
    case Host: {
        //
        // In case the domain/workgroup was discovered by the DNS-SD service, the
        // workgroup/domain might not be identical with the one defined in the network
        // neighborhood. Thus, only set the workgroup if no DNS-SD discovery was used.
        //
        HostPtr host = (*pNetworkItem).staticCast<Smb4KHost>();
        WorkgroupPtr workgroup = findWorkgroup(host->workgroupName());

        if (workgroup && !workgroup->dnsDiscovered()) {
            smbc_setWorkgroup(m_context, host->workgroupName().toUtf8().data());
        }

        //
        // Set the NetBIOS name
        //
        smbc_setNetbiosName(m_context, host->hostName().toUtf8().data());

        break;
    }
    case Share: {
        //
        // In case the domain/workgroup was discovered by the DNS-SD service, the
        // workgroup/domain might not be identical with the one defined in the network
        // neighborhood. Thus, only set the workgroup if no DNS-SD discovery was used.
        //
        SharePtr share = (*pNetworkItem).staticCast<Smb4KShare>();
        WorkgroupPtr workgroup = findWorkgroup(share->workgroupName());

        if (workgroup && !workgroup->dnsDiscovered()) {
            smbc_setWorkgroup(m_context, share->workgroupName().toUtf8().data());
        }

        //
        // Set the NetBIOS name
        //
        smbc_setNetbiosName(m_context, share->hostName().toUtf8().data());

        break;
    }
    case Directory: {
        //
        // In case the domain/workgroup was discovered by the DNS-SD service, the
        // workgroup/domain might not be identical with the one defined in the network
        // neighborhood. Thus, only set the workgroup if no DNS-SD discovery was used.
        //
        FilePtr file = (*pNetworkItem).staticCast<Smb4KFile>();
        WorkgroupPtr workgroup = findWorkgroup(file->workgroupName());

        if (workgroup && !workgroup->dnsDiscovered()) {
            smbc_setWorkgroup(m_context, file->workgroupName().toUtf8().data());
        }

        //
        // Set the NetBIOS name
        //
        smbc_setNetbiosName(m_context, file->hostName().toUtf8().data());

        break;
    }
    default: {
        break;
    }
    }

    //
    // Set the user for making the connection
    //
    // To be able to connect to a Windows 10 server and get the list
    // of shared resources, use the 'guest' user here, if the URL
    // does not provide a user name.
    //
    if (!(*pNetworkItem)->url().userName().isEmpty()) {
        smbc_setUser(m_context, (*pNetworkItem)->url().userName().toUtf8().data());
    } else {
        smbc_setUser(m_context, "guest");
    }

    //
    // Set the port
    //
    if (options) {
        if (options->useSmbPort()) {
            smbc_setPort(m_context, options->smbPort());
        } else {
            smbc_setPort(m_context, 0 /* use the default */);
        }
    } else {
        if (Smb4KSettings::useRemoteSmbPort()) {
            smbc_setPort(m_context, Smb4KSettings::remoteSmbPort());
        } else {
            smbc_setPort(m_context, 0 /* use the default */);
        }
    }

    //
    // Set the user data (this class)
    //
    smbc_setOptionUserData(m_context, this);

    //
    // Set number of master browsers to be used
    //
    if (Smb4KSettings::largeNetworkNeighborhood()) {
        smbc_setOptionBrowseMaxLmbCount(m_context, 3);
    } else {
        smbc_setOptionBrowseMaxLmbCount(m_context, 0 /* all master browsers */);
    }

    //
    // Set the protocol version if desired
    //
    if (Smb4KSettings::forceSmb1Protocol() && (*pNetworkItem)->type() == Network) {
        smbc_setOptionProtocols(m_context, "NT1", "NT1");
    } else {
        int minimal = -1;
        int maximal = -1;
        QString minimalClientProtocolVersionString, maximalClientProtocolVersionString;

        if (options) {
            if (options->useClientProtocolVersions()) {
                minimal = options->minimalClientProtocolVersion();
                maximal = options->maximalClientProtocolVersion();
            }
        } else {
            if (Smb4KSettings::useClientProtocolVersions()) {
                minimal = Smb4KSettings::minimalClientProtocolVersion();
                maximal = Smb4KSettings::maximalClientProtocolVersion();
            }
        }

        if (minimal != -1 && maximal != -1) {
            switch (minimal) {
            case Smb4KSettings::EnumMinimalClientProtocolVersion::NT1: {
                minimalClientProtocolVersionString = QStringLiteral("NT1");
                break;
            }
            case Smb4KSettings::EnumMinimalClientProtocolVersion::SMB2: {
                minimalClientProtocolVersionString = QStringLiteral("SMB2");
                break;
            }
            case Smb4KSettings::EnumMinimalClientProtocolVersion::SMB3: {
                minimalClientProtocolVersionString = QStringLiteral("SMB3");
                break;
            }
            default: {
                break;
            }
            }

            switch (Smb4KSettings::maximalClientProtocolVersion()) {
            case Smb4KSettings::EnumMaximalClientProtocolVersion::NT1: {
                maximalClientProtocolVersionString = QStringLiteral("NT1");
                break;
            }
            case Smb4KSettings::EnumMaximalClientProtocolVersion::SMB2: {
                maximalClientProtocolVersionString = QStringLiteral("SMB2");
                break;
            }
            case Smb4KSettings::EnumMaximalClientProtocolVersion::SMB3: {
                maximalClientProtocolVersionString = QStringLiteral("SMB3");
                break;
            }
            default: {
                break;
            }
            }
        }

        if (!minimalClientProtocolVersionString.isEmpty() && !maximalClientProtocolVersionString.isEmpty()) {
            smbc_setOptionProtocols(m_context, minimalClientProtocolVersionString.toLatin1().data(), maximalClientProtocolVersionString.toLatin1().data());
        } else {
            smbc_setOptionProtocols(m_context, nullptr, nullptr);
        }
    }

    //
    // Set the encryption level
    //
    if (Smb4KSettings::useEncryptionLevel()) {
        switch (Smb4KSettings::encryptionLevel()) {
        case Smb4KSettings::EnumEncryptionLevel::None: {
            smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_NONE);
            break;
        }
        case Smb4KSettings::EnumEncryptionLevel::Request: {
            smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUEST);
            break;
        }
        case Smb4KSettings::EnumEncryptionLevel::Require: {
            smbc_setOptionSmbEncryptionLevel(m_context, SMBC_ENCRYPTLEVEL_REQUIRE);
            break;
        }
        default: {
            break;
        }
        }
    }

    //
    // Set the usage of anonymous login
    //
    smbc_setOptionNoAutoAnonymousLogin(m_context, false);

    //
    // Set the usage of the winbind ccache
    //
    smbc_setOptionUseCCache(m_context, Smb4KSettings::useWinbindCCache());

    //
    // Set usage of Kerberos
    //
    if (options) {
        smbc_setOptionUseKerberos(m_context, options->useKerberos());
    } else {
        smbc_setOptionUseKerberos(m_context, Smb4KSettings::useKerberos());
    }

    smbc_setOptionFallbackAfterKerberos(m_context, 1);

    //
    // Set the channel for debug output
    //
    smbc_setOptionDebugToStderr(m_context, 1);

    //
    // Set auth callback function
    //
    smbc_setFunctionAuthDataWithContext(m_context, get_auth_data_with_context_fn);
}

void Smb4KClientJob::doLookups()
{
    //
    // Set the new context
    //
    (void)smbc_set_context(m_context);

    //
    // Get the function to open the directory.
    //
    smbc_opendir_fn openDirectory = smbc_getFunctionOpendir(m_context);

    if (!openDirectory) {
        int errorCode = errno;
        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));
        return;
    }

    //
    // Open the directory
    //
    // If we use DNS-SD, the workgroup/domain name will most likely be unknown
    // for Samba and the following function fails. Since we do not want the lookup
    // to stop here in that case, do not throw an error when using DNS-SD and
    // Network and Workgroup (parent) items.
    //
    SMBCFILE *directory = openDirectory(m_context, (*pNetworkItem)->url().toString().toUtf8().data());

    if (!directory) {
        if (!(*pNetworkItem)->dnsDiscovered() && !((*pNetworkItem)->type() == Network || (*pNetworkItem)->type() == Workgroup)) {
            int errorCode = errno;

            switch (errorCode) {
            case EACCES:
            case EPERM: {
                setError(AccessDeniedError);
                setErrorText(QString::fromUtf8(strerror(errorCode)));
                break;
            }
            case ENOENT: {
                if ((*pNetworkItem)->type() != Network) {
                    setError(ClientError);
                    setErrorText(QString::fromUtf8(strerror(errorCode)));
                }
                break;
            }
            default: {
                setError(ClientError);
                setErrorText(QString::fromUtf8(strerror(errorCode)));
                break;
            }
            }
        }

        return;
    }

    //
    // Read the directory
    //
    struct smbc_dirent *directoryEntry = nullptr;
    smbc_readdir_fn readDirectory = smbc_getFunctionReaddir(m_context);

    if (!readDirectory) {
        int errorCode = errno;
        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));
        return;
    }

    while ((directoryEntry = readDirectory(m_context, directory)) != nullptr) {
        switch (directoryEntry->smbc_type) {
        case SMBC_WORKGROUP: {
            //
            // Create a workgroup pointer
            //
            WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());

            //
            // Set the workgroup name
            //
            QString workgroupName = QString::fromUtf8(directoryEntry->name);
            workgroup->setWorkgroupName(workgroupName);

            //
            // Set the master browser
            //
            QString masterBrowserName = QString::fromUtf8(directoryEntry->comment);
            workgroup->setMasterBrowserName(masterBrowserName);

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress(masterBrowserName);

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip the
            // workgroup and delete the pointer.
            //
            if (!address.isNull()) {
                workgroup->setMasterBrowserIpAddress(address);
                *pWorkgroups << workgroup;
            } else {
                workgroup.clear();
            }

            break;
        }
        case SMBC_SERVER: {
            //
            // Create a host pointer
            //
            HostPtr host = HostPtr(new Smb4KHost());

            //
            // Set the workgroup name
            //
            host->setWorkgroupName((*pNetworkItem)->url().host());

            //
            // Set the host name
            //
            QString hostName = QString::fromUtf8(directoryEntry->name);
            host->setHostName(hostName);

            //
            // Set the comment
            //
            QString comment = QString::fromUtf8(directoryEntry->comment);
            host->setComment(comment);

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress(hostName);

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            //
            if (!address.isNull()) {
                host->setIpAddress(address);
                *pHosts << host;
            } else {
                host.clear();
            }

            break;
        }
        case SMBC_FILE_SHARE: {
            //
            // Create a share pointer
            //
            SharePtr share = SharePtr(new Smb4KShare());

            //
            // Set the workgroup name
            //
            share->setWorkgroupName((*pNetworkItem).staticCast<Smb4KHost>()->workgroupName());

            //
            // Set the host name
            //
            share->setHostName((*pNetworkItem)->url().host());

            //
            // Set the share name
            //
            share->setShareName(QString::fromUtf8(directoryEntry->name));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment));

            //
            // Set share type
            //
            share->setShareType(FileShare);

            //
            // Set the authentication data
            //
            share->setUserName((*pNetworkItem)->url().userName());
            share->setPassword((*pNetworkItem)->url().password());

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress((*pNetworkItem)->url().host());

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            //
            if (!address.isNull()) {
                share->setHostIpAddress(address);
                *pShares << share;
            } else {
                share.clear();
            }

            break;
        }
        case SMBC_PRINTER_SHARE: {
            //
            // Create a share pointer
            //
            SharePtr share = SharePtr(new Smb4KShare());

            //
            // Set the workgroup name
            //
            share->setWorkgroupName((*pNetworkItem).staticCast<Smb4KHost>()->workgroupName());

            //
            // Set the host name
            //
            share->setHostName((*pNetworkItem)->url().host());

            //
            // Set the share name
            //
            share->setShareName(QString::fromUtf8(directoryEntry->name));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment));

            //
            // Set share type
            //
            share->setShareType(PrinterShare);

            //
            // Set the authentication data
            //
            share->setUserName((*pNetworkItem)->url().userName());
            share->setPassword((*pNetworkItem)->url().password());

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress((*pNetworkItem)->url().host());

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            //
            if (!address.isNull()) {
                share->setHostIpAddress(address);
                *pShares << share;
            } else {
                share.clear();
            }

            break;
        }
        case SMBC_IPC_SHARE: {
            //
            // Create a share pointer
            //
            SharePtr share = SharePtr(new Smb4KShare());

            //
            // Set the workgroup name
            //
            share->setWorkgroupName((*pNetworkItem).staticCast<Smb4KHost>()->workgroupName());

            //
            // Set the host name
            //
            share->setHostName((*pNetworkItem)->url().host());

            //
            // Set the share name
            //
            share->setShareName(QString::fromUtf8(directoryEntry->name));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment));

            //
            // Set share type
            //
            share->setShareType(IpcShare);

            //
            // Set the authentication data
            //
            share->setUserName((*pNetworkItem)->url().userName());
            share->setPassword((*pNetworkItem)->url().password());

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress((*pNetworkItem)->url().host());

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            //
            if (!address.isNull()) {
                share->setHostIpAddress(address);
                *pShares << share;
            } else {
                share.clear();
            }

            break;
        }
        case SMBC_DIR: {
            //
            // Do not process '.' and '..' directories
            //
            QString name = QString::fromUtf8(directoryEntry->name);

            if (name != QStringLiteral(".") && name != QStringLiteral("..")) {
                //
                // Create the URL for the discovered item
                //
                QUrl u = (*pNetworkItem)->url();
                u.setPath((*pNetworkItem)->url().path() + QDir::separator() + QString::fromUtf8(directoryEntry->name));

                //
                // We do not stat directories. Directly create the directory object
                //
                FilePtr dir = FilePtr(new Smb4KFile(u, Directory));

                //
                // Set the workgroup name
                //
                dir->setWorkgroupName((*pNetworkItem).staticCast<Smb4KShare>()->workgroupName());

                //
                // Set the authentication data
                //
                dir->setUserName((*pNetworkItem)->url().userName());
                dir->setPassword((*pNetworkItem)->url().password());

                //
                // Lookup IP address
                //
                QHostAddress address = lookupIpAddress((*pNetworkItem)->url().host());

                //
                // Process the IP address.
                // If the address is null, the server most likely went offline. So, skip it
                // and delete the pointer.
                //
                if (!address.isNull()) {
                    dir->setHostIpAddress(address);
                    *pFiles << dir;
                } else {
                    dir.clear();
                }
            }

            break;
        }
        case SMBC_FILE: {
            //
            // Create the URL for the discovered item
            //
            QUrl u = (*pNetworkItem)->url();
            u.setPath((*pNetworkItem)->url().path() + QDir::separator() + QString::fromUtf8(directoryEntry->name));

            //
            // Create the directory object
            //
            FilePtr dir = FilePtr(new Smb4KFile(u, File));

            //
            // Set the workgroup name
            //
            dir->setWorkgroupName((*pNetworkItem).staticCast<Smb4KShare>()->workgroupName());

            //
            // Stat the file
            //
            // FIXME

            //
            // Set the authentication data
            //
            dir->setUserName((*pNetworkItem)->url().userName());
            dir->setPassword((*pNetworkItem)->url().password());

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress((*pNetworkItem)->url().host());

            //
            // Process the IP address.
            // If the address is null, the server most likely went offline. So, skip it
            // and delete the pointer.
            //
            if (!address.isNull()) {
                dir->setHostIpAddress(address);
                *pFiles << dir;
            } else {
                dir.clear();
            }

            break;
        }
        case SMBC_LINK: {
            qDebug() << "Processing links is not implemented.";
            qDebug() << directoryEntry->name;
            qDebug() << directoryEntry->comment;
            break;
        }
        default: {
            qDebug() << "Need to process network item " << directoryEntry->name;
            break;
        }
        }
    }

    //
    // Close the directory
    //
    smbc_closedir_fn closeDirectory = smbc_getFunctionClosedir(m_context);

    if (!closeDirectory) {
        int errorCode = errno;

        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));
        return;
    }

    (void)closeDirectory(m_context, directory);
}

void Smb4KClientJob::doPrinting()
{
    //
    // Set the new context
    //
    (void)smbc_set_context(m_context);

    //
    // The URL that is to be printed
    //
    QUrl fileUrl;

    //
    // Set the temporary directory
    //
    QTemporaryDir tempDir;

    //
    // Check if we can directly print the file
    //
    if (m_fileItem.mimetype() == QStringLiteral("application/postscript") || m_fileItem.mimetype() == QStringLiteral("application/pdf")
        || m_fileItem.mimetype().startsWith(QStringLiteral("image"))) {
        //
        // Set the URL to the incoming file
        //
        fileUrl = m_fileItem.url();
    } else if (m_fileItem.mimetype() == QStringLiteral("application/x-shellscript") || m_fileItem.mimetype().startsWith(QStringLiteral("text"))
               || m_fileItem.mimetype().startsWith(QStringLiteral("message"))) {
        //
        // Set a printer object
        //
        QPrinter printer(QPrinter::HighResolution);
        printer.setCreator(QStringLiteral("Smb4K"));
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(tempDir.path() + QDir::separator() + QStringLiteral("smb4k_print.pdf"));

        //
        // Open the file that is to be printed and read it
        //
        QStringList contents;

        QFile file(m_fileItem.url().path());

        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream ts(&file);

            while (!ts.atEnd()) {
                contents << ts.readLine();
            }
        } else {
            return;
        }

        //
        // Convert the file to PDF
        //
        QTextDocument doc;

        if (m_fileItem.mimetype().endsWith(QStringLiteral("html"))) {
            doc.setHtml(contents.join(QStringLiteral(" ")));
        } else {
            doc.setPlainText(contents.join(QStringLiteral("\n")));
        }

        doc.print(&printer);

        //
        // Set the URL to the converted file
        //
        fileUrl.setUrl(printer.outputFileName());
        fileUrl.setScheme(QStringLiteral("file"));
    } else {
        Smb4KNotification::mimetypeNotSupported(m_fileItem.mimetype());
        return;
    }

    //
    // Get the open function for the printer
    //
    smbc_open_print_job_fn openPrinter = smbc_getFunctionOpenPrintJob(m_context);

    if (!openPrinter) {
        int errorCode = errno;
        setError(ClientError);
        setErrorText(QString::fromUtf8(strerror(errorCode)));
        return;
    }

    //
    // Open the printer for printing
    //
    SMBCFILE *printer = openPrinter(m_context, (*pNetworkItem)->url().toString().toUtf8().data());

    if (!printer) {
        int errorCode = errno;

        switch (errorCode) {
        case EACCES: {
            setError(AccessDeniedError);
            setErrorText(QString::fromUtf8(strerror(errorCode)));
            break;
        }
        default: {
            setError(ClientError);
            setErrorText(QString::fromUtf8(strerror(errorCode)));
            break;
        }
        }

        return;
    }

    //
    // Open the file
    //
    QFile file(fileUrl.path());

    if (!file.open(QFile::ReadOnly)) {
        setError(FileAccessError);
        setErrorText(i18n("The file %1 could not be read", fileUrl.path()));
        return;
    }

    //
    // Write X copies of the file to the printer
    //
    char buffer[4096];
    qint64 bytes = 0;
    int copy = 0;

    while (copy < m_copies) {
        while ((bytes = file.read(buffer, sizeof(buffer))) > 0) {
            smbc_write_fn writeFile = smbc_getFunctionWrite(m_context);

            if (writeFile(m_context, printer, buffer, bytes) < 0) {
                setError(PrintFileError);
                setErrorText(i18n("The file %1 could not be printed to %2", fileUrl.path(), (*pNetworkItem).staticCast<Smb4KShare>()->displayString()));

                smbc_close_fn closePrinter = smbc_getFunctionClose(m_context);
                closePrinter(m_context, printer);
            }
        }

        copy++;
    }

    //
    // Close the printer
    //
    smbc_close_fn closePrinter = smbc_getFunctionClose(m_context);
    closePrinter(m_context, printer);
}

void Smb4KClientJob::slotStartJob()
{
    //
    // Initialize the client library
    //
    initClientLibrary();

    //
    // Process the given URL according to the passed process
    //
    switch (*pProcess) {
    case LookupDomains:
    case LookupDomainMembers:
    case LookupShares:
    case LookupFiles: {
        //
        // Do lookups using the client library
        //
        doLookups();
        break;
    }
    case PrintFile: {
        //
        // Print files using the client library
        //
        doPrinting();
        break;
    }
    default: {
        break;
    }
    }

    //
    // Emit the result
    //
    emitResult();
}

void Smb4KClientJob::slotFinishJob()
{
    if (m_context != nullptr) {
        smbc_free_context(m_context, 1);
    }
}


Smb4KDnsDiscoveryJob::Smb4KDnsDiscoveryJob(QObject *parent)
    : Smb4KClientBaseJob(parent)
{
    //
    // Set up the DNS-SD browser
    //
    m_serviceBrowser = new KDNSSD::ServiceBrowser(QStringLiteral("_smb._tcp"));

    //
    // Connections
    //
    connect(m_serviceBrowser, SIGNAL(serviceAdded(KDNSSD::RemoteService::Ptr)), this, SLOT(slotServiceAdded(KDNSSD::RemoteService::Ptr)));
    connect(m_serviceBrowser, SIGNAL(finished()), this, SLOT(slotFinished()));
}

Smb4KDnsDiscoveryJob::~Smb4KDnsDiscoveryJob()
{
    delete m_serviceBrowser;
}

void Smb4KDnsDiscoveryJob::start()
{
    QTimer::singleShot(50, this, SLOT(slotStartJob()));
}

void Smb4KDnsDiscoveryJob::slotStartJob()
{
    m_serviceBrowser->startBrowse();
}

void Smb4KDnsDiscoveryJob::slotServiceAdded(KDNSSD::RemoteService::Ptr service)
{
    switch (*pProcess) {
    case LookupDomains: {
        //
        // Check if the workgroup/domain is already known
        //
        bool foundWorkgroup = false;

        for (const WorkgroupPtr &w : qAsConst(*pWorkgroups)) {
            if (QString::compare(w->workgroupName(), service->domain(), Qt::CaseInsensitive) == 0) {
                foundWorkgroup = true;
                break;
            }
        }

        //
        // If the workgroup is not known yet, add it to the list
        //
        if (!foundWorkgroup) {
            //
            // Create the workgroup item
            //
            WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());

            //
            // Set the _DNS-SD_ domain name
            //
            workgroup->setWorkgroupName(service->domain());

            //
            // Tell the program that the workgroup was discovered by DNS-SD
            //
            workgroup->setDnsDiscovered(true);

            //
            // Add the workgroup
            //
            *pWorkgroups << workgroup;
        }
        break;
    }
    case LookupDomainMembers: {
        //
        // Check if the server is already known
        //
        bool foundServer = false;

        for (const HostPtr &h : qAsConst(*pHosts)) {
            //
            // On a local network there will most likely be no two servers with
            // identical name, thus, to avoid duplicates, only test the hostname
            // here.
            //
            if (QString::compare(h->hostName(), service->serviceName(), Qt::CaseInsensitive) == 0) {
                foundServer = true;
                break;
            }
        }

        //
        // If the server is not known yet, add it to the list
        //
        if (!foundServer) {
            //
            // Create the host item
            //
            HostPtr host = HostPtr(new Smb4KHost());

            //
            // Set the _DNS-SD_ host name
            //
            host->setHostName(service->serviceName());

            //
            // Set the _DNS-SD_ domain name
            //
            host->setWorkgroupName(service->domain());

            //
            // Tell the program that the host was discovered by DNS-SD
            //
            host->setDnsDiscovered(true);

            //
            // Lookup IP address
            //
            QHostAddress address = lookupIpAddress(service->serviceName());

            //
            // Process the IP address.
            //
            if (!address.isNull()) {
                host->setIpAddress(address);
            }

            //
            // Add the host
            //
            *pHosts << host;
        }

        break;
    }
    default: {
        break;
    }
    }
}

void Smb4KDnsDiscoveryJob::slotFinished()
{
    emitResult();
}

#ifdef USE_WS_DISCOVERY
Smb4KWsDiscoveryJob::Smb4KWsDiscoveryJob(QObject *parent)
    : Smb4KClientBaseJob(parent)
{
    //
    // The WS Discovery client
    //
    m_discoveryClient = new WSDiscoveryClient(this);

    //
    // The timer used to stop the scan
    //
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(2000);

    //
    // Connections
    //
    connect(m_discoveryClient,
            SIGNAL(probeMatchReceived(const WSDiscoveryTargetService &)),
            this,
            SLOT(slotProbeMatchReceived(const WSDiscoveryTargetService &)));
    connect(m_discoveryClient,
            SIGNAL(resolveMatchReceived(const WSDiscoveryTargetService &)),
            this,
            SLOT(slotResolveMatchReceived(const WSDiscoveryTargetService &)));

    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotDiscoveryFinished()));
}

Smb4KWsDiscoveryJob::~Smb4KWsDiscoveryJob()
{
}

void Smb4KWsDiscoveryJob::start()
{
    QTimer::singleShot(50, this, SLOT(slotStartJob()));
}

void Smb4KWsDiscoveryJob::slotStartJob()
{
    //
    // Start the client
    //
    m_discoveryClient->start();

    //
    // Define the type
    //
    KDQName type(QStringLiteral("wsdp:Device"));
    type.setNameSpace(QStringLiteral("http://schemas.xmlsoap.org/ws/2006/02/devprof"));

    //
    // Send the probe
    //
    m_discoveryClient->sendProbe({type}, {});

    //
    // Start the timer
    //
    m_timer->start();
}

void Smb4KWsDiscoveryJob::slotProbeMatchReceived(const WSDiscoveryTargetService &service)
{
    //
    // Stop the timer
    //
    m_timer->stop();

    //
    // If there is no address, we need to resolve it. Otherwise,
    // resolve the available addresses and add the discovered network
    // items to the respective lists.
    //
    if (service.xAddrList().isEmpty()) {
        m_discoveryClient->sendResolve(service.endpointReference());
    } else {
        if (!service.xAddrList().isEmpty()) {
            for (const QUrl &address : service.xAddrList()) {
                KDSoapClientInterface clientInterface(address.toString(), QStringLiteral("http://schemas.xmlsoap.org/ws/2004/09/transfer"));
                clientInterface.setSoapVersion(KDSoapClientInterface::SoapVersion::SOAP1_2);
                clientInterface.setTimeout(5000);

                KDSoapMessage soapMessage;
                KDSoapMessageAddressingProperties soapMessageProperties;
                soapMessageProperties.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200408);
                soapMessageProperties.setAction(QStringLiteral("http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"));
                soapMessageProperties.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces));
                soapMessageProperties.setDestination(service.endpointReference());
                soapMessageProperties.setReplyEndpointAddress(
                    KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous,
                                                                                 KDSoapMessageAddressingProperties::Addressing200408));
                soapMessageProperties.setSourceEndpointAddress(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces));
                soapMessage.setMessageAddressingProperties(soapMessageProperties);

                KDSoapMessage response = clientInterface.call(QString(), soapMessage);

                if (!response.isFault()) {
                    KDSoapValueList childValues = response.childValues();

                    for (const KDSoapValue &value : qAsConst(childValues)) {
                        QString entry = value.childValues()
                                            .child(QStringLiteral("Relationship"))
                                            .childValues()
                                            .child(QStringLiteral("Host"))
                                            .childValues()
                                            .child(QStringLiteral("Computer"))
                                            .value()
                                            .toString();

                        switch (*pProcess) {
                        case LookupDomains: {
                            //
                            // Get the name of the workgroup or domain
                            //
                            QString workgroupName = entry.section(QStringLiteral(":"), 1, -1);

                            //
                            // Work around an empty workgroup/domain name. Use the "LOCAL" domain from
                            // DNS-SD for that.
                            //
                            if (workgroupName.isEmpty()) {
                                workgroupName = QStringLiteral("LOCAL");
                            }

                            //
                            // Process the workgroup name. Only add a new workgroup, if it
                            // is not present already.
                            //
                            bool foundWorkgroup = false;

                            for (const WorkgroupPtr &w : qAsConst(*pWorkgroups)) {
                                if (QString::compare(w->workgroupName(), workgroupName, Qt::CaseInsensitive) == 0) {
                                    foundWorkgroup = true;
                                    break;
                                }
                            }

                            //
                            // If the workgroup is unknown, add it to the list
                            //
                            if (!foundWorkgroup) {
                                //
                                // Create the workgroup object
                                //
                                WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());

                                //
                                // Set the workgroup/domain name
                                //
                                workgroup->setWorkgroupName(workgroupName);

                                //
                                // Add the workgroup
                                //
                                *pWorkgroups << workgroup;
                            }

                            break;
                        }
                        case LookupDomainMembers: {
                            //
                            // Get the workgroup name
                            //
                            QString workgroupName = entry.section(QStringLiteral(":"), 1, -1);

                            //
                            // Work around an empty workgroup/domain name. Use the "LOCAL" domain from
                            // DNS-SD for that.
                            //
                            if (workgroupName.isEmpty()) {
                                workgroupName = QStringLiteral("LOCAL");
                            }

                            //
                            // Get the host name. Unfortunately, the delimiter depends on
                            // whether the host is member of a workgroup (/) or domain (\).
                            //
                            QString hostName;

                            if (entry.contains(QStringLiteral("/"))) {
                                hostName = entry.section(QStringLiteral("/"), 0, 0);
                            } else if (entry.contains(QStringLiteral("\\"))) {
                                hostName = entry.section(QStringLiteral("\\"), 0, 0);
                            }

                            //
                            // Process the host name. Only add a new host, if it
                            // is not present already.
                            //
                            if (!hostName.isEmpty()) {
                                //
                                // Check if the server is already known
                                //
                                bool foundServer = false;

                                for (const HostPtr &h : qAsConst(*pHosts)) {
                                    if (QString::compare(h->hostName(), hostName, Qt::CaseInsensitive) == 0
                                        && QString::compare(h->workgroupName(), workgroupName, Qt::CaseInsensitive) == 0) {
                                        foundServer = true;
                                        break;
                                    }
                                }

                                //
                                // If the server is unknown, add it to the list
                                //
                                if (!foundServer) {
                                    //
                                    // Create the host object
                                    //
                                    HostPtr host = HostPtr(new Smb4KHost());

                                    //
                                    // Set the workgroup/domain name
                                    //
                                    host->setWorkgroupName(workgroupName);

                                    //
                                    // Set the host name
                                    //
                                    host->setHostName(hostName);

                                    //
                                    // Lookup IP address
                                    //
                                    QHostAddress address = lookupIpAddress(hostName);

                                    //
                                    // Process the IP address.
                                    //
                                    if (!address.isNull()) {
                                        host->setIpAddress(address);
                                    }

                                    //
                                    // Add the host
                                    //
                                    *pHosts << host;
                                }
                            }

                            break;
                        }
                        default: {
                            break;
                        }
                        }
                    }
                }
            }
        }
    }

    //
    // Restart the timer
    //
    m_timer->start();
}

void Smb4KWsDiscoveryJob::slotResolveMatchReceived(const WSDiscoveryTargetService &service)
{
    //
    // Stop the timer
    //
    m_timer->stop();

    //
    // If there are addresses available, resolve them and add the
    // discovered network items to the respective lists.
    //
    if (!service.xAddrList().isEmpty()) {
        for (const QUrl &address : service.xAddrList()) {
            KDSoapClientInterface clientInterface(address.toString(), QStringLiteral("http://schemas.xmlsoap.org/ws/2004/09/transfer"));
            clientInterface.setSoapVersion(KDSoapClientInterface::SoapVersion::SOAP1_2);
            clientInterface.setTimeout(5000);

            KDSoapMessage soapMessage;
            KDSoapMessageAddressingProperties soapMessageProperties;
            soapMessageProperties.setAddressingNamespace(KDSoapMessageAddressingProperties::Addressing200408);
            soapMessageProperties.setAction(QStringLiteral("http://schemas.xmlsoap.org/ws/2004/09/transfer/Get"));
            soapMessageProperties.setMessageID(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces));
            soapMessageProperties.setDestination(service.endpointReference());
            soapMessageProperties.setReplyEndpointAddress(
                KDSoapMessageAddressingProperties::predefinedAddressToString(KDSoapMessageAddressingProperties::Anonymous,
                                                                             KDSoapMessageAddressingProperties::Addressing200408));
            soapMessageProperties.setSourceEndpointAddress(QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces));
            soapMessage.setMessageAddressingProperties(soapMessageProperties);

            KDSoapMessage response = clientInterface.call(QString(), soapMessage);

            if (!response.isFault()) {
                KDSoapValueList childValues = response.childValues();

                for (const KDSoapValue &value : qAsConst(childValues)) {
                    QString entry = value.childValues()
                                        .child(QStringLiteral("Relationship"))
                                        .childValues()
                                        .child(QStringLiteral("Host"))
                                        .childValues()
                                        .child(QStringLiteral("Computer"))
                                        .value()
                                        .toString();

                    switch (*pProcess) {
                    case LookupDomains: {
                        //
                        // Get the name of the workgroup or domain
                        //
                        QString workgroupName = entry.section(QStringLiteral(":"), 1, -1);

                        //
                        // Work around an empty workgroup/domain name. Use the "LOCAL" domain from
                        // DNS-SD for that.
                        //
                        if (workgroupName.isEmpty()) {
                            workgroupName = QStringLiteral("LOCAL");
                        }

                        //
                        // Process the workgroup name. Only add a new workgroup, if it
                        // is not present already.
                        //
                        bool foundWorkgroup = false;

                        for (const WorkgroupPtr &w : qAsConst(*pWorkgroups)) {
                            if (QString::compare(w->workgroupName(), workgroupName, Qt::CaseInsensitive) == 0) {
                                foundWorkgroup = true;
                                break;
                            }
                        }

                        //
                        // If the workgroup is unknown, add it to the list
                        //
                        if (!foundWorkgroup) {
                            //
                            // Create the workgroup object
                            //
                            WorkgroupPtr workgroup = WorkgroupPtr(new Smb4KWorkgroup());

                            //
                            // Set the workgroup/domain name
                            //
                            workgroup->setWorkgroupName(workgroupName);

                            //
                            // Add the workgroup
                            //
                            *pWorkgroups << workgroup;
                        }

                        break;
                    }
                    case LookupDomainMembers: {
                        //
                        // Get the workgroup name
                        //
                        QString workgroupName = entry.section(QStringLiteral(":"), 1, -1);

                        //
                        // Work around an empty workgroup/domain name. Use the "LOCAL" domain from
                        // DNS-SD for that.
                        //
                        if (workgroupName.isEmpty()) {
                            workgroupName = QStringLiteral("LOCAL");
                        }

                        //
                        // Get the host name. Unfortunately, the delimiter depends on
                        // whether the host is member of a workgroup (/) or domain (\).
                        //
                        QString hostName;

                        if (entry.contains(QStringLiteral("/"))) {
                            hostName = entry.section(QStringLiteral("/"), 0, 0);
                        } else if (entry.contains(QStringLiteral("\\"))) {
                            hostName = entry.section(QStringLiteral("\\"), 0, 0);
                        }

                        //
                        // Process the host name. Only add a new host, if it
                        // is not present already.
                        //
                        if (!hostName.isEmpty()) {
                            //
                            // Check if the server is already known
                            //
                            bool foundServer = false;

                            for (const HostPtr &h : qAsConst(*pHosts)) {
                                if (QString::compare(h->hostName(), hostName, Qt::CaseInsensitive) == 0
                                    && QString::compare(h->workgroupName(), workgroupName, Qt::CaseInsensitive) == 0) {
                                    foundServer = true;
                                    break;
                                }
                            }

                            //
                            // If the server is unknown, add it to the list
                            //
                            if (!foundServer) {
                                //
                                // Create the host object
                                //
                                HostPtr host = HostPtr(new Smb4KHost());

                                //
                                // Set the workgroup/domain name
                                //
                                host->setWorkgroupName(workgroupName);

                                //
                                // Set the host name
                                //
                                host->setHostName(hostName);

                                //
                                // Lookup IP address
                                //
                                QHostAddress address = lookupIpAddress(hostName);

                                //
                                // Process the IP address.
                                //
                                if (!address.isNull()) {
                                    host->setIpAddress(address);
                                }

                                //
                                // Add the host
                                //
                                *pHosts << host;
                            }
                        }

                        break;
                    }
                    default: {
                        break;
                    }
                    }
                }
            }
        }
    }

    //
    // Restart the timer
    //
    m_timer->start();
}

void Smb4KWsDiscoveryJob::slotDiscoveryFinished()
{
    emitResult();
}
#endif

Smb4KPreviewDialog::Smb4KPreviewDialog(const SharePtr &share, QWidget *parent)
    : QDialog(parent)
    , m_share(share)
{
    //
    // Dialog title
    //
    setWindowTitle(i18n("Preview of %1", share->displayString()));

    //
    // Attributes
    //
    setAttribute(Qt::WA_DeleteOnClose, true);

    //
    // Layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    //
    // The list widget
    //
    QListWidget *listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(listWidget, SIGNAL(itemActivated(QListWidgetItem *)), SLOT(slotItemActivated(QListWidgetItem *)));

    layout->addWidget(listWidget, 0);

    //
    // Toolbar
    // Use QToolBar here with the settings suggested by the note provided in the 'Detailed Description'
    // section of KToolBar (https://api.kde.org/frameworks/kxmlgui/html/classKToolBar.html)
    //
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    toolBar->setProperty("otherToolbar", true);

    //
    // Reload / cancel action
    //
    KDualAction *reloadAction = new KDualAction(toolBar);
    reloadAction->setObjectName(QStringLiteral("reload_action"));
    reloadAction->setInactiveText(i18n("Reload"));
    reloadAction->setInactiveIcon(KDE::icon(QStringLiteral("view-refresh")));
    reloadAction->setActiveText(i18n("Abort"));
    reloadAction->setActiveIcon(KDE::icon(QStringLiteral("process-stop")));
    reloadAction->setActive(false);
    reloadAction->setAutoToggle(false);

    connect(reloadAction, SIGNAL(toggled(bool)), this, SLOT(slotReloadActionTriggered()));

    toolBar->addAction(reloadAction);

    //
    // Up action
    //
    QAction *upAction = toolBar->addAction(KDE::icon(QStringLiteral("go-up")), i18n("Up"), this, SLOT(slotUpActionTriggered()));
    upAction->setObjectName(QStringLiteral("up_action"));
    upAction->setEnabled(false);

    toolBar->addSeparator();

    //
    // URL combo box
    //
    KUrlComboBox *urlCombo = new KUrlComboBox(KUrlComboBox::Directories, toolBar);
    urlCombo->setEditable(false);
    toolBar->addWidget(urlCombo);
    connect(urlCombo, SIGNAL(urlActivated(QUrl)), this, SLOT(slotUrlActivated(QUrl)));

    layout->addWidget(toolBar, 0);

    //
    // Dialog button box
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    QPushButton *closeButton = buttonBox->addButton(QDialogButtonBox::Close);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(slotClosingDialog()));

    layout->addWidget(buttonBox, 0);

    //
    // Set the minimum width
    //
    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    //
    // Start the preview
    //
    m_currentItem = m_share;
    QTimer::singleShot(0, this, SLOT(slotInitializePreview()));
}

Smb4KPreviewDialog::~Smb4KPreviewDialog()
{
    //
    // Clear the share
    //
    m_share.clear();

    //
    // Clear the current item
    //
    m_currentItem.clear();

    //
    // Clear the listing
    //
    while (!m_listing.isEmpty()) {
        m_listing.takeFirst().clear();
    }
}

SharePtr Smb4KPreviewDialog::share() const
{
    return m_share;
}

void Smb4KPreviewDialog::slotClosingDialog()
{
    //
    // Save the dialog size
    //
    KConfigGroup group(Smb4KSettings::self()->config(), "PreviewDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);

    //
    // Emit the aboutToClose() signal
    //
    Q_EMIT aboutToClose(this);

    //
    // Close the dialog
    //
    accept();
}

void Smb4KPreviewDialog::slotReloadActionTriggered()
{
    KDualAction *reloadAction = findChild<KDualAction *>();

    if (reloadAction->isActive()) {
        Q_EMIT requestAbort();
    } else {
        Q_EMIT requestPreview(m_currentItem);
    }
}

void Smb4KPreviewDialog::slotUpActionTriggered()
{
    //
    // Get the new URL
    //
    QUrl u = KIO::upUrl(m_currentItem->url());

    //
    // Create a new network item object, if necessary and set the new current
    // item. Also, disable the "Up" action, if necessary.
    //
    if (m_share->url().matches(u, QUrl::StripTrailingSlash)) {
        findChild<QAction *>(QStringLiteral("up_action"))->setEnabled(false);
        m_currentItem = m_share;
    } else if (m_share->url().path().length() < u.path().length()) {
        FilePtr file = FilePtr(new Smb4KFile(u, Directory));
        file->setWorkgroupName(m_share->workgroupName());
        m_currentItem = file;
    } else {
        return;
    }

    //
    // Emit the requestPreview() signal
    //
    Q_EMIT requestPreview(m_currentItem);
}

void Smb4KPreviewDialog::slotUrlActivated(const QUrl &url)
{
    //
    // Get the full authentication information. This is needed, since the combo
    // box only returns sanitized URLs, i.e. without password, etc.
    //
    QUrl u = url;
    u.setUserName(m_share->userName());
    u.setPassword(m_share->password());

    //
    // Create a new network item object, if necessary and set the new current
    // item.
    //
    if (m_share->url().matches(u, QUrl::StripTrailingSlash)) {
        m_currentItem = m_share;
    } else {
        FilePtr file = FilePtr(new Smb4KFile(u, Directory));
        file->setWorkgroupName(m_share->workgroupName());
        m_currentItem = file;
    }

    //
    // Emit the requestPreview() signal
    //
    Q_EMIT requestPreview(m_currentItem);
}

void Smb4KPreviewDialog::slotItemActivated(QListWidgetItem *item)
{
    //
    // Only process the item if it represents a directory
    //
    if (item && item->type() == Directory) {
        //
        // Find the file item, make it the current one and emit the requestPreview()
        // signal.
        //
        for (const FilePtr &f : qAsConst(m_listing)) {
            if (item->data(Qt::UserRole).toUrl().matches(f->url(), QUrl::None)) {
                m_currentItem = f;
                Q_EMIT requestPreview(m_currentItem);
                break;
            }
        }
    }
}

void Smb4KPreviewDialog::slotInitializePreview()
{
    Q_EMIT requestPreview(m_currentItem);
}

void Smb4KPreviewDialog::slotPreviewResults(const QList<FilePtr> &list)
{
    //
    // Only process data the belongs to this dialog
    //
    if (m_share->workgroupName() == list.first()->workgroupName() && m_share->hostName() == list.first()->hostName()
        && list.first()->url().path().startsWith(m_share->url().path())) {
        //
        // Clear the internal listing
        //
        while (!m_listing.isEmpty()) {
            m_listing.takeFirst().clear();
        }

        //
        // Copy the list into the private variable
        //
        m_listing = list;

        //
        // Get the list widget
        //
        QListWidget *listWidget = findChild<QListWidget *>();

        //
        // Clear the list widget
        //
        listWidget->clear();

        //
        // Insert the new listing
        //
        if (listWidget) {
            for (const FilePtr &f : list) {
                QListWidgetItem *item = new QListWidgetItem(f->icon(), f->name(), listWidget, f->isDirectory() ? Directory : File);
                item->setData(Qt::UserRole, f->url());
            }
        }

        //
        // Sort the list widget
        //
        listWidget->sortItems();

        //
        // Add the URL to the combo box and show it. Omit duplicates.
        //
        KUrlComboBox *urlCombo = findChild<KUrlComboBox *>();
        QStringList urls = urlCombo->urls();
        urls << m_currentItem->url().toString();
        urlCombo->setUrls(urls);
        urlCombo->setUrl(m_currentItem->url());

        //
        // Enable / disable the "Up" action
        //
        findChild<QAction *>(QStringLiteral("up_action"))->setEnabled(!m_share->url().matches(m_currentItem->url(), QUrl::StripTrailingSlash));
    }
}

void Smb4KPreviewDialog::slotAboutToStart(const NetworkItemPtr &item, int type)
{
    if (type == LookupFiles) {
        switch (item->type()) {
        case Share: {
            SharePtr s = item.staticCast<Smb4KShare>();

            if (m_share->workgroupName() == s->workgroupName() && m_share->hostName() == s->hostName() && s->url().path().startsWith(m_share->url().path())) {
                KDualAction *reloadAction = findChild<KDualAction *>();
                reloadAction->setActive(true);
            }

            break;
        }
        case Directory: {
            FilePtr f = item.staticCast<Smb4KFile>();

            if (m_share->workgroupName() == f->workgroupName() && m_share->hostName() == f->hostName() && f->url().path().startsWith(m_share->url().path())) {
                KDualAction *reloadAction = findChild<KDualAction *>();
                reloadAction->setActive(true);
            }

            break;
        }
        default: {
            break;
        }
        }
    }
}

void Smb4KPreviewDialog::slotFinished(const NetworkItemPtr &item, int type)
{
    if (type == LookupFiles) {
        switch (item->type()) {
        case Share: {
            SharePtr s = item.staticCast<Smb4KShare>();

            if (m_share->workgroupName() == s->workgroupName() && m_share->hostName() == s->hostName() && s->url().path().startsWith(m_share->url().path())) {
                KDualAction *reloadAction = findChild<KDualAction *>();
                reloadAction->setActive(false);
            }

            break;
        }
        case Directory: {
            FilePtr f = item.staticCast<Smb4KFile>();

            if (m_share->workgroupName() == f->workgroupName() && m_share->hostName() == f->hostName() && f->url().path().startsWith(m_share->url().path())) {
                KDualAction *reloadAction = findChild<KDualAction *>();
                reloadAction->setActive(false);
            }

            break;
        }
        default: {
            break;
        }
        }
    }
}

Smb4KPrintDialog::Smb4KPrintDialog(const SharePtr &share, QWidget *parent)
    : QDialog(parent)
    , m_share(share)
{
    //
    // Dialog title
    //
    setWindowTitle(i18n("Print File"));

    //
    // Attributes
    //
    setAttribute(Qt::WA_DeleteOnClose, true);

    //
    // Layout
    //
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    //
    // Information group box
    //
    QGroupBox *informationBox = new QGroupBox(i18n("Information"), this);
    QGridLayout *informationBoxLayout = new QGridLayout(informationBox);

    // Printer name
    QLabel *printerNameLabel = new QLabel(i18n("Printer:"), informationBox);
    QLabel *printerName = new QLabel(share->displayString(), informationBox);

    informationBoxLayout->addWidget(printerNameLabel, 0, 0);
    informationBoxLayout->addWidget(printerName, 0, 1);

    // IP address
    QLabel *ipAddressLabel = new QLabel(i18n("IP Address:"), informationBox);
    QLabel *ipAddress = new QLabel(share->hostIpAddress(), informationBox);

    informationBoxLayout->addWidget(ipAddressLabel, 1, 0);
    informationBoxLayout->addWidget(ipAddress, 1, 1);

    // Workgroup
    QLabel *workgroupNameLabel = new QLabel(i18n("Domain:"), informationBox);
    QLabel *workgroupName = new QLabel(share->workgroupName(), informationBox);

    informationBoxLayout->addWidget(workgroupNameLabel, 2, 0);
    informationBoxLayout->addWidget(workgroupName, 2, 1);

    layout->addWidget(informationBox);

    //
    // File and settings group box
    //
    QGroupBox *fileBox = new QGroupBox(i18n("File and Settings"), this);
    QGridLayout *fileBoxLayout = new QGridLayout(fileBox);

    // File
    QLabel *fileLabel = new QLabel(i18n("File:"), fileBox);
    KUrlRequester *file = new KUrlRequester(fileBox);
    file->setMode(KFile::File | KFile::LocalOnly | KFile::ExistingOnly);
    file->setUrl(QUrl::fromLocalFile(QDir::homePath()));
    file->setWhatsThis(
        i18n("This is the file you want to print on the remote printer. "
             "Currently only a few mimetypes are supported such as PDF, Postscript, plain text, and "
             "images. If the file's mimetype is not supported, you need to convert it."));
    connect(file, SIGNAL(textChanged(QString)), this, SLOT(slotUrlChanged()));

    fileBoxLayout->addWidget(fileLabel, 0, 0);
    fileBoxLayout->addWidget(file, 0, 1);

    // Copies
    QLabel *copiesLabel = new QLabel(i18n("Copies:"), fileBox);
    QSpinBox *copies = new QSpinBox(fileBox);
    copies->setValue(1);
    copies->setMinimum(1);
    copies->setWhatsThis(i18n("This is the number of copies you want to print."));

    fileBoxLayout->addWidget(copiesLabel, 1, 0);
    fileBoxLayout->addWidget(copies, 1, 1);

    layout->addWidget(fileBox);

    //
    // Buttons
    //
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    QPushButton *printButton = buttonBox->addButton(i18n("Print"), QDialogButtonBox::ActionRole);
    printButton->setObjectName(QStringLiteral("print_button"));
    printButton->setShortcut(Qt::CTRL | Qt::Key_P);
    connect(printButton, SIGNAL(clicked(bool)), this, SLOT(slotPrintButtonClicked()));

    QPushButton *cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    cancelButton->setObjectName(QStringLiteral("cancel_button"));
    cancelButton->setShortcut(Qt::Key_Escape);
    cancelButton->setDefault(true);
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(slotCancelButtonClicked()));

    layout->addWidget(buttonBox);

    //
    // Set the minimum width
    //
    setMinimumWidth(sizeHint().width() > 350 ? sizeHint().width() : 350);

    //
    // Set the dialog size
    //
    create();

    KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
    QSize dialogSize;

    if (group.exists()) {
        KWindowConfig::restoreWindowSize(windowHandle(), group);
        dialogSize = windowHandle()->size();
    } else {
        dialogSize = sizeHint();
    }

    resize(dialogSize); // workaround for QTBUG-40584

    //
    // Set the buttons
    //
    slotUrlChanged();
}

Smb4KPrintDialog::~Smb4KPrintDialog()
{
}

SharePtr Smb4KPrintDialog::share() const
{
    return m_share;
}

KFileItem Smb4KPrintDialog::fileItem() const
{
    return m_fileItem;
}

void Smb4KPrintDialog::slotUrlChanged()
{
    //
    // Take the focus from the URL requester and give it to the dialog's
    // button box
    //
    QDialogButtonBox *buttonBox = findChild<QDialogButtonBox *>();
    buttonBox->setFocus();

    //
    // Get the URL from the URL requester
    //
    KUrlRequester *file = findChild<KUrlRequester *>();
    KFileItem fileItem = KFileItem(file->url());

    //
    // Apply the settings to the buttons of the dialog's button box
    //
    QPushButton *printButton = findChild<QPushButton *>(QStringLiteral("print_button"));
    printButton->setEnabled(fileItem.url().isValid() && fileItem.isFile());
    printButton->setDefault(fileItem.url().isValid() && fileItem.isFile());

    QPushButton *cancelButton = findChild<QPushButton *>(QStringLiteral("cancel_button"));
    cancelButton->setDefault(!fileItem.url().isValid() || !fileItem.isFile());
}

void Smb4KPrintDialog::slotPrintButtonClicked()
{
    //
    // Get the file item that is to be printed
    //
    KUrlRequester *file = findChild<KUrlRequester *>();
    m_fileItem = KFileItem(file->url());

    if (m_fileItem.url().isValid()) {
        //
        // Check if the mime type is supported or if the file can be
        // converted into a supported mimetype
        //
        if (m_fileItem.mimetype() != QStringLiteral("application/postscript") && m_fileItem.mimetype() != QStringLiteral("application/pdf")
            && m_fileItem.mimetype() != QStringLiteral("application/x-shellscript") && !m_fileItem.mimetype().startsWith(QStringLiteral("text"))
            && !m_fileItem.mimetype().startsWith(QStringLiteral("message")) && !m_fileItem.mimetype().startsWith(QStringLiteral("image"))) {
            Smb4KNotification::mimetypeNotSupported(m_fileItem.mimetype());
            return;
        }

        //
        // Save the window size
        //
        KConfigGroup group(Smb4KSettings::self()->config(), "PrintDialog");
        KWindowConfig::saveWindowSize(windowHandle(), group);

        //
        // Emit the printFile() signal
        //
        QSpinBox *copies = findChild<QSpinBox *>();
        Q_EMIT printFile(m_share, m_fileItem, copies->value());

        //
        // Emit the aboutToClose() signal
        //
        Q_EMIT aboutToClose(this);

        //
        // Close the print dialog
        //
        accept();
    }
}

void Smb4KPrintDialog::slotCancelButtonClicked()
{
    //
    // Abort the printing
    //
    Smb4KClient::self()->abort();

    //
    // Emit the aboutToClose() signal
    //
    Q_EMIT aboutToClose(this);

    //
    // Reject the dialog
    //
    reject();
}
