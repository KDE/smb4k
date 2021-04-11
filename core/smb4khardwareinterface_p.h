/***************************************************************************
    Private helper class(es) for the hardware interface
                             -------------------
    begin                : Die Jul 14 2015
    copyright            : (C) 2015-2021 by Alexander Reinholdt
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

#ifndef SMB4KHARDWAREINTERFACE_P_H
#define SMB4KHARDWAREINTERFACE_P_H

// application specific includes
#include "smb4khardwareinterface.h"

// Qt includes
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QStringList>

class Smb4KHardwareInterfacePrivate
{
public:
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QStringList mountPoints;
#endif
    QScopedPointer<QDBusInterface> dbusInterface;
    QDBusUnixFileDescriptor fileDescriptor;
    bool systemOnline;
    QStringList udis;
};

class Smb4KHardwareInterfaceStatic
{
public:
    Smb4KHardwareInterface instance;
};

#endif
