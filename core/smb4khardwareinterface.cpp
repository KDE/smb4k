/*
    Provides an interface to the computer's hardware

    SPDX-FileCopyrightText: 2015-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4khardwareinterface.h"

// system includes
#include <unistd.h>

// Qt includes
#if (QT_VERSION >= QT_VERSION_CHECK(6,8,0))
#include <QApplicationStatic>
#else
#include <qapplicationstatic.h>
#endif
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QNetworkInterface>
#include <QString>
#include <QStringList>
#include <QTimer>

// KDE includes
#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/NetworkShare>
#include <solid_version.h>
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <KMountPoint>
#endif

class Smb4KHardwareInterfacePrivate
{
public:
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    QStringList mountPoints;
#endif
    QScopedPointer<QDBusInterface> dbusInterface;
    QDBusUnixFileDescriptor fileDescriptor;
    bool systemOnline;
    bool systemSleep;
    QStringList udis;
    int timerId;
};

class Smb4KHardwareInterfaceStatic
{
public:
    Smb4KHardwareInterface instance;
};

Q_APPLICATION_STATIC(Smb4KHardwareInterfaceStatic, p);

Smb4KHardwareInterface::Smb4KHardwareInterface(QObject *parent)
    : QObject(parent)
    , d(new Smb4KHardwareInterfacePrivate)
{
    d->systemOnline = false;
    d->systemSleep = false;
    d->fileDescriptor.setFileDescriptor(-1);
    d->timerId = -1;

    //
    // Set up the DBUS interface
    //
    d->dbusInterface.reset(new QDBusInterface(QStringLiteral("org.freedesktop.login1"),
                                              QStringLiteral("/org/freedesktop/login1"),
                                              QStringLiteral("org.freedesktop.login1.Manager"),
                                              QDBusConnection::systemBus(),
                                              this));

    if (!d->dbusInterface->isValid()) {
        d->dbusInterface.reset(new QDBusInterface(QStringLiteral("org.freedesktop.ConsoleKit"),
                                                  QStringLiteral("/org/freedesktop/ConsoleKit/Manager"),
                                                  QStringLiteral("org.freedesktop.ConsoleKit.Manager"),
                                                  QDBusConnection::systemBus(),
                                                  this));
    }

    QDBusConnection::systemBus()
        .connect(QString(), QString(), QStringLiteral("org.freedesktop.login1.Manager"), QStringLiteral("PrepareForSleep"), this, SLOT(slotSystemSleep(bool)));

    //
    // Get the initial list of udis for network shares
    //
    QList<Solid::Device> allDevices = Solid::Device::allDevices();

    for (const Solid::Device &device : std::as_const(allDevices)) {
        const Solid::DeviceInterface *iface = device.asDeviceInterface(Solid::DeviceInterface::NetworkShare);
        const Solid::NetworkShare *networkShare = qobject_cast<const Solid::NetworkShare *>(iface);

        if (networkShare && (networkShare->type() == Solid::NetworkShare::Cifs || networkShare->type() == Solid::NetworkShare::Smb3)) {
            d->udis << device.udi();
        }
    }

    //
    // Connections
    //
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &Smb4KHardwareInterface::slotDeviceAdded);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &Smb4KHardwareInterface::slotDeviceRemoved);

    //
    // Check the online state
    //
    checkOnlineState(false);

    //
    // Start the timer to continously check the online state
    // and, under FreeBSD, additionally the mounted shares.
    //
    d->timerId = startTimer(1000);
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

    for (const QNetworkInterface &interface : std::as_const(interfaces)) {
        if (interface.isValid() && interface.type() != QNetworkInterface::Loopback && interface.flags() & QNetworkInterface::IsRunning) {
            online = true;
            break;
        }
    }

    if (online != d->systemOnline) {
        d->systemOnline = online;

        if (emitSignal) {
            Q_EMIT onlineStateChanged(d->systemOnline);
        }
    }
}

void Smb4KHardwareInterface::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    checkOnlineState();

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
    // NOTE: Using FreeBSD 11 with KF 5.27, Solid is not able to detect mounted shares.
    // Thus, we check here whether shares have been mounted or unmounted.
    // This is a hack and should be removed as soon as possible.
    KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded | KMountPoint::NeedMountOptions);
    QStringList mountPointList, alreadyMounted;

    for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints) {
        if (mountPoint->mountType() == QStringLiteral("smbfs")) {
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
        Q_EMIT networkShareRemoved();
    }

    if (!mountPointList.isEmpty()) {
        Q_EMIT networkShareAdded();
    }

    d->mountPoints.clear();
    d->mountPoints.append(alreadyMounted);
    d->mountPoints.append(mountPointList);
#endif
}

void Smb4KHardwareInterface::slotDeviceAdded(const QString &udi)
{
    Solid::Device device(udi);

    const Solid::DeviceInterface *iface = device.asDeviceInterface(Solid::DeviceInterface::NetworkShare);
    const Solid::NetworkShare *networkShare = qobject_cast<const Solid::NetworkShare *>(iface);

    if (networkShare && (networkShare->type() == Solid::NetworkShare::Cifs || networkShare->type() == Solid::NetworkShare::Smb3)) {
        d->udis << udi;
        Q_EMIT networkShareAdded();
    }
}

void Smb4KHardwareInterface::slotDeviceRemoved(const QString &udi)
{
    if (d->udis.contains(udi)) {
        Q_EMIT networkShareRemoved();
        d->udis.removeOne(udi);
    }
}

void Smb4KHardwareInterface::slotSystemSleep(bool sleep)
{
    inhibit();
    d->systemSleep = sleep;

    if (d->systemSleep) {
        killTimer(d->timerId);
        d->timerId = -1;
        // The system will recover after a shutdown completely, so we
        // do not have the trigger any unmounts by emitting a signal
        // here. However, we will awake from a sleep later, so some things
        // should be triggered by the emission of the onlineStateChanged
        // signal.
        d->systemOnline = false;
    } else {
        d->timerId = startTimer(1000);
    }

    uninhibit();
}
