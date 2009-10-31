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

#ifndef SMB4KSOLIDINTERFACE_H
#define SMB4KSOLIDINTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QMap>

// KDE includes
#include <kdemacros.h>
#include <solid/button.h>
#include <solid/networking.h>

class KDE_EXPORT Smb4KSolidInterface : public QObject
{
  Q_OBJECT

  friend class Smb4KSolidInterfacePrivate;

  public:
    /**
     * This enumeration is used to identify the button that
     * was pressed.
     */
    enum ButtonType { SleepButton,
                      LidButton,
                      PowerButton,
                      UnknownButton };

   /**
    * This enumeration is used for the connection state of
    * the computer.
    */
    enum ConnectionStatus { Connecting,
                            Connected,
                            Disconnecting,
                            Disconnected,
                            Unknown };

    /**
     * Returns a static pointer to this class.
     *
     * @returns a static pointer to this class.
     */
    static Smb4KSolidInterface *self();

    /**
     * This function returns the current network status.
     *
     * @returns the network current status.
     */
    ConnectionStatus networkStatus() { return m_network_status; }

  signals:
    /**
     * This signal is emitted when a hardware button was pressed.
     *
     * @param type      Type of the button according to ButtonType
     *                  enumeration
     */
    void buttonPressed( Smb4KSolidInterface::ButtonType type );

    /**
     * This signal is emitted when the network status changed.
     *
     * @param status    The network status according to NetworkStatus
     *                  enumeration
     */
    void networkStatusChanged( Smb4KSolidInterface::ConnectionStatus status );

    /**
     * This signal is emitted when the computer woke up.
     */
    void wokeUp();

  protected slots:
    /**
     * This slot is connected to the deviceAdded() signal of the
     * device notifier. It is called whenever a device was added.
     *
     * @param udi       The UDI of the device
     */
    void slotDeviceAdded( const QString &udi );

    /**
     * This slot is connected to the deviceRemoved() signal of the
     * device notifier. It is called whenever a device was removed.
     *
     * @param udi       The UDI of the device
     */
    void slotDeviceRemoved( const QString &udi );

    /**
     * This slot is connected to the pressed() signal of each button
     * that was found.
     *
     * @param type      The type of the button that was pressed
     *
     * @param udi       The UDI of the button that was pressed
     */
    void slotButtonPressed( Solid::Button::ButtonType type,
                            const QString &udi );

    /**
     * This slot is connected to the plugStateChanged() signal of the
     * AC adapters.
     *
     * @param state     The plugged/unplugged state
     *
     * @param udi       The UDI of the AC adapter
     */
    void slotAcPlugStateChanged( bool state,
                                 const QString &udi );

    /**
     * This slot is connected to the chargeStateChanged() signal of the
     * primary batteries.
     *
     * @param state     The charge state according to Solid::Battery::ChargeState
     *
     * @param udi       The UDI of the battery
     */
    void slotBatteryChargeStateChanged( int state,
                                        const QString &udi );

    /**
     * This slot is connected to the chargePercentChanged() signal of the
     * primary batteries.
     *
     * @param value     The charge percent value
     *
     * @param udi       The UDI of the battery
     */
    void slotBatteryChargePercentChanged( int value,
                                          const QString &udi );

    /**
     * This slot is connected to the statusChanged() signal of the network
     * notifier.
     *
     * @param status    The network status
     */
    void slotNetworkStatusChanged( Solid::Networking::Status status );

  private:
    /**
     * The constructor
     */
    Smb4KSolidInterface();

    /**
     * The destructor
     */
    ~Smb4KSolidInterface();

    /**
     * Initialize the solid interface. Look for all devices that are
     * already there and that should observed by the solid interface.
     */
    void init();

    /**
     * Which button was pressed?
     */
    ButtonType m_button_pressed;

    /**
     * Network status.
     */
    ConnectionStatus m_network_status;

    /**
     * In this map all devices are stored that were removed when one
     * of the special buttons (sleep, lid, power) was pressed. It is
     * used to figure out, when the notebook wakes up again. It takes
     * the UDI and the button type.
     */
    QMap<QString, ButtonType> m_removed_devices;

    /**
     * The powermanagement cookie (sleep state prevention)
     */
    int m_cookie;
};

#endif
