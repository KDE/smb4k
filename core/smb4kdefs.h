/***************************************************************************
    smb4kdefs.h  -  Definitions for Smb4K
                             -------------------
    begin                : Mo Mär 15 2004
    copyright            : (C) 2004 by Alexander Reinholdt
    email                : dustpuppy@mail.berlios.de
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


#ifndef SMB4KDEFS_H
#define SMB4KDEFS_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//
// Run states
//

// (7) Core
#define CORE_STOP                            18

//
// Event types
//

#define EVENT_LOAD_SETTINGS                1001
#define EVENT_SET_FOCUS                    1002
#define EVENT_SCAN_NETWORK                 1003
#define EVENT_ADD_BOOKMARK                 1004

//
// Other definitions
//

#define TIMER_INTERVAL                       25

#endif
