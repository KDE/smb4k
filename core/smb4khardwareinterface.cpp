/***************************************************************************
    Provides an interface to the computer's hardware
                             -------------------
    begin                : Die Jul 14 2015
    copyright            : (C) 2015-2020 by Alexander Reinholdt
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

#include <unistd.h>

// Qt includes
#include <QDebug>
#include <QTest>
#include <QStringLiteral>
#include <QDBusReply>

// KDE includes
#include <Solid/DeviceNotifier>
#include <Solid/Device>
#include <Solid/NetworkShare>
#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include <KIOCore/KMountPoint>
#endif

Q_GLOBAL_STATIC(Smb4KHardwareInterfaceStatic, p);


Smb4KHardwareInterface::Smb4KHardwareInterface(QObject *parent)
: QObject(parent), d(new Smb4KHardwareInterfacePrivate)
{
  d->networkSession = nullptr;
  d->fileDescriptor.setFileDescriptor(-1);
  
  d->dbusInterface.reset(new QDBusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus(), this));
  
  if (!d->dbusInterface->isValid())
  {
    d->dbusInterface.reset(new QDBusInterface("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager", "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus(), this));
  }
  
  connect(&d->networkConfigManager, SIGNAL(updateCompleted()), this, SLOT(slotNetworkConfigUpdated()));
  connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
  connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  startTimer(2000);
#endif
}


Smb4KHardwareInterface::~Smb4KHardwareInterface()
{
}


Smb4KHardwareInterface* Smb4KHardwareInterface::self()
{
  return &p->instance;
}


void Smb4KHardwareInterface::updateNetworkConfig()
{
  d->networkConfigManager.updateConfigurations();
}


bool Smb4KHardwareInterface::isOnline() const
{
  if (d->networkSession)
  {
    return (d->networkSession->state() == QNetworkSession::Connected);
  }
  
  return false;
}


void Smb4KHardwareInterface::inhibit()
{
  if (d->fileDescriptor.isValid())
  {
    return;
  }
  
  if (d->dbusInterface->isValid())
  {
    QVariantList args;
    args << QStringLiteral("shutdown:sleep");
    args << QStringLiteral("Smb4K");
    args << QStringLiteral("Mounting or unmounting in progress");
    args << QStringLiteral("delay");
    
    QDBusReply<QDBusUnixFileDescriptor> descriptor = d->dbusInterface->callWithArgumentList(QDBus::Block, QStringLiteral("Inhibit"), args);
    
    if (descriptor.isValid())
    {
      d->fileDescriptor = descriptor.value();
    }
  }
}


void Smb4KHardwareInterface::uninhibit()
{
  if (!d->fileDescriptor.isValid())
  {
    return;
  }
  
  if (d->dbusInterface->isValid())
  {
    close(d->fileDescriptor.fileDescriptor());
    d->fileDescriptor.setFileDescriptor(-1);
  }
}


#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
// Using FreeBSD 11 with KF 5.27, Solid is not able to detect mounted shares.
// Thus, we check here whether shares have been mounted or unmounted.
// This is a hack and should be removed as soon as possible.
void Smb4KHardwareInterface::timerEvent(QTimerEvent */*e*/)
{
  KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::BasicInfoNeeded|KMountPoint::NeedMountOptions);
  QStringList mountPointList, alreadyMounted;
  
  for (const QExplicitlySharedDataPointer<KMountPoint> &mountPoint : mountPoints)
  {
    if (QString::compare(mountPoint->mountType(), "smbfs") == 0 || QString::compare(mountPoint->mountType(), "cifs") == 0)
    {
      mountPointList.append(mountPoint->mountPoint());
    }
  }
  
  QMutableStringListIterator it(mountPointList);
  
  while (it.hasNext())
  {
    QString mp = it.next();
    int index = -1;
    
    if ((index = d->mountPoints.indexOf(mp)) != -1)
    {
      d->mountPoints.removeAt(index);
      alreadyMounted.append(mp);
      it.remove();
    }
  }
  
  if (!d->mountPoints.isEmpty())
  {
    emit networkShareRemoved();
  }
  
  if (!mountPointList.isEmpty())
  {
    emit networkShareAdded();
  }
  
  d->mountPoints.clear();
  d->mountPoints.append(alreadyMounted);
  d->mountPoints.append(mountPointList);
}
#endif


void Smb4KHardwareInterface::slotNetworkConfigUpdated()
{
  //
  // Create a network session object if necessary and connect it to the stateChanged()
  // signal to monitor changes of the network connection.
  // 
  if (!d->networkSession)
  {
    d->networkSession = new QNetworkSession(d->networkConfigManager.defaultConfiguration(), this);
    connect(d->networkSession, SIGNAL(stateChanged(QNetworkSession::State)), this, SLOT(slotConnectionStateChanged(QNetworkSession::State)));
  }

  //
  // Tell the application that the network configuration was updated
  // 
  emit networkConfigUpdated();
  
  //
  // Check the state of the network session and emit the onlineStateChanged()
  // signal accordingly.
  // 
  if (d->networkSession->state() == QNetworkSession::Connected)
  {
    emit onlineStateChanged(true);
  }
  else
  {
    emit onlineStateChanged(false);
  }
}


void Smb4KHardwareInterface::slotConnectionStateChanged(QNetworkSession::State state)
{
  if (state == QNetworkSession::Connected)
  {
    emit onlineStateChanged(true);
  }
  else
  {
    emit onlineStateChanged(false);
  }
}



void Smb4KHardwareInterface::slotDeviceAdded(const QString& udi)
{
  Solid::Device device(udi);
  
  if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare))
  {
    d->udis.append(udi);
    emit networkShareAdded();
  }
}


void Smb4KHardwareInterface::slotDeviceRemoved(const QString& udi)
{
  Solid::Device device(udi);
  
  // For some reason, the device has no valid type at the moment (Frameworks 5.9, 
  // July 2015). Thus, we need the code in the else block for now. 
  if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare))
  {
    emit networkShareRemoved();
  }
  else
  {
    if (d->udis.contains(udi))
    {
      emit networkShareRemoved();
      d->udis.removeOne(udi);
    }
  }
}


