/***************************************************************************
    smb4kscanner  -  This class retrieves all workgroups, servers and
    shares found on the network neighborhood
                             -------------------
    begin                : So Mai 22 2011
    copyright            : (C) 2011-2013 by Alexander Reinholdt
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
#include "smb4knetworkobject.h"
#include "smb4kglobal.h"

// Qt includes
#include <QtCore/QTimerEvent>
#include <QtCore/QScopedPointer>
#include <QtGui/QWidget>
#include <QtDeclarative/QDeclarativeListProperty>

// KDE includes
#include <kcompositejob.h>
#include <kdemacros.h>

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

class KDE_EXPORT Smb4KScanner : public KCompositeJob
{
  Q_OBJECT

  Q_PROPERTY( QDeclarativeListProperty<Smb4KNetworkObject> workgroups READ workgroups NOTIFY workgroupsListChanged )
  Q_PROPERTY( QDeclarativeListProperty<Smb4KNetworkObject> hosts READ hosts NOTIFY hostsListChanged )
  Q_PROPERTY( QDeclarativeListProperty<Smb4KNetworkObject> shares READ shares NOTIFY sharesListChanged )
  Q_PROPERTY( bool running READ isRunning CONSTANT )

  friend class Smb4KScannerPrivate;

  public:
    /**
     * Constructor
     */
    explicit Smb4KScanner( QObject *parent = 0 );

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
     * This function tests whether a process @p process is running. If you also
     * define the network item, you can test if a certain process is running.
     *
     * @param process         The group of processes
     *
     * @param item            The network item for a more fine grained testing
     */
    bool isRunning( Smb4KGlobal::Process process,
                    Smb4KBasicNetworkItem *item = NULL );

    /**
     * This function aborts all scan jobs at once.
     */
    Q_INVOKABLE void abortAll();

    /**
     * This function aborts a scan jobs that match the combination
     * of arguments passed. The network item entry may be NULL, the 
     * process must always be defined.
     *
     * The @p item argument can be used to specifically define one single
     * process to be killed. If you only define @p process, a whole group
     * of processes is going to be killed.
     *
     * @param process         The group of processes that are to be killed
     *
     * @param item            The network item for a more fine grained killing
     */
    void abort( Smb4KGlobal::Process process,
                Smb4KBasicNetworkItem *item = NULL );

    /**
     * This function starts the composite job
     */
    Q_INVOKABLE void start();

    /**
     * This function looks up all domains and workgroups in the 
     * network neighborhood. Hown this is done depends on the 
     * settings the user chose.
     *
     * @param parent          The parent widget
     */
    void lookupDomains( QWidget *parent = 0 );

    /**
     * This function looks up all hosts in a certain domain or
     * workgroup.
     * 
     * @param workgroup       The workgroup object
     *
     * @param parent          The parent widget
     */
    void lookupDomainMembers( Smb4KWorkgroup *workgroup,
                              QWidget *parent = 0 );

    /**
     * This function looks up all shared resources a certain 
     * @p host provides.
     * 
     * @param host            The host object
     * 
     * @param parent          The parent widget
     */
    void lookupShares( Smb4KHost *host,
                       QWidget *parent = 0 );

    /**
     * This function looks up additional information from a
     * certain @p host.
     *
     * @param host            The host object
     * 
     * @param parent          The parent widget
     */
    void lookupInfo( Smb4KHost *host,
                     QWidget *parent = 0 );

    /**
     * This function returns the list of workgroups. Basically, this is the 
     * Smb4KGlobal::workgroupsList() list converted into a list of Smb4KNetworkItem
     * objects.
     * 
     * @returns the list of discovered workgroups.
     */
    QDeclarativeListProperty<Smb4KNetworkObject> workgroups();

    /**
     * This function returns the list of hosts. Basically, this is the
     * Smb4KGlobal::hostsList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of discovered hosts.
     */
    QDeclarativeListProperty<Smb4KNetworkObject> hosts();

    /**
     * This function returns the list of shares. Basically, this is the
     * Smb4KGlobal::sharesList() list converted into a list of Smb4KNetworkItem
     * objects.
     *
     * @returns the list of discovered shares.
     */
    QDeclarativeListProperty<Smb4KNetworkObject> shares();
    
    /**
     * This function takes a KUrl object and initiates a network scan depending 
     * on the @param type of the network item.
     * 
     * Please note that this function only works with network objects that are 
     * already known. All others will be ignored.
     * 
     * @param url         The URL of the network item
     * 
     * @param type        The type of the network item
     */
    Q_INVOKABLE void lookup( const QUrl &url, int type );
    
    /**
     * This function takes a KUrl object, looks up the respective network object
     * and returns it. If there is not such an object, NULL is returned.
     * 
     * Please note that this function only works with network objects that are 
     * already known. All others will be ignored.
     * 
     * @param url         The URL of the network item
     * 
     * @param type        The type of the network item
     * 
     * @returns The network item or NULL if it was not found.
     */
    Q_INVOKABLE Smb4KNetworkObject *find( const QUrl &url, int type );
    
  protected:
    /**
     * Reimplemented from QObject
     */
    void timerEvent( QTimerEvent *e );

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
    void aboutToStart( Smb4KBasicNetworkItem *item,
                       int process );

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
    void finished( Smb4KBasicNetworkItem *item,
                   int process );

    /**
     * This signal emits the list of workgroups that were discovered.
     *
     * @param workgroups   The list of workgroups
     */
    void workgroups( const QList<Smb4KWorkgroup *> &workgroups );

    /**
     * This signal is emitted when the list of workgroups changed.
     */
    void workgroupsListChanged();

    /**
     * This signal emits the list of hosts that were discovered.
     *
     * @param workgroup   The workgroup that was scanned. This argument
     *                    is NULL in case several workgroups are present
     *                    in the hosts list.
     *
     * @param hosts       The list of hosts
     */
    void hosts( Smb4KWorkgroup *workgroup,
                const QList<Smb4KHost *> &hosts );

    /**
     * This signal is emitted when the list of hosts changed. You
     * need to check Smb4KGlobal::hostsList() yourself for changes if
     * you connect to this signal.
     */
    void hostsListChanged();
    
    /**
     * This signal is emitted when the list of shares are certain host 
     * offers has been acquired.
     * 
     * @param host        The host that was queried
     * 
     * @param shares      The list of shares belonging to @p host
     */
    void shares( Smb4KHost *host,
                 const QList<Smb4KShare *> &shares );

    /**
     * This signal is emitted when the list of shares changed.
     */
    void sharesListChanged();
    
    /**
     * This signal is emitted when the additional information has been
     * acquired from a certain host.
     * 
     * @param host        The host with the acquired information
     */
    void info( Smb4KHost *host );
    
    /**
     * This signal is emitted when an authentication error occurred.
     * 
     * @param host        The host that is affected
     */
    void authError( Smb4KHost *host,
                    int process );

    /**
     * This signal is emitted when an IP address was successfully looked
     * up.
     *
     * @param host          The host
     */
    void ipAddress( Smb4KHost *host );

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
    void slotJobFinished( KJob *job );

    /**
     * Called when an authentication error occurred while a master 
     * browser was queried for the list of workgroups/domains
     */
    void slotAuthError( Smb4KQueryMasterJob *job );
    
    /**
     * Called when an authentication error occurred while a master
     * browser was queried for the list of workgroup/domain members
     */
    void slotAuthError( Smb4KLookupDomainMembersJob *job );
    
    /**
     * Called when an authentication error occurred while a host
     * was queried for the list of shares
     */
    void slotAuthError( Smb4KLookupSharesJob *job );

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
    void slotAboutToStartHostsLookup( Smb4KWorkgroup *workgroup );

    /**
     * A lookup process for hosts has finished
     */
    void slotHostsLookupFinished( Smb4KWorkgroup *workgroup );
    
    /**
     * A lookup process for shares is about to be started
     */
    void slotAboutToStartSharesLookup( Smb4KHost *host );

    /**
     * A lookup process for shares has finished
     */
    void slotSharesLookupFinished( Smb4KHost *host  );
    
    /**
     * A lookup process for shares is about to be started
     */
    void slotAboutToStartInfoLookup( Smb4KHost *host );

    /**
     * A lookup process for shares has finished
     */
    void slotInfoLookupFinished( Smb4KHost *host  );

    /**
     * Is called when workgroups and domains have been looked
     * up
     */
    void slotWorkgroups( const QList<Smb4KWorkgroup *> &workgroups_list );

    /**
     * Is called when hosts have been looked up by the IP scan method
     */
    void slotHosts( const QList<Smb4KHost *> &hosts_list );

    /**
     * Is called when hosts have been looked up by the normal lookup
     * method
     */
    void slotHosts( Smb4KWorkgroup *workgroup,
                    const QList<Smb4KHost *> &hosts_list );
    
    /**
     * Is called when shares have been looked up
     */
    void slotShares( Smb4KHost *host,
                     const QList<Smb4KShare *> &shares_list );
    
    /**
     * Is called when additional information has been acquired
     */
    void slotInfo( Smb4KHost *host );

    /**
     * Is called when an IP address was looked up
     */
    void slotProcessIPAddress( Smb4KHost *host );

  private:
    /**
     * Pointer to Smb4KScannerPrivate class
     */
    const QScopedPointer<Smb4KScannerPrivate> d;
};

#endif
