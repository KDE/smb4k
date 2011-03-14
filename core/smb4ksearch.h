/***************************************************************************
    smb4ksearch  -  This class does custom searches
                             -------------------
    begin                : Tue Mar 08 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSEARCH_H
#define SMB4KSEARCH_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

// KDE includes
#include <kcompositejob.h>
#include <kdemacros.h>

// forward declarations
class Smb4KSearchPrivate;
class Smb4KSearchJob;
class Smb4KBasicNetworkItem;


class KDE_EXPORT Smb4KSearch : public KCompositeJob
{
  Q_OBJECT

  friend class Smb4KSearchPrivate;

  public:
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KSynchronizer class.
     */
    static Smb4KSearch *self();

    /**
     * Searches for specific search string in the network neighborhood.
     *
     * @param string          The search string
     *
     * @param parent          The parent widget
     */
    void search( const QString &string,
                 QWidget *parent = 0 );

    /**
     * This function tells you whether searches are running
     * or not.
     *
     * @returns TRUE if at least one search is running
     */
    bool isRunning();

    /**
     * With this function you can test whether a search is already/still
     * running.
     *
     * @returns TRUE if a search is already/still running
     */
    bool isRunning( const QString &string );

    /**
     * This function aborts all searches at once.
     */
    void abortAll();

    /**
     * This function aborts the searching for a certain search string 
     * in the network neighborhood.
     *
     * @param string          The search string
     */
    void abort( const QString &string );

    /**
     * This function starts the composite job
     */
    void start();

  signals:
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

    /**
     * This signal is emitted when the search returned a result.
     *
     * @param item          The network item
     *
     * @param known         Is the item already known?
     */
    void result( Smb4KBasicNetworkItem *item,
                 bool known );

  protected slots:
    /**
     * Invoked by start() function
     */
    void slotStartJobs();

    /**
     * Called when a job finished
     */
    void slotJobFinished( KJob *job );

    /**
     * Called when an authentication error occurred
     */
    void slotAuthError( Smb4KSearchJob *job );

    /**
     * Called when an search result was found
     */
    void slotProcessSearchResult( Smb4KBasicNetworkItem * );

    /**
     * Called when the program is about to quit
     */
    void slotAboutToQuit();

  private:
    /**
     * Constructor
     */
    Smb4KSearch();

    /**
     * Destructor
     */
    ~Smb4KSearch();
};

#endif
