/*
    This is the global namespace for Smb4K.

    SPDX-FileCopyrightText: 2005-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kglobal.h"
#include "smb4kclient.h"
#include "smb4kglobal_p.h"
#include "smb4kglobalenums.h"
#include "smb4kmounter.h"
#include "smb4knotification.h"
#include "smb4ksynchronizer.h"

// Qt includes
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0))
#include <QApplicationStatic>
#else
#include <qapplicationstatic.h>
#endif
#include <QDebug>
#include <QDirIterator>
#include <QEventLoop>
#include <QRecursiveMutex>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

// KDE includes
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KProcess>

Q_APPLICATION_STATIC(Smb4KGlobalPrivate, p);
QRecursiveMutex mutex;

const QList<WorkgroupPtr> &Smb4KGlobal::workgroupsList()
{
    return p->workgroupsList;
}

WorkgroupPtr Smb4KGlobal::findWorkgroup(const QString &name)
{
    WorkgroupPtr workgroup;

    mutex.lock();

    for (const WorkgroupPtr &w : std::as_const(p->workgroupsList)) {
        if (QString::compare(w->workgroupName(), name, Qt::CaseInsensitive) == 0) {
            workgroup = w;
            break;
        }
    }

    mutex.unlock();

    return workgroup;
}

bool Smb4KGlobal::addWorkgroup(WorkgroupPtr workgroup)
{
    Q_ASSERT(workgroup);

    bool added = false;

    if (workgroup) {
        mutex.lock();

        if (!findWorkgroup(workgroup->workgroupName())) {
            p->workgroupsList.append(workgroup);
            added = true;
        }

        mutex.unlock();
    }

    return added;
}

bool Smb4KGlobal::updateWorkgroup(WorkgroupPtr workgroup)
{
    Q_ASSERT(workgroup);

    bool updated = false;

    if (workgroup) {
        mutex.lock();

        WorkgroupPtr existingWorkgroup = findWorkgroup(workgroup->workgroupName());

        if (existingWorkgroup) {
            existingWorkgroup->update(workgroup.data());
            updated = true;
        }

        mutex.unlock();
    }

    return updated;
}

bool Smb4KGlobal::removeWorkgroup(WorkgroupPtr workgroup)
{
    Q_ASSERT(workgroup);

    bool removed = false;

    if (workgroup) {
        mutex.lock();

        int index = p->workgroupsList.indexOf(workgroup);

        if (index != -1) {
            // The workgroup was found. Remove it.
            p->workgroupsList.takeAt(index).clear();
            removed = true;
        } else {
            // Try harder to find the workgroup.
            WorkgroupPtr wg = findWorkgroup(workgroup->workgroupName());

            if (wg) {
                index = p->workgroupsList.indexOf(wg);

                if (index != -1) {
                    p->workgroupsList.takeAt(index).clear();
                    removed = true;
                }
            }

            workgroup.clear();
        }

        mutex.unlock();
    }

    return removed;
}

void Smb4KGlobal::clearWorkgroupsList()
{
    mutex.lock();

    while (!p->workgroupsList.isEmpty()) {
        p->workgroupsList.takeFirst().clear();
    }

    mutex.unlock();
}

const QList<HostPtr> &Smb4KGlobal::hostsList()
{
    return p->hostsList;
}

HostPtr Smb4KGlobal::findHost(const QString &name, const QString &workgroup)
{
    HostPtr host;

    mutex.lock();

    for (const HostPtr &h : std::as_const(p->hostsList)) {
        if ((workgroup.isEmpty() || QString::compare(h->workgroupName(), workgroup, Qt::CaseInsensitive) == 0)
            && QString::compare(h->hostName(), name, Qt::CaseInsensitive) == 0) {
            host = h;
            break;
        }
    }

    mutex.unlock();

    return host;
}

bool Smb4KGlobal::addHost(HostPtr host)
{
    Q_ASSERT(host);

    bool added = false;

    if (host) {
        mutex.lock();

        if (!findHost(host->hostName(), host->workgroupName())) {
            p->hostsList.append(host);
            added = true;
        }

        mutex.unlock();
    }

    return added;
}

bool Smb4KGlobal::updateHost(HostPtr host)
{
    Q_ASSERT(host);

    bool updated = false;

    if (host) {
        mutex.lock();

        HostPtr existingHost = findHost(host->hostName(), host->workgroupName());

        if (existingHost) {
            existingHost->update(host.data());
            updated = true;
        }

        mutex.unlock();
    }

    return updated;
}

bool Smb4KGlobal::removeHost(HostPtr host)
{
    Q_ASSERT(host);

    bool removed = false;

    if (host) {
        mutex.lock();

        int index = p->hostsList.indexOf(host);

        if (index != -1) {
            // The host was found. Remove it.
            p->hostsList.takeAt(index).clear();
            removed = true;
        } else {
            // Try harder to find the host.
            HostPtr h = findHost(host->hostName(), host->workgroupName());

            if (h) {
                index = p->hostsList.indexOf(h);

                if (index != -1) {
                    p->hostsList.takeAt(index).clear();
                    removed = true;
                }
            }

            host.clear();
        }

        mutex.unlock();
    }

    return removed;
}

void Smb4KGlobal::clearHostsList()
{
    mutex.lock();

    while (!p->hostsList.isEmpty()) {
        p->hostsList.takeFirst().clear();
    }

    mutex.unlock();
}

QList<HostPtr> Smb4KGlobal::workgroupMembers(WorkgroupPtr workgroup)
{
    QList<HostPtr> hosts;

    mutex.lock();

    for (const HostPtr &h : std::as_const(p->hostsList)) {
        if (QString::compare(h->workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive) == 0) {
            hosts << h;
        }
    }

    mutex.unlock();

    return hosts;
}

const QList<SharePtr> &Smb4KGlobal::sharesList()
{
    return p->sharesList;
}

SharePtr Smb4KGlobal::findShare(const QUrl &url, const QString &workgroup)
{
    SharePtr share;

    mutex.lock();

    for (const SharePtr &s : std::as_const(p->sharesList)) {
        if (QString::compare(s->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                             url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                             Qt::CaseInsensitive)
                == 0
            && (workgroup.isEmpty() || QString::compare(s->workgroupName(), workgroup, Qt::CaseInsensitive) == 0)) {
            share = s;
            break;
        }
    }

    mutex.unlock();

    return share;
}

bool Smb4KGlobal::addShare(SharePtr share)
{
    Q_ASSERT(share);

    bool added = false;

    if (share) {
        mutex.lock();

        //
        // Add the share
        //
        if (!findShare(share->url(), share->workgroupName())) {
            //
            // Set the share mounted
            // Only honor shares that are owned by the user
            //
            QList<SharePtr> mountedShares = findShareByUrl(share->url());

            if (!mountedShares.isEmpty()) {
                for (const SharePtr &s : std::as_const(mountedShares)) {
                    if (!s->isForeign()) {
                        share->setMountData(s.data());
                        break;
                    } else {
                        continue;
                    }
                }
            }

            //
            // Add it
            //
            p->sharesList.append(share);
            added = true;
        }
    }

    mutex.unlock();

    return added;
}

bool Smb4KGlobal::updateShare(SharePtr share)
{
    Q_ASSERT(share);

    bool updated = false;

    if (share) {
        mutex.lock();

        //
        // Updated the share
        //
        SharePtr existingShare = findShare(share->url(), share->workgroupName());

        if (existingShare) {
            //
            // Set the share mounted
            // Only honor shares that are owned by the user
            //
            QList<SharePtr> mountedShares = findShareByUrl(share->url());

            if (!mountedShares.isEmpty()) {
                for (const SharePtr &s : std::as_const(mountedShares)) {
                    if (!s->isForeign()) {
                        share->setMountData(s.data());
                        break;
                    } else {
                        continue;
                    }
                }
            }

            //
            // Update it
            //
            existingShare->update(share.data());
            updated = true;
        }

        mutex.unlock();
    }

    return updated;
}

bool Smb4KGlobal::removeShare(SharePtr share)
{
    Q_ASSERT(share);

    bool removed = false;

    if (share) {
        mutex.lock();

        int index = p->sharesList.indexOf(share);

        if (index != -1) {
            // The share was found. Remove it.
            p->sharesList.takeAt(index).clear();
            removed = true;
        } else {
            // Try harder to find the share.
            SharePtr s = findShare(share->url(), share->workgroupName());

            if (s) {
                index = p->sharesList.indexOf(s);

                if (index != -1) {
                    p->sharesList.takeAt(index).clear();
                    removed = true;
                }
            }

            share.clear();
        }

        mutex.unlock();
    }

    return removed;
}

void Smb4KGlobal::clearSharesList()
{
    mutex.lock();

    while (!p->sharesList.isEmpty()) {
        p->sharesList.takeFirst().clear();
    }

    mutex.unlock();
}

QList<SharePtr> Smb4KGlobal::sharedResources(HostPtr host)
{
    QList<SharePtr> shares;

    mutex.lock();

    for (const SharePtr &s : std::as_const(p->sharesList)) {
        if (QString::compare(s->hostName(), host->hostName(), Qt::CaseInsensitive) == 0
            && QString::compare(s->workgroupName(), host->workgroupName(), Qt::CaseInsensitive) == 0) {
            shares += s;
        }
    }

    mutex.unlock();

    return shares;
}

const QList<SharePtr> &Smb4KGlobal::mountedSharesList()
{
    return p->mountedSharesList;
}

SharePtr Smb4KGlobal::findShareByPath(const QString &path)
{
    SharePtr share;

    mutex.lock();

    if (!path.isEmpty() && !p->mountedSharesList.isEmpty()) {
        for (const SharePtr &s : std::as_const(p->mountedSharesList)) {
            if (QString::compare(s->path(), path, Qt::CaseInsensitive) == 0
                || (!s->isInaccessible() && QString::compare(s->canonicalPath(), path, Qt::CaseInsensitive) == 0)) {
                share = s;
                break;
            }
        }
    }

    mutex.unlock();

    return share;
}

QList<SharePtr> Smb4KGlobal::findShareByUrl(const QUrl &url)
{
    QList<SharePtr> shares;

    mutex.lock();

    if (!url.isEmpty() && url.isValid() && !p->mountedSharesList.isEmpty()) {
        for (const SharePtr &s : std::as_const(p->mountedSharesList)) {
            if (QString::compare(s->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 url.toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                == 0) {
                shares << s;
                break;
            }
        }
    }

    mutex.unlock();

    return shares;
}

QList<SharePtr> Smb4KGlobal::findInaccessibleShares()
{
    QList<SharePtr> inaccessibleShares;

    mutex.lock();

    for (const SharePtr &s : std::as_const(p->mountedSharesList)) {
        if (s->isInaccessible()) {
            inaccessibleShares += s;
        }
    }

    mutex.unlock();

    return inaccessibleShares;
}

bool Smb4KGlobal::addMountedShare(SharePtr share)
{
    Q_ASSERT(share);

    bool added = false;

    if (share) {
        mutex.lock();

        //
        // Copy the mount data to the network share if available.
        // Only honor shares that were mounted by the user.
        //
        if (!share->isForeign()) {
            // Network share
            SharePtr networkShare = findShare(share->url(), share->workgroupName());

            if (networkShare) {
                networkShare->setMountData(share.data());
            }
        }

        if (!findShareByPath(share->path())) {
            //
            // Check if we have to add a workgroup name and/or IP address
            //
            HostPtr networkHost = findHost(share->hostName(), share->workgroupName());

            if (networkHost) {
                // Set the IP address
                if (!share->hasHostIpAddress() || networkHost->ipAddress() != share->hostIpAddress()) {
                    share->setHostIpAddress(networkHost->ipAddress());
                }

                // Set the workgroup name
                if (share->workgroupName().isEmpty()) {
                    share->setWorkgroupName(networkHost->workgroupName());
                }
            }

            p->mountedSharesList.append(share);
            added = true;

            p->onlyForeignShares = true;

            for (const SharePtr &s : std::as_const(p->mountedSharesList)) {
                if (!s->isForeign()) {
                    p->onlyForeignShares = false;
                    break;
                }
            }
        }

        mutex.unlock();
    }

    return added;
}

bool Smb4KGlobal::updateMountedShare(SharePtr share)
{
    Q_ASSERT(share);

    bool updated = false;

    if (share) {
        mutex.lock();

        //
        // Copy the mount data to the network share (needed for unmounting from the network browser)
        // Only honor shares that were mounted by the user.
        //
        if (!share->isForeign()) {
            SharePtr networkShare = findShare(share->url(), share->workgroupName());

            if (networkShare) {
                networkShare->setMountData(share.data());
            }
        }

        SharePtr mountedShare = findShareByPath(share->path());

        if (mountedShare) {
            //
            // Check if we have to add a workgroup name and/or IP address
            //
            HostPtr networkHost = findHost(share->hostName(), share->workgroupName());

            if (networkHost) {
                // Set the IP address
                if (!share->hasHostIpAddress() || networkHost->ipAddress() != share->hostIpAddress()) {
                    share->setHostIpAddress(networkHost->ipAddress());
                }

                // Set the workgroup name
                if (share->workgroupName().isEmpty()) {
                    share->setWorkgroupName(networkHost->workgroupName());
                }
            }

            //
            // Update share
            //
            mountedShare->setMountData(share.data());
            updated = true;
        }

        mutex.unlock();
    }

    return updated;
}

bool Smb4KGlobal::removeMountedShare(SharePtr share, bool takeOnly)
{
    Q_ASSERT(share);

    bool removed = false;

    if (share) {
        mutex.lock();

        // Reset the mount data for the _network share_
        if (!share->isForeign()) {
            SharePtr networkShare = findShare(share->url(), share->workgroupName());

            if (networkShare) {
                networkShare->resetMountData();
            }
        }

        int index = p->mountedSharesList.indexOf(share);

        if (index != -1) {
            p->mountedSharesList.takeAt(index);
            removed = true;
        } else {
            SharePtr s = findShareByPath(share->isInaccessible() ? share->path() : share->canonicalPath());

            if (s) {
                index = p->mountedSharesList.indexOf(s);

                if (index != -1) {
                    p->mountedSharesList.takeAt(index).clear();
                    removed = true;
                }
            }
        }

        if (share && !takeOnly) {
            share.clear();
        }

        for (const SharePtr &s : std::as_const(p->mountedSharesList)) {
            if (!s->isForeign()) {
                p->onlyForeignShares = false;
                break;
            }
        }

        mutex.unlock();
    }

    return removed;
}

bool Smb4KGlobal::onlyForeignMountedShares()
{
    return p->onlyForeignShares;
}

void Smb4KGlobal::openShare(SharePtr share, OpenWith openWith)
{
    if (!share || share->isInaccessible()) {
        return;
    }

    switch (openWith) {
    case FileManager: {
        QUrl url = QUrl::fromLocalFile(share->canonicalPath());

        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url);
        job->setFollowRedirections(false);
        job->setAutoDelete(true);
        job->start();

        break;
    }
    case Konsole: {
        QString konsole = QStandardPaths::findExecutable(QStringLiteral("konsole"));

        if (!konsole.isEmpty()) {
            KIO::CommandLauncherJob *job = new KIO::CommandLauncherJob(konsole);
            job->setWorkingDirectory(share->canonicalPath());
            job->setAutoDelete(true);
            job->start();
        } else {
            Smb4KNotification::commandNotFound(QStringLiteral("konsole"));
        }

        break;
    }
    default: {
        break;
    }
    }
}

const QString Smb4KGlobal::machineNetbiosName()
{
    return p->machineNetbiosName;
}

const QString Smb4KGlobal::machineWorkgroupName()
{
    return p->machineWorkgroupName;
}

#if defined(Q_OS_LINUX)
QStringList Smb4KGlobal::allowedMountArguments()
{
    return p->allowedMountArguments;
}
#endif

const QString Smb4KGlobal::findMountExecutable()
{
    QStringList paths;
    paths << QStringLiteral("/bin");
    paths << QStringLiteral("/sbin");
    paths << QStringLiteral("/usr/bin");
    paths << QStringLiteral("/usr/sbin");
    paths << QStringLiteral("/usr/local/bin");
    paths << QStringLiteral("/usr/local/sbin");

#if defined(Q_OS_LINUX)
    return QStandardPaths::findExecutable(QStringLiteral("mount.cifs"), paths);
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    return QStandardPaths::findExecutable(QStringLiteral("mount_smbfs"), paths);
#else
    return QString();
#endif
}

const QString Smb4KGlobal::findUmountExecutable()
{
    QStringList paths;
    paths << QStringLiteral("/bin");
    paths << QStringLiteral("/sbin");
    paths << QStringLiteral("/usr/bin");
    paths << QStringLiteral("/usr/sbin");
    paths << QStringLiteral("/usr/local/bin");
    paths << QStringLiteral("/usr/local/sbin");

    return QStandardPaths::findExecutable(QStringLiteral("umount"), paths);
}

const QString Smb4KGlobal::dataLocation()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() + QStringLiteral("smb4k");
}

void Smb4KGlobal::wait(int time)
{
    QEventLoop loop;
    QTimer::singleShot(time, &loop, SLOT(quit()));
    loop.exec();
}

const QString Smb4KGlobal::findMacAddress(const QString &ipAddress)
{
    QString macAddress, executable;

#if defined(Q_OS_LINUX)
    executable = QStandardPaths::findExecutable(QStringLiteral("ip"));
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QHostAddress address(ipAddress);

    if (address.protocol() == QHostAddress::IPv4Protocol) {
        executable = QStandardPaths::findExecutable(QStringLiteral("arp"));
    } else if (address.protocol() == QHostAddress::IPv6Protocol) {
        executable = QStandardPaths::findExecutable(QStringLiteral("ndp"));
    }
#endif

    if (!executable.isEmpty()) {
        QStringList command;
        command << executable;
#if defined(Q_OS_LINUX)
        command << QStringLiteral("neighbor");
        command << QStringLiteral("show");
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
        command << QStringLiteral("-an");
#endif

        KProcess process;
        process.setProgram(command);
        process.setOutputChannelMode(KProcess::SeparateChannels);

        if (process.execute(-1) >= 0) {
            QStringList result = QString::fromLocal8Bit(process.readAllStandardOutput()).split(QStringLiteral("\n"));

            for (const QString &r : result) {
#if defined(Q_OS_LINUX)
                if (r.section(QStringLiteral(" "), 0, 0) == ipAddress) {
                    macAddress = r.section(QStringLiteral(" "), 4, 4);
                    break;
                }
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
                if (address.protocol() == QHostAddress::IPv4Protocol
                    && r.section(QStringLiteral(" "), 1, 1).remove(QStringLiteral("(")).remove(QStringLiteral(")")) == ipAddress) {
                    macAddress = r.simplified().section(QStringLiteral(" "), 3, 3);
                    break;
                } else if (address.protocol() == QHostAddress::IPv6Protocol
                           && r.section(QStringLiteral(" "), 0, 0).section(QStringLiteral("%"), 0, 0) == ipAddress) {
                    macAddress = r.simplified().section(QStringLiteral(" "), 1, 1);
                    break;
                }
#endif
            }
        }
    }

    return macAddress;
}
