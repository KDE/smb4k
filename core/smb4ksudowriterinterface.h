/***************************************************************************
    smb4ksudowriterinterface  -  This class provides an interface to the
    smb4k_sudowriter utility program that writes entries to the sudoers
    file.
                             -------------------
    begin                : Sa Aug 2 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

#ifndef SMB4KSUDOWRITERINTERFACE_H
#define SMB4KSUDOWRITERINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>

// KDE includes
#include <kdemacros.h>

/**
 * This class provides an interface to the smb4k_sudowriter utility program
 * that writes to the sudoers file.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KSudoWriterInterface : public QObject
{
  Q_OBJECT

  friend class Smb4KSudoWriterInterfacePrivate;

  public:
    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KSudoWriterInterface class.
     */
    static Smb4KSudoWriterInterface *self();

    /**
     * This function adds the name of the current user to the Smb4K section
     * in the sudoers file by invoking the smb4k_sudowriter utility program.
     * If no Smb4K section exists, the utility program will create one.
     *
     * @returns TRUE on success.
     */
    bool addUser();

    /**
     * This function removes the name of the current user from the Smb4K
     * section in the sudoers file by invoking the smb4k_sudowriter utility
     * program. If the user is the only entry, the Smb4K section will be
     * removed from the sudoers file.
     *
     * @returns TRUE on success.
     */
    bool removeUser();

  private:
    /**
     * The constructor.
     */
    Smb4KSudoWriterInterface();

    /**
     * The destructor.
     */
    ~Smb4KSudoWriterInterface();
};

#endif
