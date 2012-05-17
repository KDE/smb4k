/***************************************************************************
    smb4kprocess  -  This class executes shell processes.
                             -------------------
    begin                : Mi Mär 4 2009
    copyright            : (C) 2009-2012 by Alexander Reinholdt
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

#ifndef SMB4KPROCESS_H
#define SMB4KPROCESS_H

// Qt includes
#include <QtCore/QScopedPointer>

// KDE includes
#include <kprocess.h>

// forward declarations
class Smb4KProcessPrivate;

/**
 * This class provides a version of KProcess adjusted to the needs of Smb4K.
 * It executes shell processes.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KProcess : public KProcess
{
  Q_OBJECT

  friend class Smb4KProcessPrivate;

  public:
    /**
     * The constructor
     *
     * @param parent        The parent object
     */
    Smb4KProcess( QObject *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KProcess();

    /**
     * Abort the process. This function sends SIGKILL to the process
     * and waits until it exited.
     */
    void abort();

    /**
     * This function returns TRUE if the process was aborted and FALSE
     * otherwise.
     *
     * @returns TRUE if the process was aborted.
     */
    bool isAborted() const;

  private:
    const QScopedPointer<Smb4KProcessPrivate> d;
};


#endif
