/***************************************************************************
    smb4ksolidinterface_p  -  This class includes private helper classes
    for the Smb4KSolidInterface class
                             -------------------
    begin                : So Jun 30 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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

#ifndef SMB4KSOLIDINTERFACE_P_H
#define SMB4KSOLIDINTERFACE_P_H

class Smb4KSolidInterfacePrivate
{
  public:
    Smb4KSolidInterface::ButtonType buttonPressed;
    Smb4KSolidInterface::ConnectionStatus networkStatus;
    int sleepCookie;
};


class Smb4KSolidInterfaceStatic
{
  public:
    Smb4KSolidInterface instance;
};

#endif
