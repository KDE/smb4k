/*
    Private classes for the SMB client

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCLIENT_P_H
#define SMB4KCLIENT_P_H

// application specific includes
#include "smb4kclient.h"
#include "smb4kfile.h"
#include "smb4kglobal.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kworkgroup.h"

// Samba includes
#include <libsmbclient.h>

// Qt includes
#include <QHostAddress>
#include <QTimer>
#include <QUrl>

// KDE includes
#include <KDNSSD/RemoteService>
#include <KDNSSD/ServiceBrowser>
#include <KJob>

#ifdef USE_WS_DISCOVERY
#include <WSDiscoveryClient>
#endif

class Smb4KClientBaseJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KClientBaseJob(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KClientBaseJob();

    /**
     * Set the process
     */
    void setProcess(Smb4KGlobal::Process process);

    /**
     * Return the process
     */
    Smb4KGlobal::Process process() const;

    /**
     * Set the basic network item
     */
    void setNetworkItem(NetworkItemPtr networkItem);

    /**
     * Return the basic network item
     */
    NetworkItemPtr networkItem() const;

    /**
     * The list of workgroups that was discovered.
     */
    QList<WorkgroupPtr> workgroups();

    /**
     * The list of hosts that was discovered.
     */
    QList<HostPtr> hosts();

    /**
     * The list shares that was discovered.
     */
    QList<SharePtr> shares();

    /**
     * The ist of files and directories that were discovered.
     */
    QList<FilePtr> files();

    /**
     * Error enumeration
     *
     * @enum ClientError         The client failed
     * @enum AccessDeniedError   Permission denied
     * @enum FileAccessError     The file could not be read
     * @enum PrintFileError      The file could not be printed
     */
    enum { ClientError = UserDefinedError, AccessDeniedError, FileAccessError, PrintFileError };

protected:
    Smb4KGlobal::Process *pProcess;
    NetworkItemPtr *pNetworkItem;
    QList<WorkgroupPtr> *pWorkgroups;
    QList<HostPtr> *pHosts;
    QList<SharePtr> *pShares;
    QList<FilePtr> *pFiles;
    QHostAddress lookupIpAddress(const QString &name);

private:
    Smb4KGlobal::Process m_process;
    NetworkItemPtr m_networkItem;
    QList<WorkgroupPtr> m_workgroups;
    QList<HostPtr> m_hosts;
    QList<SharePtr> m_shares;
    QList<FilePtr> m_files;
};

class Smb4KClientJob : public Smb4KClientBaseJob
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KClientJob(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KClientJob();

    /**
     * Starts the job.
     */
    void start() override;

    /**
     * Set the file that is to be printed
     */
    void setPrintFileItem(const KFileItem &item);

    /**
     * Get the file that is to be printed
     */
    KFileItem printFileItem() const;

    /**
     * Set the number of copies that are to be printed
     */
    void setPrintCopies(int copies);

    /**
     * Get the number of copies that are to be printed
     */
    int printCopies() const;

    /**
     * The authentication function for libsmbclient
     */
    void get_auth_data_fn(const char *server,
                          const char *share,
                          char *workgroup,
                          int maxLenWorkgroup,
                          char *username,
                          int maxLenUsername,
                          char *password,
                          int maxLenPassword);

protected Q_SLOTS:
    void slotStartJob();
    void slotFinishJob();

private:
    void initClientLibrary();
    void doLookups();
    void doPrinting();
    SMBCCTX *m_context;
    KFileItem m_fileItem;
    int m_copies;
};

class Smb4KDnsDiscoveryJob : public Smb4KClientBaseJob
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KDnsDiscoveryJob(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KDnsDiscoveryJob();

    /**
     * Start the job
     */
    void start() override;

protected Q_SLOTS:
    void slotStartJob();
    void slotServiceAdded(KDNSSD::RemoteService::Ptr service);
    void slotFinished();

private:
    KDNSSD::ServiceBrowser *m_serviceBrowser;
};

#ifdef USE_WS_DISCOVERY
class Smb4KWsDiscoveryJob : public Smb4KClientBaseJob
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KWsDiscoveryJob(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KWsDiscoveryJob();

    /**
     * Start the job
     */
    void start() override;

protected Q_SLOTS:
    void slotStartJob();
    void slotProbeMatchReceived(const WSDiscoveryTargetService &service);
    void slotResolveMatchReceived(const WSDiscoveryTargetService &service);
    void slotDiscoveryFinished();

private:
    WSDiscoveryClient *m_discoveryClient;
    QTimer *m_timer;
};
#endif

class Smb4KClientPrivate
{
public:
    struct QueueContainer {
        Smb4KGlobal::Process process;
        NetworkItemPtr networkItem;
        KFileItem printFileItem;
        int printCopies;
    };
    QList<WorkgroupPtr> tempWorkgroupList;
    QList<HostPtr> tempHostList;
    QList<QueueContainer> queue;
};

class Smb4KClientStatic
{
public:
    Smb4KClient instance;
};

#endif
