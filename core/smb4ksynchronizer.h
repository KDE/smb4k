/***************************************************************************
    smb4ksynchronizer  -  This is the synchronizer of Smb4K.
                             -------------------
    begin                : Mo Jul 4 2005
    copyright            : (C) 2005-2009 by Alexander Reinholdt
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

#ifndef SMB4KSYNCHRONIZER_H
#define SMB4KSYNCHRONIZER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kprocess.h>

// Qt includes
#include <QObject>
#include <QString>
#include <QCache>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KShare;
class Smb4KSynchronizationInfo;
class SynchronizationThread;


/**
 * This is a core class of Smb4K. It manages the synchronization of remote
 * shares with a local copy (and vice versa).
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */


class KDE_EXPORT Smb4KSynchronizer : public QObject
{
  Q_OBJECT

  friend class Smb4KSynchronizerPrivate;

  public:
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KSynchronizer class.
     */
    static Smb4KSynchronizer *self();

    /**
     * This function synchronizes a destination with the source. It takes a
     * pointer to an Smb4KSynchronizationInfo object. This will then be filled
     * with the data received via stdout and emitted by the progress() signal.
     *
     * @warning You must not delete the pointer while the synchronization process is
     * running. If you do so, you will most likely experience crashes.
     *
     * @param info        The pointer to the Smb4KSynchronizationInfo object
     */
    void synchronize( Smb4KSynchronizationInfo *info );

    /**
     * This function reports if a synchronization process for @p info is currently
     * running.
     *
     * @param info        The Smb4KSynchronizationInfo object.
     *
     * @returns TRUE if the process is running.
     */
    bool isRunning( Smb4KSynchronizationInfo *info );

    /**
     * This function reports if the synchronizer is running or not. This function
     * will return TRUE as long as at least one synchronization thread is running.
     * To check if a certain synchronization process is running, use the
     * @see isRunning( int pid ) function.
     *
     * @returns TRUE if the synchronizer is running and FALSE otherwise.
     */
    bool isRunning() { return m_working; }

    /**
     * This function aborts the synchronization process that is represented by a
     * certain Smb4KSynchronizationInfo object @p info.
     *
     * @param info        The Smb4KSynchronizationInfo object
     */
    void abort( Smb4KSynchronizationInfo *info );

    /**
     * With this function you can check if the synchronization process represented
     * by @p info was aborted. It will return TRUE only if it was forcibly ended
     * using the @see abort( Smb4KSynchronizationInfo *info ) function.
     *
     * @param info        The Smb4KSynchronizationInfo object
     *
     * @returns TRUE if the process was aborted.
     */
    bool isAborted( Smb4KSynchronizationInfo *info );

    /**
     * This function aborts all synchronization processes that are currently
     * running.
     */
    void abortAll();

    /**
     * This function returns the current state of the synchronizer. The state
     * is defined in the smb4kdefs.h file.
     *
     * @returns the current state of the synchronizer.
     */
    int currentState() { return m_state; }

  signals:
    /**
     * This signal is emitted when the current run state changed. Use the currentState()
     * function to read the current run state.
     */
    void stateChanged();

    /**
     * This signal is emitted just before the synchronization process is
     * started.
     *
     * @param info        The Smb4KSynchronizationInfo object that was passed to
     *                    synchronize().
     */
    void aboutToStart( Smb4KSynchronizationInfo *info );

    /**
     * This signal is emitted after the synchronization process finished.
     *
     * @param info        The Smb4KSynchronizationInfo object that was passed to
     *                    synchronize().
     */
    void finished( Smb4KSynchronizationInfo *info );

    /**
     * Emit information about the progress of current synchronization process.
     * The information that's available may only be partial, i.e. that maybe
     * the file name or the rate or somthing else is missing. That's because
     * of the way the output of rsync is processed.
     *
     * @param info        Information about progress of the synchronization
     * process
     */
    void progress( Smb4KSynchronizationInfo *info );

  protected slots:
    /**
     * This slot is connected to QCoreApplication::aboutToQuit() signal.
     * It aborts all running processes.
     */
    void slotAboutToQuit();

    /**
     * This slot is called when a thread finished.
     */
    void slotThreadFinished();

  private:
    /**
     * The constructor of the synchronizer.
     */
    Smb4KSynchronizer();

    /**
     * The destructor.
     */
    ~Smb4KSynchronizer();

    /**
     * The cache that stores the threads.
     */
    QCache<QString, SynchronizationThread> m_cache;

    /**
     * This boolean is TRUE if the synchronizer is working and FALSE otherwise.
     */
    bool m_working;

    /**
     * The state.
     */
    int m_state;
};

#endif
