/***************************************************************************
    smb4ksolidinterface  -  This class provides an interface to KDE's
    Solid framework.
                             -------------------
    begin                : So Sep 14 2008
    copyright            : (C) 2008 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

// KDE includes
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/acadapter.h>
#include <solid/battery.h>
#include <solid/powermanagement.h>

// application specific includes
#include <smb4ksolidinterface.h>

class Smb4KSolidInterfacePrivate
{
  public:
    Smb4KSolidInterface instance;
};

K_GLOBAL_STATIC( Smb4KSolidInterfacePrivate, priv );


Smb4KSolidInterface::Smb4KSolidInterface()
: QObject(),
  m_button_pressed( UnknownButton ),
  m_network_status( Unknown )
{
  init();
}


Smb4KSolidInterface::~Smb4KSolidInterface()
{
}


Smb4KSolidInterface *Smb4KSolidInterface::self()
{
  return &priv->instance;
}


void Smb4KSolidInterface::init()
{
  // Connect to device notifier.
  connect( Solid::DeviceNotifier::instance(), SIGNAL( deviceAdded( const QString & ) ),
           this,                              SLOT( slotDeviceAdded( const QString & ) ) );

  connect( Solid::DeviceNotifier::instance(), SIGNAL( deviceRemoved( const QString & ) ),
           this,                              SLOT( slotDeviceRemoved( const QString & ) ) );

  // Get the buttons
  QList<Solid::Device> list_btn = Solid::Device::listFromType( Solid::DeviceInterface::Button, QString() );

  foreach ( Solid::Device device_btn, list_btn )
  {
    if ( device_btn.isValid() )
    {
      Solid::Button *button = device_btn.as<Solid::Button>();
      connect( button, SIGNAL( pressed( Solid::Button::ButtonType, const QString & ) ),
               this,   SLOT( slotButtonPressed( Solid::Button::ButtonType, const QString & ) ) );

      continue;
    }
    else
    {
      continue;
    }
  }

  // FIXME: Check for hibernation, etc. as well as for the system waking
  // up again.

  // Get the AC adapter(s)
  QList<Solid::Device> list_ac = Solid::Device::listFromType( Solid::DeviceInterface::AcAdapter, QString() );

  foreach ( Solid::Device device_ac, list_ac )
  {
    if ( device_ac.isValid() )
    {
      Solid::AcAdapter *acadapter = device_ac.as<Solid::AcAdapter>();
      connect( acadapter, SIGNAL( plugStateChanged( bool, const QString & ) ),
               this,     SLOT( slotAcPlugStateChanged( bool, const QString & ) ) );

      // FIXME: Check the current state.

      continue;
    }
    else
    {
      continue;
    }
  }

  // Get the primary batteries
  QList<Solid::Device> list_bat = Solid::Device::listFromType( Solid::DeviceInterface::Battery, QString() );

  foreach ( Solid::Device device_bat, list_bat )
  {
    if ( device_bat.isValid() )
    {
      Solid::Battery *battery = device_bat.as<Solid::Battery>();

      switch ( battery->type() )
      {
        case Solid::Battery::PrimaryBattery:
        {
          connect( battery, SIGNAL( chargeStateChanged( int, const QString & ) ),
                   this,    SLOT( slotBatteryChargeStateChanged( int, const QString & ) ) );

          connect( battery, SIGNAL( chargePercentChanged( int, const QString & ) ),
                   this,    SLOT( slotBatteryChargePercentChanged( int, const QString & ) ) );

          // FIXME: Check charge state and value

          break;
        }
        default:
        {
          break;
        }
      }

      continue;
    }
    else
    {
      continue;
    }
  }

  // Check network status and connect to the notifier
  slotNetworkStatusChanged( Solid::Networking::status() );

  connect( Solid::Networking::notifier(), SIGNAL( statusChanged( Solid::Networking::Status ) ),
           this,                          SLOT( slotNetworkStatusChanged( Solid::Networking::Status ) ) );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSolidInterface::slotDeviceAdded( const QString &udi )
{
  // Work around the fact that there is no signal that tells
  // us when the computer woke up. Check the UDIs in the map
  // and emit the wokeUp()
  if ( m_button_pressed != UnknownButton &&
       m_removed_devices[udi] == m_button_pressed )
  {
    m_removed_devices.remove( udi );

    if ( m_removed_devices.isEmpty() )
    {
      // Work around: Computer woke up.
      m_button_pressed = UnknownButton;
      emit wokeUp();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSolidInterface::slotDeviceRemoved( const QString &udi )
{
  // Work around the fact that there is no signal that tells us,
  // that the computer has been woken up. Store the removed device(s)
  // and the pressed button to figure out later (in slotDeviceAdded())
  // if the computer became active again.
  if ( m_button_pressed != UnknownButton )
  {
    m_removed_devices.insert( udi, m_button_pressed );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSolidInterface::slotButtonPressed( Solid::Button::ButtonType type, const QString &udi )
{
  switch ( type )
  {
    case Solid::Button::LidButton:
    {
      m_button_pressed = LidButton;
      break;
    }
    case Solid::Button::SleepButton:
    {
      m_button_pressed = SleepButton;
      break;
    }
    case Solid::Button::PowerButton:
    {
      m_button_pressed = PowerButton;
      break;
    }
    default:
    {
      m_button_pressed = UnknownButton;
      break;
    }
  }

  emit buttonPressed( m_button_pressed );
}


void Smb4KSolidInterface::slotAcPlugStateChanged( bool state, const QString &udi )
{
  if ( state )
  {
    kDebug() << "AC adapter plugged ..." << endl;
  }
  else
  {
    kDebug() << "AC adapter unplugged ..." << endl;
  }
}


void Smb4KSolidInterface::slotBatteryChargeStateChanged( int state, const QString &udi )
{
  switch ( state )
  {
    case Solid::Battery::Discharging:
    {
      kDebug() << "Battery is discharging ..." << endl;
      break;
    }
    case Solid::Battery::Charging:
    {
      kDebug() << "Battery is charging ..." << endl;
      break;
    }
    default:
    {
      kDebug() << "Unknown battery state ..." << endl;
      break;
    }
  }
}


void Smb4KSolidInterface::slotBatteryChargePercentChanged( int value, const QString &udi )
{
  kDebug() << "Battery charge percent value: " << value << endl;
}


void Smb4KSolidInterface::slotNetworkStatusChanged( Solid::Networking::Status status )
{
  switch ( status )
  {
    case Solid::Networking::Connecting:
    {
      m_network_status = Connecting;
      break;
    }
    case Solid::Networking::Connected:
    {
      m_network_status = Connected;
      break;
    }
    case Solid::Networking::Disconnecting:
    {
      m_network_status = Disconnecting;
      break;
    }
    case Solid::Networking::Unconnected:
    {
      m_network_status = Disconnected;
      break;
    }
    case Solid::Networking::Unknown:
    default:
    {
      m_network_status = Unknown;
      break;
    }
  }

  emit networkStatusChanged( m_network_status );
}

#include "smb4ksolidinterface.moc"
