/*
    This class provides helper classes for Smb4KDeclarative
    -------------------
    begin                : Mo 02 Sep 2013
    SPDX-FileCopyrightText: 2013-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
#include "smb4kbookmarkobject.h"
#include "smb4knetworkobject.h"
#include "smb4kprofileobject.h"

class Smb4KDeclarativePrivate
{
public:
    QList<Smb4KNetworkObject *> workgroupObjects;
    QList<Smb4KNetworkObject *> hostObjects;
    QList<Smb4KNetworkObject *> shareObjects;
    QList<Smb4KNetworkObject *> mountedObjects;
    QList<Smb4KBookmarkObject *> bookmarkObjects;
    QList<Smb4KBookmarkObject *> bookmarkCategoryObjects;
    QList<Smb4KProfileObject *> profileObjects;
};

#endif
