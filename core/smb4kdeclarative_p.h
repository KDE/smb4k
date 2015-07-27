/***************************************************************************
    smb4kdeclarative - This class provides helper classes for 
    Smb4KDeclarative
                             -------------------
    begin                : Mo 02 Sep 2013
    copyright            : (C) 2013-2015 by Alexander Reinholdt
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

#ifndef SMB4KDECLARATIVE_P_H
#define SMB4KDECLARATIVE_P_H

// application specific includes
#include "smb4kscanner.h"
#include "smb4kmounter.h"
#include "smb4knetworkobject.h"
#include "smb4kbookmarkobject.h"
#include "smb4kprofileobject.h"

class Smb4KDeclarativePrivate
{
  public:
    QList<Smb4KNetworkObject *> workgroupObjects;
    QList<Smb4KNetworkObject *> hostObjects;
    QList<Smb4KNetworkObject *> shareObjects;
    QList<Smb4KNetworkObject *> mountedObjects;
    QList<Smb4KBookmarkObject *> bookmarkObjects;
    QList<Smb4KBookmarkObject *> bookmarkGroupObjects;
    QList<Smb4KProfileObject *> profileObjects;
};

#endif
