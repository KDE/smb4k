/***************************************************************************
    smb4ksearch  -  This class searches for custom search strings.
                             -------------------
    begin                : So Apr 27 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

#ifndef SMB4KSEARCH_H
#define SMB4KSEARCH_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QCache>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KBasicNetworkItem;
class SearchThread;

/**
 * This class searches for network items (hosts and shares) and returns them.
 *
 * This class belongs to the core of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */


class KDE_EXPORT Smb4KSearch : public QObject
{
  Q_OBJECT

  friend class Smb4KSearchPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KSearch *self();

    /**
     * Search for a given search item. This can either be a share or a host.
     * The search will be done by default by using Samba's smbtree program.
     * If you provided an IP address, nmblookup will be used automatically.
     *
     * @param string            The search item
     */
    void search( const QString &string );

    /**
     * Aborts the scan process that looks for the search item @p string.
     *
     * @param string            The search item
     */
    void abort( const QString &string );

    /**
     * With this function you can check if the search process represented
     * by @p string was aborted. It will return TRUE only if it was forcibly ended
     * using the @see abort( const QString &string ) function.
     *
     * @param info        The search string
     *
     * @returns TRUE if the process was aborted.
     */
    bool isAborted( const QString &string );

    /**
     * Aborts all running search processes.
     */
    void abortAll();

    /**
     * With this function you can check if the process for the search item
     * @p string is (still) running.
     *
     * @param string            The search item
     *
     * @returns TRUE if the search for search item @p string is running and
     * FALSE otherwise.
     */
    bool isRunning( const QString &string );

    /**
     * This function reports if the search is running or not.
     *
     * @returns TRUE if at least one search is running and FALSE otherwise.
     */
    bool isRunning() { return !m_cache.isEmpty(); }

    /**
     * This function returns the current state of the previewer. The state is
     * defined in the smb4kdefs.h file.
     *
     * @returns the current state of the mounter.
     */
    int currentState() { return m_state; }

  signals:
    /**
     * This signal is emitted when the current run state changed. Use the currentState()
     * function to read the current run state.
     */
    void stateChanged();

    /**
     * This signal emits a network item that matches the search
     * criterion.
     *
     * @param item              The Smb4KBasicNetworkItem object
     *
     * @param known             TRUE if the network item is known to the application
     *                          and FALSE otherwise. In case of a share, "known" means
     *                          mounted.
     */
    void result( Smb4KBasicNetworkItem *item,
                 bool known );

    /**
     * This signal is emitted when a search process is about to be started. It passes
     * the search string to the receiver.
     *
     * @param string        The search string
     */
    void aboutToStart( const QString &string );

    /**
     * This signal is emitted when a search process has finished. It passes the
     * search string to the receiver.
     *
     * @param string        The search string
     */
    void finished( const QString &string );

  protected slots:
    /**
     * This slot is connected to QCoreApplication::aboutToQuit() signal.
     * It aborts all running processes.
     */
    void slotAboutToQuit();

    /**
     * This slot is called whenever a search result is emitted by a thread.
     * It does the necessary last adjustments before the result is made
     * available by emitting it.
     *
     * @param item          The network item that was discovered.
     */
    void slotProcessSearchResult( Smb4KBasicNetworkItem *item );

    /**
     * This slot is called when a thread finished.
     */
    void slotThreadFinished();

  private:
    /**
     * The constructor.
     */
    Smb4KSearch();

    /**
     * The destructor.
     */
    ~Smb4KSearch();

    /**
     * The state
     */
    int m_state;

    /**
     * The cache that holds the threads
     */
   QCache<QString,SearchThread> m_cache;
};

#endif
