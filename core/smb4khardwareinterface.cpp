/***************************************************************************
    Provides an interface to the computer's hardware
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

// application specific includes
#include "smb4khardwareinterface.h"
#include "smb4khardwareinterface_p.h"

// system includes
#include <unistd.h>

// Qt includes
#include <QDBusReply>
#include <QDebug>
#include <QNetworkInterface>
#include <QString>
#include <QTest>
#include <QTimer>

// KDE includes
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/NetworkShare>
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <KIOCore/KMountPoint>
#endif

Q_GLOBAL_STATIC(Smb4KHardwareInterfaceStatic, p);

Smb4KHardwareInterface::Smb4KHardwareInterface(QObject *parent)
    : QObject(parent)
    , d(new Smb4KHardwareInterfacePrivate)
{
    //
    // Initialize some members
    //
    d->systemOnline = false;
    d->fileDescriptor.setFileDescriptor(-1);

    //
    // Set up the DBUS interface
    //
    d->dbusInterface.reset(
        new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus(), this));

    if (!d->dbusInterface->isValid()) {
        d->dbusInterface.reset(new QDBusInterface("org.freedesktop.ConsoleKit",
                                                  "/org/freedesktop/ConsoleKit/Manager",
                                                  "org.freedesktop.ConsoleKit.Manager",
                                                  QDBusConnection::systemBus(),
                                                  this));
    }

    //
    // Connections
    //
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));

    //
    // Check the online state
    //
    checkOnlineState(false);

    //
    // Start the timer to continously check the online state
    // and, under FreeBSD, additionally the mounted shares.
    //
    startTimer(1000);
}

Smb4KHardwareInterface::~Smb4KHardwareInterface()
{
}

Smb4KHardwareInterface *Smb4KHardwareInterface::self()
{
    return &p->instance;
}

bool Smb4KHardwareInterface::isOnline() const
{
    return d->systemOnline;
}

void Smb4KHardwareInterface::inhibit()
{
    if (d->fileDescriptor.isValid()) {
        return;
    }

    if (d->dbusInterface->isValid()) {
        QVariantList args;
        args << QStringLiteral("shutdown:sleep");
        args << QStringLiteral("Smb4K");
        args << QStringLiteral("Mounting or unmounting in progress");
        args << QStringLiteral("delay");

        QDBusReply<QDBusUnixFileDescriptor> descriptor = d->dbusInterface->callWithArgumentList(QDBus::Block, QStringLiteral("Inhibit"), args);

        if (descriptor.isValid()) {
            d->fileDescriptor = descriptor.value();
        }
    }
}

void Smb4KHardwareInterface::uninhibit()
{
    if (!d->fileDescriptor.isValid()) {
        return;
    }

    if (d->dbusInterface->isValid()) {
        close(d->fileDescriptor.fileDescriptor());
        d->fileDescriptor.setFileDescriptor(-1);
    }
}

void Smb4KHardwareInterface::checkOnlineState(bool emitSignal)
{
    bool online = false;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &interface : qAsConst(interfaces)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning)
#else
        if (interface.isValid() && !(interface.flags() & QNetworkInterface::IsLoopBack) && interface.flags() & QNetworkInterface::IsRunning)
#endif
        {
            online = true;
            break;
        }
    }

    if (online != d->systemOnline) {
        d->systemOnline = online;

        if (emitSignal) {
            emit onlineStateChanged(d->systemOnline);
        }
    }
}

void Smb4KHardwareInterface::timerEvent(QTimerEvent * /*e*/)
{
    //
    // Check for network connectivity
    //
    checkOnlineState();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    //
    // Using FreeBSD 11 with KF 5.27, Solid is not able to detect mounted shares.
    // Thus, we check here whether shares have been mounted or unmounted.
    // This is a hack and should be removed as soon as possible.
    //
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);
    QStringList mountPointList, alreadyMounted;

    for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints) {
        if (QString::compare(mountPoint->mountType(), "smbfs") == 0 || QString::compare(mountPoint->mountType(), "cifs") == 0) {
            mountPointList.append(mountPoint->mountPoint());
        }
    }

    QMutableStringListIterator it(mountPointList);

    while (it.hasNext()) {
        QString mp = it.next();
        int index = -1;

        if ((index = d->mountPoints.indexOf(mp)) != -1) {
            d->mountPoints.removeAt(index);
            alreadyMounted.append(mp);
            it.remove();
        }
    }

    if (!d->mountPoints.isEmpty()) {
        emit networkShareRemoved();
    }

    if (!mountPointList.isEmpty()) {
        emit networkShareAdded();
    }

    d->mountPoints.clear();
    d->mountPoints.append(alreadyMounted);
    d->mountPoints.append(mountPointList);
#endif
}

void Smb4KHardwareInterface::slotDeviceAdded(const QString &udi)
{
    //
    // Create a device from the UDI
    //
    Solid::Device device(udi);

    //
    // Check if the device is a network device and emit the networkShareAdded()
    // signal if it is.
    //
    if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        d->udis << udi;
        emit networkShareAdded();
    }
}

void Smb4KHardwareInterface::slotDeviceRemoved(const QString &udi)
{
    //
    // Create a device from the UDI
    //
    Solid::Device device(udi);

    //
    // Check if the device is a network device and emit the networkShareRemoved()
    // signal if it is.
    //
    // NOTE:For some reason, the device has no valid type. Thus, we need the code
    // in the else block for now.
    //
    if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare)) {
        emit networkShareRemoved();
    } else {
        if (d->udis.contains(udi)) {
            emit networkShareRemoved();
            d->udis.removeOne(udi);
        }
    }
}
