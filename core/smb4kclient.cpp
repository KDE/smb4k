/*
    This class provides the interface to the libsmbclient library.

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kclient.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kclient_p.h"
#include "smb4kcredentialsmanager.h"
#include "smb4kcustomsettings.h"
#include "smb4kcustomsettingsmanager.h"
#include "smb4khardwareinterface.h"
#include "smb4khomesshareshandler.h"
#include "smb4knotification.h"
#include "smb4ksettings.h"

// Qt includes
#include <QApplication>
#include <QHostAddress>
#include <QPointer>
#include <QTimer>
#include <QUdpSocket>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KClientStatic, p);

Smb4KClient::Smb4KClient(QObject *parent)
    : KCompositeJob(parent)
    , d(new Smb4KClientPrivate)
{
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Smb4KClient::slotAboutToQuit);
    connect(Smb4KCredentialsManager::self(), &Smb4KCredentialsManager::credentialsUpdated, this, &Smb4KClient::slotCredentialsUpdated);
}

Smb4KClient::~Smb4KClient()
{
}

Smb4KClient *Smb4KClient::self()
{
    return &p->instance;
}

void Smb4KClient::start()
{
    //
    // Connect to the online state monitoring
    //
    connect(Smb4KHardwareInterface::self(), &Smb4KHardwareInterface::onlineStateChanged, this, &Smb4KClient::slotOnlineStateChanged, Qt::UniqueConnection);

    //
    // Start the scanning
    //
    if (Smb4KHardwareInterface::self()->isOnline()) {
        QTimer::singleShot(50, this, SLOT(slotStartJobs()));
    }
}

bool Smb4KClient::isRunning()
{
    return hasSubjobs();
}

void Smb4KClient::abort()
{
    QListIterator<KJob *> it(subjobs());

    while (it.hasNext()) {
        it.next()->kill(KJob::EmitResult);
    }
}

void Smb4KClient::lookupDomains()
{
    //
    // Send Wakeup-On-LAN packets
    //
    if (Smb4KSettings::enableWakeOnLAN()) {
        QList<CustomSettingsPtr> wakeOnLanEntries = Smb4KCustomSettingsManager::self()->wakeOnLanEntries();

        if (!wakeOnLanEntries.isEmpty()) {
            NetworkItemPtr item = NetworkItemPtr(new Smb4KBasicNetworkItem());
            Q_EMIT aboutToStart(item, WakeUp);

            QUdpSocket *socket = new QUdpSocket(this);

            for (int i = 0; i < wakeOnLanEntries.size(); ++i) {
                if (wakeOnLanEntries.at(i)->wakeOnLanSendBeforeNetworkScan()) {
                    QHostAddress addr;

                    if (wakeOnLanEntries.at(i)->hasIpAddress()) {
                        addr.setAddress(wakeOnLanEntries.at(i)->ipAddress());
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
                    QStringList parts = wakeOnLanEntries.at(i)->macAddress().split(QStringLiteral(":"), Qt::SkipEmptyParts);

                    for (int j = 0; j < 16; ++j) {
                        for (int k = 0; k < parts.size(); ++k) {
                            QString item = QStringLiteral("0x") + parts.at(k);
                            sequence.append(QChar(item.toInt(nullptr, 16)).toLatin1());
                        }
                    }

                    socket->writeDatagram(sequence, addr, 9);
                }
            }

            delete socket;

            // Wait the defined time
            int stop = 1000 * Smb4KSettings::wakeOnLANWaitingTime() / 250;
            int i = 0;

            while (i++ < stop) {
                wait(250);
            }

            Q_EMIT finished(item, WakeUp);
            item.clear();
        }
    }

    //
    // Emit the aboutToStart() signal
    //
    NetworkItemPtr networkItem = NetworkItemPtr(new Smb4KBasicNetworkItem(Network));
    networkItem->setUrl(QUrl(QStringLiteral("smb://")));
    Q_EMIT aboutToStart(networkItem, LookupDomains);

    //
    // Set the busy cursor
    //
    if (!hasSubjobs()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

    //
    // Create the client job
    //
    Smb4KClientJob *clientJob = new Smb4KClientJob(this);
    clientJob->setNetworkItem(networkItem);
    clientJob->setProcess(LookupDomains);

#ifdef USE_WS_DISCOVERY
    //
    // Create the WS Discovery job, if desired
    //
    Smb4KWsDiscoveryJob *wsDiscoveryJob = nullptr;

    if (Smb4KSettings::useWsDiscovery()) {
        wsDiscoveryJob = new Smb4KWsDiscoveryJob(this);
        wsDiscoveryJob->setNetworkItem(networkItem);
        wsDiscoveryJob->setProcess(LookupDomains);
    }
#endif

    //
    // Create the DNS-SD job, if desired
    //
    Smb4KDnsDiscoveryJob *dnsDiscoveryJob = nullptr;

    if (Smb4KSettings::useDnsServiceDiscovery()) {
        dnsDiscoveryJob = new Smb4KDnsDiscoveryJob(this);
        dnsDiscoveryJob->setNetworkItem(networkItem);
        dnsDiscoveryJob->setProcess(LookupDomains);
    }

    //
    // Add the jobs to the subjobs
    //
    addSubjob(clientJob);

#ifdef USE_WS_DISCOVERY
    if (wsDiscoveryJob) {
        addSubjob(wsDiscoveryJob);
    }
#endif

    if (dnsDiscoveryJob) {
        addSubjob(dnsDiscoveryJob);
    }

    //
    // Start the jobs
    //
    clientJob->start();

#ifdef USE_WS_DISCOVERY
    if (wsDiscoveryJob) {
        wsDiscoveryJob->start();
    }
#endif

    if (dnsDiscoveryJob) {
        dnsDiscoveryJob->start();
    }

    //
    // Clear the network item
    //
    networkItem.clear();
}

void Smb4KClient::lookupDomainMembers(const WorkgroupPtr &workgroup)
{
    //
    // Emit the aboutToStart() signal
    //
    Q_EMIT aboutToStart(workgroup, LookupDomainMembers);

    //
    // Set the busy cursor
    //
    if (!hasSubjobs()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

    //
    // Create the client job
    //
    Smb4KClientJob *clientJob = new Smb4KClientJob(this);
    clientJob->setNetworkItem(workgroup);
    clientJob->setProcess(LookupDomainMembers);

#ifdef USE_WS_DISCOVERY
    //
    // Create the WS Discovery job, if desired
    //
    Smb4KWsDiscoveryJob *wsDiscoveryJob = nullptr;

    if (Smb4KSettings::useWsDiscovery()) {
        wsDiscoveryJob = new Smb4KWsDiscoveryJob(this);
        wsDiscoveryJob->setNetworkItem(workgroup);
        wsDiscoveryJob->setProcess(LookupDomainMembers);
    }
#endif

    //
    // Create the DNS-SD job, if desired
    //
    Smb4KDnsDiscoveryJob *dnsDiscoveryJob = nullptr;

    if (Smb4KSettings::useDnsServiceDiscovery()) {
        dnsDiscoveryJob = new Smb4KDnsDiscoveryJob(this);
        dnsDiscoveryJob->setNetworkItem(workgroup);
        dnsDiscoveryJob->setProcess(LookupDomainMembers);
    }

    //
    // Add the jobs to the subjobs
    //
    addSubjob(clientJob);

#ifdef USE_WS_DISCOVERY
    if (wsDiscoveryJob) {
        addSubjob(wsDiscoveryJob);
    }
#endif

    if (dnsDiscoveryJob) {
        addSubjob(dnsDiscoveryJob);
    }

    //
    // Start the job
    //
    clientJob->start();

#ifdef USE_WS_DISCOVERY
    if (wsDiscoveryJob) {
        wsDiscoveryJob->start();
    }
#endif

    if (dnsDiscoveryJob) {
        dnsDiscoveryJob->start();
    }
}

void Smb4KClient::lookupShares(const HostPtr &host)
{
    //
    // Emit the aboutToStart() signal
    //
    Q_EMIT aboutToStart(host, LookupShares);

    //
    // Create the job
    //
    Smb4KClientJob *job = new Smb4KClientJob(this);
    job->setNetworkItem(host);
    job->setProcess(LookupShares);

    //
    // Set the busy cursor
    //
    if (!hasSubjobs()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

    //
    // Add the job to the subjobs
    //
    addSubjob(job);

    //
    // Start the job
    //
    job->start();
}

void Smb4KClient::lookupFiles(const NetworkItemPtr &item)
{
    if (item->type() == Share || (item->type() == FileOrDirectory && item.staticCast<Smb4KFile>()->isDirectory())) {
        Q_EMIT aboutToStart(item, LookupFiles);

        Smb4KClientJob *job = new Smb4KClientJob(this);
        job->setNetworkItem(item);
        job->setProcess(LookupFiles);

        if (!hasSubjobs()) {
            QApplication::setOverrideCursor(Qt::BusyCursor);
        }

        addSubjob(job);

        job->start();
    }
}

void Smb4KClient::printFile(const SharePtr &share, const KFileItem &fileItem, int copies)
{
    if (fileItem.mimetype() != QStringLiteral("application/postscript") && fileItem.mimetype() != QStringLiteral("application/pdf")
        && fileItem.mimetype() != QStringLiteral("application/x-shellscript") && !fileItem.mimetype().startsWith(QStringLiteral("text"))
        && !fileItem.mimetype().startsWith(QStringLiteral("message")) && !fileItem.mimetype().startsWith(QStringLiteral("image"))) {
        Smb4KNotification::mimetypeNotSupported(fileItem.mimetype());
        return;
    }

    //
    // Emit the aboutToStart() signal
    //
    Q_EMIT aboutToStart(share, PrintFile);

    //
    // Create the job
    //
    Smb4KClientJob *job = new Smb4KClientJob(this);
    job->setNetworkItem(share);
    job->setPrintFileItem(fileItem);
    job->setPrintCopies(copies);
    job->setProcess(PrintFile);

    //
    // Set the busy cursor
    //
    if (!hasSubjobs()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

    //
    // Add the job to the subjobs
    //
    addSubjob(job);

    //
    // Start the job
    //
    job->start();
}

void Smb4KClient::search(const QString &item)
{
    //
    // Create empty basic network item
    //
    NetworkItemPtr networkItem = NetworkItemPtr(new Smb4KBasicNetworkItem());

    //
    // Emit the aboutToStart() signal
    //
    Q_EMIT aboutToStart(networkItem, NetworkSearch);

    //
    // Before doing the search, lookup all domains, servers and shares in the
    // network neighborhood.
    //
    lookupDomains();

    while (isRunning()) {
        wait(50);
    }

    for (const WorkgroupPtr &workgroup : workgroupsList()) {
        lookupDomainMembers(workgroup);

        while (isRunning()) {
            wait(50);
        }
    }

    for (const HostPtr &host : hostsList()) {
        lookupShares(host);

        while (isRunning()) {
            wait(50);
        }
    }

    //
    // Do the actual search
    //
    QList<SharePtr> results;

    for (const SharePtr &share : sharesList()) {
        if (share->shareName().contains(item, Qt::CaseInsensitive)) {
            results << share;
        }
    }

    //
    // Emit the search results
    //
    Q_EMIT searchResults(results);

    //
    // Emit the finished() signal
    //
    Q_EMIT finished(networkItem, NetworkSearch);
}

void Smb4KClient::processErrors(Smb4KClientBaseJob *job)
{
    switch (job->error()) {
    case Smb4KClientJob::AccessDeniedError: {
        switch (job->networkItem()->type()) {
        case Host: {
            Smb4KClientPrivate::QueueContainer container;
            container.process = job->process();
            container.networkItem = job->networkItem();
            d->queue.append(container);
            Q_EMIT requestCredentials(job->networkItem());

            break;
        }
        case Share: {
            Smb4KClientPrivate::QueueContainer container;
            container.process = job->process();
            container.networkItem = job->networkItem();

            if (job->process() == Smb4KGlobal::PrintFile) {
                Smb4KClientJob *clientJob = qobject_cast<Smb4KClientJob *>(job);
                container.printFileItem = clientJob->printFileItem();
                container.printCopies = clientJob->printCopies();
            }

            d->queue.append(container);
            Q_EMIT requestCredentials(job->networkItem());

            break;
        }
        case FileOrDirectory: {
            Smb4KClientPrivate::QueueContainer container;
            container.process = job->process();
            container.networkItem = job->networkItem();
            d->queue.append(container);

            FilePtr file = job->networkItem().staticCast<Smb4KFile>();
            SharePtr share = SharePtr(new Smb4KShare());
            share->setWorkgroupName(file->workgroupName());
            share->setHostName(file->hostName());
            share->setShareName(file->shareName());
            share->setUserName(file->userName());
            share->setPassword(file->password());
            Q_EMIT requestCredentials(share);

            break;
        }
        default: {
            qDebug() << "Authentication error. URL:" << job->networkItem()->url();
            break;
        }
        }

        break;
    }
    default: {
        Smb4KNotification::networkCommunicationFailed(job->errorText());
        break;
    }
    }
}

void Smb4KClient::processWorkgroups(Smb4KClientBaseJob *job)
{
    //
    // Collect the workgroups found while scanning
    //
    QList<WorkgroupPtr> discoveredWorkgroups = job->workgroups();

    for (const WorkgroupPtr &newWorkgroup : qAsConst(discoveredWorkgroups)) {
        bool foundWorkgroup = false;

        for (const WorkgroupPtr &workgroup : qAsConst(d->tempWorkgroupList)) {
            if (workgroup->workgroupName() == newWorkgroup->workgroupName()) {
                foundWorkgroup = true;
                break;
            }
        }

        if (!foundWorkgroup) {
            d->tempWorkgroupList << newWorkgroup;
        }
    }

    //
    // When scanning finished, process the workgroups
    //
    if (!isRunning()) {
        // Remove obsolete workgroups and their members
        QListIterator<WorkgroupPtr> it(workgroupsList());

        while (it.hasNext()) {
            WorkgroupPtr workgroup = it.next();

            bool foundWorkgroup = false;

            for (const WorkgroupPtr &w : qAsConst(d->tempWorkgroupList)) {
                if (w->workgroupName() == workgroup->workgroupName()) {
                    foundWorkgroup = true;
                    break;
                }
            }

            if (!foundWorkgroup) {
                QList<HostPtr> obsoleteHosts = workgroupMembers(workgroup);

                while (!obsoleteHosts.isEmpty()) {
                    removeHost(obsoleteHosts.takeFirst());
                }

                removeWorkgroup(workgroup);
            }
        }

        // Add new workgroups and update existing ones
        for (const WorkgroupPtr &workgroup : qAsConst(d->tempWorkgroupList)) {
            if (!findWorkgroup(workgroup->workgroupName())) {
                addWorkgroup(workgroup);

                // Since this is a new workgroup, no master browser is present.
                HostPtr masterBrowser = HostPtr(new Smb4KHost());
                masterBrowser->setWorkgroupName(workgroup->workgroupName());
                masterBrowser->setHostName(workgroup->masterBrowserName());
                masterBrowser->setIpAddress(workgroup->masterBrowserIpAddress());
                masterBrowser->setIsMasterBrowser(true);

                addHost(masterBrowser);
            } else {
                updateWorkgroup(workgroup);

                // Check if the master browser changed
                QList<HostPtr> members = workgroupMembers(workgroup);

                for (const HostPtr &host : qAsConst(members)) {
                    if (workgroup->masterBrowserName() == host->hostName()) {
                        host->setIsMasterBrowser(true);

                        if (!host->hasIpAddress() && workgroup->hasMasterBrowserIpAddress()) {
                            host->setIpAddress(workgroup->masterBrowserIpAddress());
                        }
                    } else {
                        host->setIsMasterBrowser(false);
                    }
                }
            }
        }

        // Clear the temporary workgroup list
        while (!d->tempWorkgroupList.isEmpty()) {
            d->tempWorkgroupList.takeFirst().clear();
        }

        Q_EMIT workgroups();
    }
}

void Smb4KClient::processHosts(Smb4KClientBaseJob *job)
{
    //
    // Collect the hosts found while scanning. Always prefer
    // insertion of hosts with a real workgroup/domain over
    // the ones with the DNS-SD domain (e.g. LOCAL).
    //
    QList<HostPtr> discoveredHosts = job->hosts();

    for (const HostPtr &newHost : qAsConst(discoveredHosts)) {
        bool foundHost = false;

        QMutableListIterator<HostPtr> it(d->tempHostList);

        while (it.hasNext()) {
            HostPtr host = static_cast<HostPtr>(it.next());

            if (newHost->url().matches(host->url(), QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                if (newHost->workgroupName() == host->workgroupName()) {
                    foundHost = true;
                } else if (host->dnsDiscovered()) {
                    it.remove();
                }

                break;
            }
        }

        if (!foundHost) {
            d->tempHostList << newHost;
        }
    }

    //
    // When scanning finished, process the hosts
    //
    if (!isRunning()) {
        // Get the workgroup pointer. Although several scans might have been
        // running, the workgroup should have been always the same.
        WorkgroupPtr workgroup = job->networkItem().staticCast<Smb4KWorkgroup>();

        // Remove obsolete workgroup/domain members
        QList<HostPtr> members = workgroupMembers(workgroup);
        QListIterator<HostPtr> it(members);

        while (it.hasNext()) {
            HostPtr host = it.next();

            bool foundHost = false;

            for (const HostPtr &h : qAsConst(d->tempHostList)) {
                if (h->workgroupName() == host->workgroupName() && h->hostName() == host->hostName()) {
                    foundHost = true;
                    break;
                }
            }

            if (!foundHost) {
                QList<SharePtr> obsoleteShares = sharedResources(host);

                while (!obsoleteShares.isEmpty()) {
                    removeShare(obsoleteShares.takeFirst());
                }

                removeHost(host);
            }
        }

        // Add new hosts and update existing ones
        for (const HostPtr &host : qAsConst(d->tempHostList)) {
            if (host->hostName() == workgroup->masterBrowserName()) {
                host->setIsMasterBrowser(true);
            } else {
                host->setIsMasterBrowser(false);
            }

            if (!findHost(host->hostName(), host->workgroupName())) {
                addHost(host);
            } else {
                updateHost(host);
            }
        }

        // Clear the temporary host list
        while (!d->tempHostList.isEmpty()) {
            d->tempHostList.takeFirst().clear();
        }

        Q_EMIT hosts(workgroup);
    }
}

void Smb4KClient::processShares(Smb4KClientBaseJob *job)
{
    //
    // To look up the shared resources of a host, we only use
    // Samba's client library, so we do not have to wait until
    // several jobs finished.
    //
    // Get the host pointer
    //
    HostPtr host = job->networkItem().staticCast<Smb4KHost>();

    //
    // Copy the list of discovered shares
    //
    QList<SharePtr> discoveredShares = job->shares();

    //
    // Remove obsolete shares
    //
    QList<SharePtr> sharedRes = sharedResources(host);
    QListIterator<SharePtr> it(sharedRes);

    while (it.hasNext()) {
        SharePtr share = it.next();

        bool foundShare = false;

        for (const SharePtr &s : qAsConst(discoveredShares)) {
            if (s->workgroupName() == share->workgroupName() && s->url().matches(share->url(), QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                foundShare = true;
                break;
            }
        }

        if (!foundShare || (share->isHidden() && !Smb4KSettings::detectHiddenShares()) || (share->isPrinter() && !Smb4KSettings::detectPrinterShares())) {
            removeShare(share);
        }
    }

    //
    // Add new shares and update existing ones
    //
    for (const SharePtr &share : qAsConst(discoveredShares)) {
        // Process only those shares that the user wants to see
        if (share->isHidden() && !Smb4KSettings::detectHiddenShares()) {
            continue;
        }

        if (share->isPrinter() && !Smb4KSettings::detectPrinterShares()) {
            continue;
        }

        // Add or update the shares
        if (!findShare(share->url(), share->workgroupName())) {
            addShare(share);
        } else {
            updateShare(share);
        }
    }

    Q_EMIT shares(host);
}

void Smb4KClient::processFiles(Smb4KClientBaseJob *job)
{
    QList<FilePtr> discoveredFiles = job->files();
    QList<FilePtr> list;

    for (const FilePtr &file : qAsConst(discoveredFiles)) {
        if (file->isHidden() && !Smb4KSettings::previewHiddenItems()) {
            continue;
        }

        list << file;
    }

    Q_EMIT files(list);
}

void Smb4KClient::slotStartJobs()
{
    lookupDomains();
}

void Smb4KClient::slotOnlineStateChanged(bool online)
{
    if (online) {
        slotStartJobs();
    } else {
        abort();
    }
}

void Smb4KClient::slotResult(KJob *job)
{
    //
    // Remove the job
    //
    removeSubjob(job);

    //
    // Get the client base job
    //
    Smb4KClientBaseJob *clientBaseJob = qobject_cast<Smb4KClientBaseJob *>(job);

    //
    // Define a network item pointer and the process value for the
    // finished() signal.
    //
    NetworkItemPtr networkItem = clientBaseJob->networkItem();
    Smb4KGlobal::Process process = clientBaseJob->process();

    //
    // Get the result from the query and process it
    //
    if (clientBaseJob->error() == 0) {
        switch (networkItem->type()) {
        case Network: {
            // Process the discovered workgroups
            processWorkgroups(clientBaseJob);
            break;
        }
        case Workgroup: {
            // Process the discovered workgroup members
            processHosts(clientBaseJob);
            break;
        }
        case Host: {
            // Process the discovered shares
            processShares(clientBaseJob);
            break;
        }
        case Share: {
            processFiles(clientBaseJob);
            break;
        }
        case FileOrDirectory: {
            if (networkItem.staticCast<Smb4KFile>()->isDirectory()) {
                processFiles(clientBaseJob);
            }
            break;
        }
        default: {
            break;
        }
        }
    } else {
        processErrors(clientBaseJob);
    }

    //
    // Emit the finished signal when all subjobs finished
    //
    if (!hasSubjobs()) {
        Q_EMIT finished(networkItem, process);
    }

    //
    // Clear the network item pointer
    //
    networkItem.clear();

    //
    // Restore the cursor
    //
    if (!hasSubjobs()) {
        QApplication::restoreOverrideCursor();
    }
}

void Smb4KClient::slotAboutToQuit()
{
    abort();
}

void Smb4KClient::slotAbort()
{
    abort();
}

void Smb4KClient::slotCredentialsUpdated(const QUrl &url)
{
    if (!url.isEmpty() && !d->queue.isEmpty()) {
        QMutableListIterator<Smb4KClientPrivate::QueueContainer> it(d->queue);

        while (it.hasNext()) {
            Smb4KClientPrivate::QueueContainer container = it.next();

            QUrl networkItemUrl = container.networkItem->url();
            QUrl parentNetworkItemUrl = container.networkItem->url().resolved(QUrl(QStringLiteral(".."))).adjusted(QUrl::StripTrailingSlash);

            if (QString::compare(networkItemUrl.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                    == 0
                || QString::compare(parentNetworkItemUrl.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                    Qt::CaseInsensitive)
                    == 0) {
                switch (container.networkItem->type()) {
                case Host: {
                    HostPtr host = container.networkItem.staticCast<Smb4KHost>();
                    host->setUserName(url.userName());
                    host->setPassword(url.password());
                    lookupShares(host);
                    break;
                }
                case Share: {
                    SharePtr share = container.networkItem.staticCast<Smb4KShare>();
                    share->setUserName(url.userName());
                    share->setPassword(url.password());
                    if (container.process == Smb4KGlobal::PrintFile) {
                        printFile(share, container.printFileItem, container.printCopies);
                    } else {
                        lookupFiles(share);
                    }
                    break;
                }
                case FileOrDirectory: {
                    FilePtr file = container.networkItem.staticCast<Smb4KFile>();
                    file->setUserName(url.userName());
                    file->setPassword(url.password());
                    lookupFiles(file);
                    break;
                }
                default: {
                    break;
                }
                }

                it.remove();
            }
        }
    }
}
