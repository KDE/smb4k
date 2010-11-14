/***************************************************************************
    smb4kprint  -  The printing core class.
                             -------------------
    begin                : Tue Mar 30 2004
    copyright            : (C) 2004-2010 by Alexander Reinholdt
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

#ifndef SMB4KPRINT_H
#define SMB4KPRINT_H

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
class Smb4KPrintInfo;
class Smb4KAuthInfo;
class PrintThread;


/**
 * This is a core class. It provides the interface for printing documents over
 * the network neighborhood.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KPrint : public QObject
{
  Q_OBJECT

  friend class Smb4KPrintPrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KPrint *self();

    /**
     * This function starts the printing of a file.
     *
     * @param printInfo   The Smb4KPrintInfo object
     */
    void print( Smb4KPrintInfo *printInfo );

    /**
     * Aborts the print process that is represented by @p info.
     *
     * @param printInfo   The Smb4KPrintInfo object
     */
    void abort( Smb4KPrintInfo *printInfo );

    /**
     * Aborts all print processes at once.
     */
    void abortAll();

    /**
     * This function reports if a certain process that is represented by @p info
     * is running.
     *
     * @param printInfo   The Smb4KPrintInfo object
     *
     * @returns TRUE if the process is running.
     */
    bool isRunning( Smb4KPrintInfo *printInfo );

    /**
     * This function returns TRUE if the printer handler is running and
     * FALSE otherwise.
     *
     * @returns           TRUE is the printer handler is running and FALSE otherwise.
     */
    bool isRunning() { return !m_cache.isEmpty(); }

    /**
     * This function returns the current state of the print interface. The state
     * is defined in the smb4kdefs.h file.
     *
     * @returns the current state of the print interface.
     */
    int currentState() { return m_state; }

  signals:
    /**
     * This signal is emitted when the current run state changed. Use the currentState()
     * function to read the current run state.
     */
    void stateChanged();

    /**
     * This signal is emitted when a print process represented by @p info is about to
     * be started.
     *
     * @param printInfo   The Smb4KPrintInfo object
     */
    void aboutToStart( Smb4KPrintInfo *printInfo );

    /**
     * This signal is emitted when the print process represented by @p info finished.
     *
     * @param printInfo   The Smb4KPrintInfo object
     */
    void finished( Smb4KPrintInfo *printInfo );

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
     * The constructor.
     */
    Smb4KPrint();

    /**
     * The destructor.
     */
    ~Smb4KPrint();

    /**
     * The cache that holds the threads.
     */
    QCache<QString, PrintThread> m_cache;

    /**
     * The state
     */
    int m_state;
};

#endif
