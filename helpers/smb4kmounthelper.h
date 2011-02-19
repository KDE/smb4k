/***************************************************************************
    smb4kmounthelper  -  The helper that mounts and unmounts shares.
                             -------------------
    begin                : Sa Okt 16 2010
    copyright            : (C) 2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KMOUNTHELPER_H
#define SMB4KMOUNTHELPER_H

// Qt includes
#include <QObject>

// KDE includes
#include <kauth.h>

using namespace KAuth;

class Smb4KMountHelper : public QObject
{
  Q_OBJECT
  
  public slots:
    /**
     * Mounts a CIFS/SMBFS share.
     * 
     * The following arguments are recognized:
     * mount_command      The full mount command (mandatory)
     * share_url          The URL of the share (mandatory)
     * share_mountpoint   The mountpoint of the share (mandatory)
     * share_workgroup    The workgroup of the share (optional)
     * share_comment      The comment of the share (optional)
     * key                The key of this action (optional, mandatory for Smb4K)
     */
    ActionReply mount( const QVariantMap &args );

    /**
     * Unmounts a CIFS/SMBFS share.
     *
     * The following arguments are recognized:
     * unmount_command    The full unmount command (mandatory)
     * share_url          The URL of the share (optional, mandatory for Smb4K)
     * share_mountpoint   The mountpoint of the share (mandatory)
     * key                The key of this action (optional, mandatory for Smb4K)
     */
    ActionReply unmount( const QVariantMap &args );
};

#endif
