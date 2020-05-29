/***************************************************************************
    The helper that mounts and unmounts shares.
                             -------------------
    begin                : Sa Okt 16 2010
    copyright            : (C) 2010-2020 by Alexander Reinholdt
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

#ifndef SMB4KMOUNTHELPER_H
#define SMB4KMOUNTHELPER_H

// Qt includes
#include <QObject>

// KDE includes
#include <KAuth/KAuthActionReply>

using namespace KAuth;

class Smb4KMountHelper : public QObject
{
  Q_OBJECT

  public slots:
    /**
     * Mounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply mount(const QVariantMap &args);

    /**
     * Unmounts a CIFS/SMBFS share.
     */
    KAuth::ActionReply unmount(const QVariantMap &args);
};

#endif
