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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/


#ifndef SMB4KDEFS_H
#define SMB4KDEFS_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//
// Run states
//

// (1) Scanner
#define SCANNER_LOOKUP_DOMAINS                0
#define SCANNER_QUERY_MASTER_BROWSER          1
#define SCANNER_SCAN_BROADCAST_AREAS          2
#define SCANNER_OPEN_WORKGROUP                3
#define SCANNER_OPEN_HOST                     4
#define SCANNER_QUERY_INFO                    5
#define SCANNER_STOP                          6

// (2) Mounter
#define MOUNTER_MOUNT                         7
#define MOUNTER_UNMOUNT                       8
#define MOUNTER_STOP                          9

// (3) Printing
#define PRINT_START                          10
#define PRINT_STOP                           11

// (4) Synchronizer
#define SYNCHRONIZER_START                   12
#define SYNCHRONIZER_STOP                    13

// (5) Previewer
#define PREVIEWER_START                      14
#define PREVIEWER_STOP                       15

// (6) Search
#define SEARCH_START                         16
#define SEARCH_STOP                          17

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
