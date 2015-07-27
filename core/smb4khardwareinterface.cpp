/***************************************************************************
    smb4khardwareinterface  -  Provides an interface to the computer's 
    hardware
                             -------------------
    begin                : Die Jul 14 2015
    copyright            : (C) 2015 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4khardwareinterface.h"
#include "smb4khardwareinterface_p.h"

// Qt includes
#include <QtCore/QDebug>

// KDE includes
#include <Solid/DeviceNotifier>
#include <Solid/Device>
#include <Solid/NetworkShare>
#include <KDELibs4Support/solid/powermanagement.h>

Q_GLOBAL_STATIC(Smb4KHardwareInterfaceStatic, p);


Smb4KHardwareInterface::Smb4KHardwareInterface(QObject *parent)
: QObject(parent), d(new Smb4KHardwareInterfacePrivate)
{
  d->networkConfigUpdated = false;
  
  connect(&d->networkConfigManager, SIGNAL(updateCompleted()), this, SLOT(slotNetworkConfigUpdated()));
  connect(&d->networkConfigManager, SIGNAL(onlineStateChanged(bool)), this, SIGNAL(onlineStateChanged(bool)));
  connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)), this, SLOT(slotDeviceAdded(QString)));
  connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)), this, SLOT(slotDeviceRemoved(QString)));
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


bool Smb4KHardwareInterface::networkConfigIsUpdated() const
{
  return d->networkConfigUpdated;
}


bool Smb4KHardwareInterface::isOnline() const
{
  return d->networkConfigManager.isOnline();
}


int Smb4KHardwareInterface::beginSuppressingSleep(const QString& reason)
{
  return Solid::PowerManagement::beginSuppressingSleep(reason);
}


void Smb4KHardwareInterface::endSuppressingSleep(int cookie)
{
  Solid::PowerManagement::stopSuppressingSleep(cookie);
}


void Smb4KHardwareInterface::slotNetworkConfigUpdated()
{
  d->networkConfigUpdated = true;
  emit networkConfigUpdated();
}


void Smb4KHardwareInterface::slotDeviceAdded(const QString& udi)
{
  Solid::Device device(udi);
  
  if (device.isDeviceInterface(Solid::DeviceInterface::NetworkShare))
  {
    d->udis.append(udi);
    emit networkShareAdded();
  }
  else
  {
    // Do nothing
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
    else
    {
      // Do nothing
    }
  }
}



