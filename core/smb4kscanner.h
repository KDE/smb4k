/***************************************************************************
    smb4kscanner.h  -  The network scan core class of Smb4K.
                             -------------------
    begin                : Sam Mai 31 2003
    copyright            : (C) 2003-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSCANNER_H
#define SMB4KSCANNER_H

#ifndef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QCache>
#include <QList>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KBasicNetworkItem;
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class BasicScanThread;


/**
 * This class generates upon request the list of workgroups/domains, the list
 * of members of a certain workgroup/domain or the list of shared resources
 * (shares) a host offers. Additionally, you can query a host for extra
 * information.
 *
 * The discovered workgroups and hosts are stored in the global workgroups
 * and hosts list, respectively (@see Smb4KGlobal).
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KScanner : public QObject
{
  Q_OBJECT

  friend class Smb4KScannerPrivate;

  public:
    /**
     * Defines the lookup method
     */
    enum Method { LookupDomains,
                  LookupDomainMembers,
                  LookupShares,
                  LookupInfo };
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KScanner *self();

    /**
     * Scan the network neighborhood for domains and workgroups. How this
     * is done depends on the user's settings.
     *
     * You need to connect to the workgroups() signal to retrieve the results
     * of the search.
     */
    void lookupDomains();

    /**
     * Query the domain/workgroup master browser for the domain/workgroup
     * members.
     *
     * You need to connect to the hosts() signal to retrieve the results of
     * the search.
     *
     * @param workgroup   The workgroup that contains the master browser
     *                    that is to be queried.
     */
    void lookupDomainMembers( Smb4KWorkgroup *workgroup );

    /**
     * Query the host represented by @p host for its list of shared resources.
     *
     * You need to connect to the shares() signal to retrieve the results of
     * the search.
     * 
     * @note If set, Smb4KHost::protocol() and Smb4KHost::port() will overwrite 
     * the other prossible protocol or port options (e.g. default protocol/port
     * or custom protocol/port).
     *
     * @param host        The Smb4KHost item representing the host
     */
    void lookupShares( Smb4KHost *host );

     /**
     * Query a host for more info (i.e. server and OS string, etc.). If the
     * information was already gathered, this function immediately emits the
     * info() signal with the requested information and exits. If no information
     * was requested before, a query is started.
     *
     * @param host            The Smb4KHost item representing the host
     */
    void lookupInfo( Smb4KHost *host );

    /**
     * Aborts either the scan process for the network item @p item or all scan
     * processes of a specific kind of @p process.
     *
     * If you provide a non-null @p item object, it will always be favored over
     * the @p process entry.
     *
     * @param item            The network item object
     *
     * @param process         The process that is used
     */
    void abort( Smb4KBasicNetworkItem *item,
                int process );

    /**
     * Aborts all network scans at once.
     */
    void abortAll();

    /**
     * With this function you can check if a process for a single network
     * item @p item or all processes of a certain kind denoted by @p process
     * is/are (still) running.
     *
     * If you provide a non-null @p item object, it will always be favored over
     * the @p process entry.
     *
     * @param item            The network item object
     *
     * @param process         The process that is used
     *
     * @returns TRUE if the process is/the processes are (still) running.
     */
    bool isRunning( Smb4KBasicNetworkItem *item,
                    int process );

    /**
     * This function reports if the scanner is running or not.
     *
     * @returns             TRUE if the scanner is running and FALSE otherwise.
     */
    bool isRunning() { return !m_cache.isEmpty(); }

    /**
     * This function inserts a @p host into the list of known servers. If it belongs to
     * a workgroup that was not known until now, a new Smb4KWorkgroup item is also
     * created and added to the list of known workgroups. @p host is then marked as
     * pseudo master browser. On success, this function emits the hostAdded() and
     * hostListChanged() signals. If the host is already known, nothing is done.
     *
     * @param host          The host that should be inserted
     */
    void insertHost( Smb4KHost *host );

    /**
     * This function initializes the network the network browser. It starts the internal
     * timer that is needed to process the incoming requests and also starts the initial
     * network scan. You need to call this function before any other.
     */
    void init();

    /**
     * This function returns the current state of the scanner. The state is defined in the
     * smb4kdefs.h file.
     *
     * @returns the current state of the scanner.
     */
    int currentState() { return m_state; }

  signals:
    /**
     * This signal is emitted when the current run state changed. Use the currentState()
     * function to read the current run state.
     */
    void stateChanged();

    /**
     * This signal is emitted, when the workgroup list has been updated.
     *
     * @param list          The list of workgroups in the network neighborhood.
     */
    void workgroups( const QList<Smb4KWorkgroup *> &list );

    /**
     * Emits a @p list of hosts. If @p workgroup is non-zero, the list carries the members
     * of that workgroup. Otherwise, it is the list of all hosts that are available on the
     * network.
     *
     * @param workgroup     The workgroup the hosts belong to or NULL
     *
     * @param list          The list of hosts
     */
    void hosts( Smb4KWorkgroup *workgroup,
                const QList<Smb4KHost *> &list );

    /**
     * Emits the list of shares.
     *
     * @param host          The host that shares the resources
     *
     * @param list          The list of shares
     */
    void shares( Smb4KHost *host,
                 const QList<Smb4KShare *> &list );

    /**
     * This signal is emitted if addtional information for the host represented
     * by @p host was requested and is available.
     *
     * @param host          The Smb4KHost item that carries the requested information.
     */
    void info( Smb4KHost *host );

    /**
     * This signal is emitted when the list of hosts is changed.
     */
    void hostListChanged();

    /**
     * This signal is emitted when a host has been added to the list of known
     * hosts via the insertHost() function.
     *
     * @param host          The Smb4KHost that represents the host that was added.
     */
    void hostInserted( Smb4KHost *host );

    /**
     * This signal is emitted when a scan process is about to be started. It passes
     * the network @p item as well as the lookup method that is used to the receiver.
     *
     * Please note that the network @p item may be NULL. The @p process, however,
     * will always be valid.
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
     * Please note that the network @p item may be NULL. The @p process, however,
     * will always be valid.
     *
     * @param item          The Smb4KBasicNetworkItem object
     *
     * @param process       The process that was used
     */
    void finished( Smb4KBasicNetworkItem *item,
                   int process );

  protected slots:
    /**
     * This slot is called by the QApplication::aboutToQuit() signal.
     * Is does everything that has to be done before the program
     * really exits.
     */
    void slotAboutToQuit();

    /**
     * This slot is called when a thread finished.
     */
    void slotThreadFinished();

    /**
     * This slot takes a list of discovered workgroups and puts them into the global
     * list. Obsolete entries will be deleted and new ones created.
     *
     * @param workgroups    The list of workgroups.
     */
    void slotWorkgroups( QList<Smb4KWorkgroup> &workgroups );

    /**
     * This slot takes a list of discovered hosts @p hosts and puts them into the global
     * list. Obsolete entries will be deleted and new ones created.
     *
     * The @p workgroup item represents the workgroup where the hosts belong to. If
     * the @p workgroup is NULL, it is considered that the host list was retrieved
     * via a broadcast area lookup and is the list of available hosts.
     *
     * @param workgroup     The workgroup that the hosts belong to.
     *
     * @param hosts         The list of hosts
     */
    void slotHosts( Smb4KWorkgroup *workgroup,
                    QList<Smb4KHost> &hosts );

    /**
     * This slot takes a list of discovered shares @p shares and puts them into the global
     * list. Obsolete entries will be deleted and new ones created.
     *
     * The @p host item represents the host where the shares are located.
     *
     * @param host          The host that shares the resources
     *
     * @param shares        The list of shares
     */
    void slotShares( Smb4KHost *host,
                     QList<Smb4KShare> &shares );

    /**
     * This slot takes a host and processes the additional information that was looked up
     * by the lookup process. It will not only emit the host item but also update entries
     * in the global list, if necessary.
     *
     * @param host          The host item that carries the additional information
     */
    void slotInformation( Smb4KHost *host );

  private:
    /**
     * The constructor.
     */
    Smb4KScanner();

    /**
     * The destructor.
     */
    ~Smb4KScanner();

    /**
     * The current state.
     */
    int m_state;

    /**
     * The cache that holds the threads
     */
    QCache<QString,BasicScanThread> m_cache;
};
#endif
