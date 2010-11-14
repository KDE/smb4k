/***************************************************************************
    smb4kmounter.h  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
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

#ifndef SMB4KMOUNTER_H
#define SMB4KMOUNTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QFile>
#include <QString>
#include <QCache>

// KDE includes
#include <kdemacros.h>
#include <kauth.h>

// application specific includes
#include <smb4ksolidinterface.h>

// forward declarations
class Smb4KShare;
class Smb4KAuthInfo;
class BasicMountThread;

using namespace KAuth;

/**
 * This is one of the core classes of Smb4K. It manages the mounting
 * and unmounting of remote Samba/Windows shares. Additionally it maintains a
 * list of all mounts with SMBFS and CIFS file system that are present
 * on the system.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KMounter : public QObject
{
  Q_OBJECT

  friend class Smb4KMounterPrivate;

  public:
    /**
     * This enumeration denotes the process
     */
    enum Process { MountShare,
                   UnmountShare,
                   UnmountAllShares };

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
     */
    void mountShare( Smb4KShare *share );

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
     */
    void unmountShare( Smb4KShare *share,
                       bool force = false,
                       bool silent = false );

    /**
     * Unmounts all shares at once. It invokes unmountShare() on each share
     * in the global list of mounted shares.
     */
    void unmountAllShares();

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
    bool isRunning() { return !m_cache.isEmpty(); }

    /**
     * This function executes Smb4KMounter::slotAboutToQuit(). Under normal circumstances,
     * there is no need to use this function, because everything that is done here is
     * also done when QApplication::aboutToQuit() is emitted. However, it comes in handy
     * when you need to perform last actions in a plugin.
     */
    void prepareForShutdown();

    /**
     * This function initializes import of mounted shares and the remounting of recently
     * used shares.
     */
    void init();

    /**
     * This function returns the current state of the mounter. The state is defined in the
     * smb4kdefs.h file.
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
     * This signal is emitted when a mount process for the share @p share is
     * about to be started.
     *
     * @param share             The Smb4KShare object
     *
     * @param process           The kind of process
     */
    void aboutToStart( Smb4KShare *share,
                       int process );

    /**
     * This signal is emitted when the mount process for the share @p share
     * finished.
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
     * This slot is called by the QApplication::aboutToQuit() signal.
     * Is does everything that has to be done before the program
     * really exits.
     */
    void slotAboutToQuit();

    /**
     * This slot is called when an action finished.
     * 
     * @param reply       The KAuth::ActionReply object
     */
    void slotActionFinished( ActionReply reply );
    
    /**
     * This slot is called whenever a share was successfully mounted by the 
     * mount action.
     *
     * @param reply       The KAuth::ActionReply object
     */
    void slotShareMounted( ActionReply reply );
    
    /**
     * This slot is called whenever a share was successfully unmounted by the
     * unmount action.
     * 
     * @param reply       The KAuth::ActionReply object
     */
    void slotShareUnmounted( ActionReply reply );
    
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
     * Checks the file system of the share and its accessibility and set
     * the appropriate values accordingly. Additionally, the free and total
     * disk space are determined.
     *
     * @param share           The share that should be checked.
     */
    void check( Smb4KShare *share );

    /**
     * Save all shares that need to be remounted.
     */
    void saveSharesForRemount();
    
    /**
     * Create a mount action.
     *
     * @param share           The share object from which a KAuth::Action is to be
     *                        created
     *
     * @param action          The KAuth::Action object
     * 
     * @returns TRUE if the cration was successful.
     */
    bool createMountAction( Smb4KShare *share,
                            Action *action );

    /**
     * Create an unmount action.
     *
     * @param share           The share object from which a KAuth::Action is to be
     *                        created
     *
     * @param force           Force the unmounting
     * 
     * @param silent          Do not report any errors
     *
     * @param action          The KAuth::Action object
     * 
     * @returns TRUE if the cration was successful.
     */
    bool createUnmountAction( Smb4KShare *share,
                              bool force,
                              bool silent,
                              Action *action );
                            
    /**
     * The current state.
     */
    int m_state;

    /**
     * Timer ID
     */
    int m_timer_id;

    /**
     * Time out
     */
    int m_timeout;

    /**
     * The cache that holds the currently performed
     * actions
     */
    QCache<QString,Action> m_cache;
};

#endif
