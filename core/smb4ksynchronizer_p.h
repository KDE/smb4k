/***************************************************************************
    smb4ksynchronizer_p  -  This file contains private helper classes for
    the Smb4KSynchronizer class.
                             -------------------
    begin                : Fr Okt 24 2008
    copyright            : (C) 2008-2014 by Alexander Reinholdt
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

#ifndef SMB4KSYNCHRONIZER_P_H
#define SMB4KSYNCHRONIZER_P_H

// application specific includes
#include "smb4ksynchronizer.h"
#include "smb4kprocess.h"

// KDE includes
#include <kjob.h>
#include <kurlrequester.h>
#include <kdialog.h>
#include <kuiserverjobtracker.h>

// forward declarations
class Smb4KShare;

class Smb4KSyncJob : public KJob
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    explicit Smb4KSyncJob( QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KSyncJob();

    /**
     * Returns TRUE if the job was started and FALSE
     * otherwise
     * 
     * @returns TRUE if the job was started
     */
    bool isStarted() { return m_started; }

    /**
     * Starts the synchronization
     */
    void start();

    /**
     * Setup the synchronization process. You need to set the share, the parent
     * widget is optional.
     *
     * This function must be called before start() is run.
     *
     * @param src       The source
     *
     * @param dest      The destination
     */
    void setupSynchronization( Smb4KShare *share,
                               QWidget *parent = 0 );

    /**
     * Returns the source directory.
     *
     * @returns the source directory.
     */
    const KUrl &source() { return m_src; }

    /**
     * Returns the destination directory.
     *
     * @returns the destination directory.
     */
    const KUrl &destination() { return m_dest; }

  Q_SIGNALS:
    /**
     * This signal is emitted when a job is started. The emitted path
     * is the one of the destination.
     *
     * @param dest        The destination's URL
     */
    void aboutToStart( const QString &dest );

    /**
     * This signal is emitted when a job has finished. The emitted
     * URL is the one of the destination.
     *
     * @param dest        The destination's URL
     */
    void finished( const QString &dest );
     
  protected:
    /**
     * Reimplemented from KJob. Kills the internal process and
     * then the job itself.
     */
    bool doKill();
    
  protected Q_SLOTS:
    void slotStartSynchronization();
    void slotReadStandardOutput();
    void slotReadStandardError();
    void slotProcessFinished( int exitCode, QProcess::ExitStatus status );

  private:
    bool m_started;
    Smb4KShare *m_share;
    QWidget *m_parent_widget;
    KUrl m_src;
    KUrl m_dest;
    Smb4KProcess *m_proc;
    KUiServerJobTracker *m_job_tracker;
};


class Smb4KSynchronizationDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param share         The share item
     *
     * @param parent        The parent widget
     */
    explicit Smb4KSynchronizationDialog( Smb4KShare *share, QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSynchronizationDialog();

    /**
     * The source URL
     */
    const KUrl source();

    /**
     * The destination URL
     */
    const KUrl destination();

  protected Q_SLOTS:
    /**
     * This slot is called when the User1 button is clicked.
     * It initializes the synchronization.
     */
    void slotUser1Clicked();

    /**
     * This slot is called when the User2 button is clicked.
     * It swaps the source and destination.
     */
    void slotUser2Clicked();

    /**
     * This slot is called when the Cancel button is clicked.
     * It aborts any action the synchronizer is performing.
     */
    void slotUser3Clicked();

  private:
    /**
     * A pointer to the share object
     */
    Smb4KShare *m_share;

    /**
     * The source URL requester
     */
    KUrlRequester *m_source;

    /**
     * The destination URL requester
     */
    KUrlRequester *m_destination;
};


class Smb4KSynchronizerPrivate
{
};


class Smb4KSynchronizerStatic
{
  public:
    Smb4KSynchronizer instance;
};

#endif
