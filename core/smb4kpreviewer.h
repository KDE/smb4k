/***************************************************************************
    smb4kpreviewer  -  This class queries a remote share for a preview
                             -------------------
    begin                : Mo Mai 28 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

#ifndef SMB4KPREVIEWER_H
#define SMB4KPREVIEWER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QString>
#include <QCache>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KPreviewItem;
class PreviewThread;

/**
 * This class is part of the core of Smb4K. It queries a remote SMB share for
 * a preview and returns the result.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPreviewer : public QObject
{
  Q_OBJECT

  friend class Smb4KPreviewerPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KPreviewer *self();

    /**
     * Get a preview of the contents of @p item.
     *
     * In the case that @p item represents a 'homes' share, the user will be
     * prompted for the user name with which he wants to log in and the share
     * name of @p item will be set to the result.
     *
     * To retrieve the preview, you need to connect to the result() signal.
     *
     * @param item              The item for which a preview should be
     *                          requested.
     */
    void preview( Smb4KPreviewItem *item );

    /**
     * This function reports if the preview process for the share represented
     * by @p item is running.
     *
     * @param item              The Smb4KPreviewItem object
     *
     * @returns TRUE if the process is running.
     */
    bool isRunning( Smb4KPreviewItem *item );

    /**
     * Using this function, you can find out whether the previewer is running
     * at the moment.
     *
     * @returns TRUE if the previewer is running or FALSE otherwise.
     */
    bool isRunning() { return !m_cache.isEmpty(); }

    /**
     * Abort any action the previewer is performing at the moment.
     */
    void abortAll();

    /**
     * Aborts the preview process that is running for the share represented by
     * @p item.
     *
     * @param item              The Smb4KPreviewItem object
     */
    void abort( Smb4KPreviewItem *item );

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
     * Emits the preview after the process exited successfully. Get the contents
     * of the remote share by looping through the Smb4KPreviewItem::contents() list.
     *
     * @param item              The item for which the preview was received.
     */
    void result( Smb4KPreviewItem *item );

    /**
     * This signal is emitted when a preview process for the share represented
     * by the preview item @p item is about to be started.
     *
     * @param item              The Smb4KPreviewItem object
     */
    void aboutToStart( Smb4KPreviewItem *item );

    /**
     * This signal is emitted when the preview process for the share represented
     * by the preview item @p item finished.
     *
     * @param item              The Smb4KPreviewItem object
     */
    void finished( Smb4KPreviewItem *item );

  protected slots:
    /**
     * This slot is called when a thread finished.
     */
    void slotThreadFinished();

  private:
    /**
     * The constructor
     */
    Smb4KPreviewer();

    /**
     * The destructor
     */
    ~Smb4KPreviewer();

    /**
     * The cache that holds the threads
     */
    QCache<QString, PreviewThread> m_cache;

    /**
     * The current state
     */
    int m_state;
};

#endif
