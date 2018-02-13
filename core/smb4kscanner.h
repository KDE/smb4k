/***************************************************************************
    This class retrieves all workgroups, servers and shares found on the 
    network neighborhood
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2018 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSCANNER_H
#define SMB4KSCANNER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QTimerEvent>
#include <QScopedPointer>
#include <QWidget>

// KDE includes
#include <KCoreAddons/KCompositeJob>

// forward declarations
class Smb4KBasicNetworkItem;
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KQueryMasterJob;
class Smb4KLookupDomainMembersJob;
class Smb4KLookupSharesJob;
class Smb4KScannerPrivate;


/**
 * This class scans the network neighborhood for workgroups/domains,
 * hosts and shares. It looks up additions information and IP addresses.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class Q_DECL_EXPORT Smb4KScanner : public KCompositeJob
{
  Q_OBJECT

  friend class Smb4KScannerPrivate;

  public:
    /**
     * Constructor
     */
    explicit Smb4KScanner(QObject *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KScanner();
    
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KScanner class.
     */
    static Smb4KScanner *self();

    /**
     * This function tells you whether scan jobs are running
     * or not.
     *
     * @returns TRUE if at least one scan job is running
     */
    bool isRunning();

    /**
     * This function aborts all scan jobs at once.
     */
    void abortAll();

    /**
     * This function starts the composite job
     */
    void start();

    /**
     * This function looks up all domains and workgroups in the 
     * network neighborhood. Hown this is done depends on the 
     * settings the user chose.
     *
     * @param parent          The parent widget
     */
    void lookupDomains(QWidget *parent = 0);

    /**
     * This function looks up all hosts in a certain domain or
     * workgroup.
     * 
     * @param workgroup       The workgroup object
     *
     * @param parent          The parent widget
     */
    void lookupDomainMembers(WorkgroupPtr workgroup, QWidget *parent = 0);

    /**
     * This function looks up all shared resources a certain 
     * @p host provides.
     * 
     * @param host            The host object
     * 
     * @param parent          The parent widget
     */
    void lookupShares(HostPtr host, QWidget *parent = 0);
    
  protected:
    /**
     * Reimplemented from QObject
     */
    void timerEvent(QTimerEvent *e);

  Q_SIGNALS:
    /**
     * This signal is emitted when a scan process is about to be started. It passes
     * the network @p item as well as the lookup method that is used to the receiver.
     *
     * Please note that the network @p item may be empty, i.e. type() returns Unknown.
     * The @p process, however, will always be meaningful.
     *
     * @param item          The Smb4KBasicNetworkItem object
     *
     * @param process       The process that is used
     */
    void aboutToStart(const NetworkItemPtr &item, int process);

    /**
     * This signal is emitted when a scan process has finished. It passes the
     * network @p item as well as the lookup method that was used to the receiver.
     *
     * Please note that the network @p item may be empty, i.e. type() returns Unknown.
     * The @p process, however, will always be meaningful.
     *
     * @param item          The Smb4KBasicNetworkItem object
     *
     * @param process       The process that was used
     */
    void finished(const NetworkItemPtr &item, int process);

    /**
     * This signal is emitted when the list of workgroups was updated/changed.
     */
    void workgroups();

    /**
     * This signal is emitted when the list of members of the workgroup @p workgroup 
     * was discovered.
     *
     * @param workgroup   The workgroup that was scanned
     */
    void hosts(const WorkgroupPtr &workgroup);
    
    /**
     * This signal is emitted when the list of shares a certain host @p host offers 
     * has been acquired.
     * 
     * @param host        The host that was queried
     */
    void shares(const HostPtr &host);
    
    /**
     * This signal is emitted when an authentication error occurred.
     * 
     * @param host        The host that is affected
     */
    void authError(const HostPtr &host, int process);

    /**
     * This signal is emitted when an IP address was successfully looked
     * up.
     *
     * @param host          The host
     */
    void ipAddress(const HostPtr &host);

  protected Q_SLOTS:
    /**
     * Starts the composite job
     */
    void slotStartJobs();

    /**
     * Do last things before the application goes down
     */
    void slotAboutToQuit();

    /**
     * Called when a job finished
     */
    void slotJobFinished(KJob *job);

    /**
     * Called when an authentication error occurred while a master 
     * browser was queried for the list of workgroups/domains
     */
    void slotAuthError(Smb4KQueryMasterJob *job);
    
    /**
     * Called when an authentication error occurred while a master
     * browser was queried for the list of workgroup/domain members
     */
    void slotAuthError(Smb4KLookupDomainMembersJob *job);
    
    /**
     * Called when an authentication error occurred while a host
     * was queried for the list of shares
     */
    void slotAuthError(Smb4KLookupSharesJob *job);

    /**
     * A lookup process for workgroups or domains is about to 
     * be started
     */
    void slotAboutToStartDomainsLookup();

    /**
     * A lookup process for workgroups or domains has finished
     */
    void slotDomainsLookupFinished();

    /**
     * A lookup process for hosts (domain members) is about to
     * be started
     */
    void slotAboutToStartHostsLookup(const WorkgroupPtr &workgroup);

    /**
     * A lookup process for hosts has finished
     */
    void slotHostsLookupFinished(const WorkgroupPtr &workgroup);
    
    /**
     * A lookup process for shares is about to be started
     */
    void slotAboutToStartSharesLookup(const HostPtr &host);

    /**
     * A lookup process for shares has finished
     */
    void slotSharesLookupFinished(const HostPtr &host);
    
    /**
     * Is called when workgroups and domains have been looked
     * up
     */
    void slotWorkgroups(const QList<WorkgroupPtr> &workgroups_list);

    /**
     * Is called when hosts have been looked up by the normal lookup
     * method
     */
    void slotHosts(const WorkgroupPtr &workgroup, const QList<HostPtr> &hosts_list);
    
    /**
     * Is called when shares have been looked up
     */
    void slotShares(const HostPtr &host, const QList<SharePtr> &shares_list);
    
    /**
     * Is called when an IP address was looked up
     */
    void slotProcessIPAddress(const HostPtr &host);
    
    /**
     * This slot is called when the online state changed. It is connected
     * to the Smb4KHardwareInterface::onlineStateChanged() signal.
     */
    void slotOnlineStateChanged(bool online);

  private:
    /**
     * Pointer to Smb4KScannerPrivate class
     */
    const QScopedPointer<Smb4KScannerPrivate> d;
};

#endif
