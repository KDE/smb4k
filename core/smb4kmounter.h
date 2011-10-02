/***************************************************************************
    smb4kmounter.h  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KMOUNTER_H
#define SMB4KMOUNTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QFile>
#include <QString>
#include <QStringList>

// KDE includes
#include <kdemacros.h>
#include <kauth.h>
#include <kcompositejob.h>

// application specific includes
#include <smb4ksolidinterface.h>

// forward declarations
class Smb4KShare;
class Smb4KAuthInfo;
class Smb4KMountDialog;
class Smb4KMountJob;
class Smb4KUnmountJob;

using namespace KAuth;

/**
 * This is one of the core classes of Smb4K. It manages the mounting
 * and unmounting of remote Samba/Windows shares. Additionally it maintains a
 * list of all mounts with SMBFS and CIFS file system that are present
 * on the system.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KMounter : public KCompositeJob
{
  Q_OBJECT

  friend class Smb4KMounterPrivate;

  public:
    /**
     * This enumeration denotes the process
     */
    enum Process { MountShare,
                   UnmountShare };

    /**
     * Returns a static pointer to this class.
     */
    static Smb4KMounter *self();

    /**
     * Aborts the mount/unmount process that is running for the share @p share.
     *
     * @param share           The Smb4KShare object
     */
    void abort( Smb4KShare *share );

    /**
     * Aborts all running processes at once.
     */
    void abortAll();

    /**
     * This function attempts to mount a share.
     *
     * @param share       The Smb4KShare object that is representing the share.
     * 
     * @param parent      The parent widget
     */
    void mountShare( Smb4KShare *share,
                     QWidget *parent = 0 );

    /**
     * Mount a share via a mount dialog. The mount dialog is opened and you have
     * to enter the UNC and optionally the workgroup and IP address.
     *
     * @param parent      The parent widget of this dialog
     */
    void openMountDialog( QWidget *parent = 0 );

    /**
     * Mounts a list of shares at once.
     *
     * @param shares      The list of shares
     * 
     * @param parent      The parent widget
     */
    void mountShares( const QList<Smb4KShare *> &shares,
                      QWidget *parent = 0 );

    /**
     * This function attempts to unmount a share. This can either be done the "normal"
     * way, or you may force it. If you decide to force the unmounting by setting
     * @p force to TRUE, under Linux a lazy unmount will be initiated. With the parameter
     * @p silent you can suppress any error messages.
     *
     * @param share       The share object that should be unmounted.
     *
     * @param force       Force the unmounting of the share.
     *
     * @param silent      Determines whether this function should emit an error code in
     *                    case of an error. The default value is FALSE.
     * 
     * @param parent      The parent widget
     */
    void unmountShare( Smb4KShare *share,
                       bool silent = false,
                       QWidget *parent = 0 );
    
    /**
     * This function attempts to unmount a list of shares. This can either be done the "normal"
     * way, or you may force it. If you decide to force the unmounting by setting
     * @p force to TRUE, under Linux a lazy unmount will be initiated. With the parameter
     * @p silent you can suppress any error messages.
     * 
     * @param shares      The list of shares that is to be unmounted
     * 
     * @param force       Force the unmounting of the share
     * 
     * @param silent      Determines whether this function should emit an error code in
     *                    case of an error. The default value is FALSE.
     * 
     * @param parent      The parent widget
     */
    void unmountShares( const QList<Smb4KShare *> &shares,
                        bool silent = false,
                        QWidget *parent = 0 );

    /**
     * Unmounts all shares at once. This is a convenience function. It calls
     * unmountShares() to unmount all currently mounted shares.
     * 
     * @param parent      The parent widget
     */
    void unmountAllShares( QWidget *parent = 0 );

    /**
     * This function reports if the mount process for @p share is running.
     *
     * @param share       The Smb4KShare object
     *
     * @returns TRUE if the process is running.
     */
    bool isRunning( Smb4KShare *share );

    /**
     * This function reports if the mounter is running or not.
     *
     * @returns TRUE if the mounter is running and FALSE otherwise.
     */
    bool isRunning() { return hasSubjobs(); }

    /**
     * This function starts the composite job
     */
    void start();

  signals:
    /**
     * This signal is emitted whenever a share item was updated. This mainly happens
     * from within the import() function.
     * 
     * @param share             The share item that was just updated.
     */ 
    void updated( Smb4KShare *share );

    /**
     * This signal is emitted when a share has successfully been mounted.
     *
     * @param share             The share that was just mounted.
     */
    void mounted( Smb4KShare *share );

    /**
     * This signal is emitted after a share was unmounted and directly before
     * it is removed from the global list of mounted shares.
     * 
     * Please note that this signal may be emitted BEFORE a share is actually
     * removed from the global list of shares! 
     *
     * @param share            The share that is going to be unmounted.
     */
    void unmounted( Smb4KShare *share );

    /**
     * This signal is emitted when a mount/unmount process for the share 
     * @p share is about to be started.
     *
     * @param share             The Smb4KShare object
     *
     * @param process           The kind of process
     */
    void aboutToStart( Smb4KShare *share,
                       int process );

    /**
     * This signal is emitted when the mount/unmount process for the share 
     * @p share finished.
     *
     * @param share             The Smb4KShare object
     *
     * @param process           The kind of process
     */
    void finished( Smb4KShare *share,
                   int process );

  protected:
    /**
     * Reimplemented from QObject to process the queue.
     *
     * @param event             The QTimerEvent event
     */
    void timerEvent( QTimerEvent *event );

  protected slots:
    /**
     * Starts the composite job
     */
    void slotStartJobs();
    
    /**
     * This slot is called by the QApplication::aboutToQuit() signal.
     * Is does everything that has to be done before the program
     * really exits.
     */
    void slotAboutToQuit();
    
    /**
     * This slot is called when a job finished
     * 
     * @param job         The job that finished
     */
    void slotJobFinished( KJob *job );
    
    /**
     * Called when an authentication error occurred
     */
    void slotAuthError( Smb4KMountJob *job );

    /**
     * Called when mounting for one or more shares should be retried
     */
    void slotRetryMounting( Smb4KMountJob *job );

    /**
     * This slot is called whenever a share was successfully mounted by the 
     * mount job.
     *
     * @param share       The share object
     */
    void slotShareMounted( Smb4KShare *share );
    
    /**
     * This slot is called whenever a share was successfully unmounted by the
     * unmount job.
     * 
     * @param reply       The share object
     */
    void slotShareUnmounted( Smb4KShare *share );
    
    /**
     * This slot is called by the Solid interface when a hardware button was
     * pressed. It is used to unmount shares according to the wishes of the
     * user.
     *
     * @param type            The button type
     */
    void slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType type );

    /**
     * This slot is called by the Solid interface when the computer woke up
     * from a sleep or stand-by state. It initializes the remounting of the
     * previously unmounted shares.
     */
    void slotComputerWokeUp();

    /**
     * This slot is called by the Solid interface when the network status
     * changed. It is used to initialize network actions when the network
     * became available.
     *
     * @param status          The new network status
     */
    void slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus status );

    /**
     * Called when a mount job started. It just emits the 
     * aboutToStart( Smb4KShare *, MountShare ) signal for each share
     * that is processed.
     *
     * @param shares          The list of shares that are going to be mounted
     */
    void slotAboutToStartMounting( const QList<Smb4KShare> &shares );

    /**
     * Called when a mount job has finished. It just emits the
     * finished( Smb4KShare *, MountShare ) signal for each share that 
     * was processed.
     *
     * @param shares          The shares that were mounted
     */
    void slotFinishedMounting( const QList<Smb4KShare> &shares );
    
    /**
     * Called when an unmount job started. It just emits the 
     * aboutToStart( Smb4KShare *, UnmountShare ) signal for each share
     * that is processed.
     *
     * @param shares          The shares that are going to be unmounted
     */
    void slotAboutToStartUnmounting( const QList<Smb4KShare> &shares );

    /**
     * Called when an unmount job has finished. It just emits the
     * finished( Smb4KShare *, UnmountShare ) signal for each share that
     * was processed.
     *
     * @param shares          The shares that were unmounted
     */
    void slotFinishedUnmounting( const QList<Smb4KShare> &shares );
    
    /**
     * Called when a mounted share has been stat'ed.
     * 
     * @param job             The KIO::StatJob
     */
    void slotStatResult( KJob *job );

  private:
    /**
     * The constructor.
     */
    Smb4KMounter();
    /**
     * The destructor.
     */
    ~Smb4KMounter();

    /**
     * Trigger the remounting of shares.
     */
    void triggerRemounts();

    /**
     * Imports mounted shares.
     */
    void import();

    /**
     * Checks the accessibility, the UID and GID and the usage of
     * a share.
     * 
     * Only use this function when you are absolutely sure that a 
     * share did not vanish, i.e. the server was shut down. Otherwise
     * you will provoke lock-ups that render the application at least
     * temporarily useless.
     * 
     * @param share           The share that should be checked.
     */
    void check( Smb4KShare *share );

    /**
     * Save all shares that need to be remounted.
     */
    void saveSharesForRemount();
    
    /**
     * Time out
     */
    int m_timeout;

    /**
     * Retries
     */
    QList<Smb4KShare> m_retries;
    
    /**
     * Shares that were imported from /proc/mounts or by reading
     * KMountpoint stuff.
     */
    QList<Smb4KShare> m_imported_shares;

    /**
     * The mount dialog
     */
    Smb4KMountDialog *m_dialog;
};

#endif
