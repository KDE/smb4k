/***************************************************************************
    smb4ksolidinterface  -  This class provides an interface to KDE's
    Solid framework.
                             -------------------
    begin                : So Sep 14 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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

#ifndef SMB4KSOLIDINTERFACE_H
#define SMB4KSOLIDINTERFACE_H

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

// KDE includes
#include <kdemacros.h>
#include <solid/button.h>
#include <solid/networking.h>

// forward declarations
class Smb4KSolidInterfacePrivate;

/**
 * This class provides an interface to the hardware of the computer
 * Smb4K is running on. It is used to react on network connects and
 * disconnects, hibernation, etc.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

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
     * The constructor
     */
    explicit Smb4KSolidInterface( QObject *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSolidInterface();

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
    ConnectionStatus networkStatus() const;
    
    /**
     * Suppress the sleep of the system. This function should be
     * called when the application should do something before the
     * sleep kicks in.
     *
     * @param reason      The reason why the sleep is suppressed
     */
    void beginSleepSuppression( const QString &reason = QString() );
    
    /**
     * Stop suppressing the sleep of the system.
     */
    void endSleepSuppression();
    
  Q_SIGNALS:
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
     * This signal is a convenience signal and is emitted when the computer 
     * woke up. It is connected to the Solid::Powermanagement::Notifier::resumingFromSuspend()
     * signal.
     */
    void wokeUp();

  protected Q_SLOTS:
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
     * Initialize the solid interface. Look for all devices that are
     * already there and that should observed by the solid interface.
     */
    void init();

    /**
     * Pointer to Smb4KSolidInterfacePrivate class
     */
    const QScopedPointer<Smb4KSolidInterfacePrivate> d;
};

#endif
