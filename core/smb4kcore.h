/***************************************************************************
    smb4kcore  -  The main core class of Smb4K.
                             -------------------
    begin                : Do Apr 8 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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

#ifndef SMB4KCORE_H
#define SMB4KCORE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>

// KDE includes
#include <kdemacros.h>

class Smb4KShare;
class Smb4KCorePrivate;


/**
 * This is the top-level core class. It inherits most of the core classes.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KCore : public QObject
{
  Q_OBJECT

  friend class Smb4KCorePrivate;

  public:
    /**
     * Returns a static pointer to this class.
     */
    static Smb4KCore *self();

    /**
     * Returns TRUE if one of the core classes is doing something.
     */
    static bool isRunning();

    /**
     * Aborts any action of the core.
     */
    static void abort();

    /**
     * This function initializes the core classes.
     */
    void init();

  protected slots:
    /**
     * This slot is connected to the QApplication::aboutToQuit() signal. It is invoked
     * when the application is shut down by the KDE logout or by pressing CTRL+Q, etc.
     */
    void slotAboutToQuit();

  private:
    /**
     * The constructor.
     */
    Smb4KCore();

    /**
     * The destructor.
     */
    ~Smb4KCore();

    /**
     * Searches for the needed programs and emits an error
     * if mandatory ones are missing.
     */
    void searchPrograms();

    /**
     * Set default values for settings that depend on the system Smb4K is
     * running on and that have to be set dynamically.
     */
    void setDefaultSettings();
};

#endif
