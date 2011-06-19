/***************************************************************************
    smb4kprocess  -  This class executes shell processes.
                             -------------------
    begin                : Mi Mär 4 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KPROCESS_H
#define SMB4KPROCESS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// KDE includes
#include <kprocess.h>

/**
 * This class provides a version of KProcess adjusted to the needs of Smb4K.
 * It executes shell processes.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KProcess : public KProcess
{
  Q_OBJECT

  public:
    /**
     * The type enumeration
     */
    enum Type { LookupIP,
                Mount,
                Unmount,
                Preview,
                Print,
                Synchronize,
                LookupDomains,
                LookupDomainMembers,
                LookupShares,
                LookupInfo,
                Search,
                Kill,
                WriteSudoers,
                Unknown };

    /**
     * The constructor
     *
     * @param parent        The parent object
     */
    Smb4KProcess( Type type,
                  QObject *parent = 0 );

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
    bool isAborted() { return m_aborted; }

    /**
     * This function returns the type of the process.
     *
     * @returns the type of the process.
     */
    Type type() const { return m_type; }

  private:
    Type m_type;
    bool m_aborted;
};


#endif
