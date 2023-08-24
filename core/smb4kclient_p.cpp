/*
    Private classes for the SMB client

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <KDualAction>
#include <KFileItem>
#include <KIO/Global>
#include <KIconLoader>
#include <KLocalizedString>
#include <KUrlComboBox>
#include <KUrlRequester>
#include <KWindowConfig>

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
            if (QString::fromUtf8(server, -1).toUpper() != QString::fromUtf8(workgroup, -1).toUpper()) {
                //
                // This is the master browser. Create a host object for it.
                //
                HostPtr masterBrowser = HostPtr(new Smb4KHost());
                masterBrowser->setWorkgroupName(QString::fromUtf8(workgroup, -1));
                masterBrowser->setHostName(QString::fromUtf8(server, -1));

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
    case FileOrDirectory: {
        //
        // The file object
        //
        FilePtr file = (*pNetworkItem).staticCast<Smb4KFile>();

        if (file->isDirectory()) {
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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));

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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
    case FileOrDirectory: {
        //
        // In case the domain/workgroup was discovered by the DNS-SD service, the
        // workgroup/domain might not be identical with the one defined in the network
        // neighborhood. Thus, only set the workgroup if no DNS-SD discovery was used.
        //
        FilePtr file = (*pNetworkItem).staticCast<Smb4KFile>();

        if (file->isDirectory()) {
            WorkgroupPtr workgroup = findWorkgroup(file->workgroupName());

            if (workgroup && !workgroup->dnsDiscovered()) {
                smbc_setWorkgroup(m_context, file->workgroupName().toUtf8().data());
            }

            //
            // Set the NetBIOS name
            //
            smbc_setNetbiosName(m_context, file->hostName().toUtf8().data());
        }

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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
                setErrorText(QString::fromUtf8(strerror(errorCode), -1));
                break;
            }
            case ENOENT: {
                if ((*pNetworkItem)->type() != Network) {
                    setError(ClientError);
                    setErrorText(QString::fromUtf8(strerror(errorCode), -1));
                }
                break;
            }
            default: {
                setError(ClientError);
                setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
            QString workgroupName = QString::fromUtf8(directoryEntry->name, -1);
            workgroup->setWorkgroupName(workgroupName);

            //
            // Set the master browser
            //
            QString masterBrowserName = QString::fromUtf8(directoryEntry->comment, -1);
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
            QString hostName = QString::fromUtf8(directoryEntry->name, -1);
            host->setHostName(hostName);

            //
            // Set the comment
            //
            QString comment = QString::fromUtf8(directoryEntry->comment, -1);
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
            share->setShareName(QString::fromUtf8(directoryEntry->name, -1));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment, -1));

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
            share->setShareName(QString::fromUtf8(directoryEntry->name, -1));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment, -1));

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
            share->setShareName(QString::fromUtf8(directoryEntry->name, -1));

            //
            // Set the comment
            //
            share->setComment(QString::fromUtf8(directoryEntry->comment, -1));

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
            QString name = QString::fromUtf8(directoryEntry->name, -1);

            if (name != QStringLiteral(".") && name != QStringLiteral("..")) {
                //
                // Create the URL for the discovered item
                //
                QUrl u = (*pNetworkItem)->url();
                u.setPath((*pNetworkItem)->url().path() + QDir::separator() + QString::fromUtf8(directoryEntry->name, -1));

                //
                // We do not stat directories. Directly create the directory object
                //
                FilePtr dir = FilePtr(new Smb4KFile(u));
                dir->setDirectory(true);

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
            u.setPath((*pNetworkItem)->url().path() + QDir::separator() + QString::fromUtf8(directoryEntry->name, -1));

            //
            // Create the file object
            //
            FilePtr file = FilePtr(new Smb4KFile(u));

            //
            // Set the workgroup name
            //
            file->setWorkgroupName((*pNetworkItem).staticCast<Smb4KShare>()->workgroupName());

            //
            // Stat the file
            //
            // FIXME

            //
            // Set the authentication data
            //
            file->setUserName((*pNetworkItem)->url().userName());
            file->setPassword((*pNetworkItem)->url().password());

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
                file->setHostIpAddress(address);
                *pFiles << file;
            } else {
                file.clear();
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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
        setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
            setErrorText(QString::fromUtf8(strerror(errorCode), -1));
            break;
        }
        default: {
            setError(ClientError);
            setErrorText(QString::fromUtf8(strerror(errorCode), -1));
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
    switch (m_serviceBrowser->isAvailable()) {
    case KDNSSD::ServiceBrowser::Working: {
        break;
    }
    case KDNSSD::ServiceBrowser::Stopped: {
        Smb4KNotification::zeroconfError(i18n("The Zeroconf daemon is not running. No servers are discovered using DNS-SD."));
        break;
    }
    case KDNSSD::ServiceBrowser::Unsupported: {
        Smb4KNotification::zeroconfError(i18n("Zeroconf support is not available in this version of KDE."));
        break;
    }
    default: {
        Smb4KNotification::zeroconfError(i18n("An unknown error with Zeroconf occurred."));
        break;
    }
    }

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
