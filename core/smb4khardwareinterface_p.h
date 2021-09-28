/*
    Private helper class(es) for the hardware interface

    SPDX-FileCopyrightText: 2015-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
