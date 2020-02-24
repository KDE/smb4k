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

#ifndef SMB4KHARDWAREINTERFACE_H
#define SMB4KHARDWAREINTERFACE_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QScopedPointer>
#include <QUrl>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

class Smb4KHardwareInterfacePrivate;

/**
 * This class provides an interface to the computer's hardware.
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 2.0.0
 */

class Q_DECL_EXPORT Smb4KHardwareInterface : public QObject
{
  Q_OBJECT
  
  friend class Smb4KHardwareInterfacePrivate;
  
  public:
    /**
     * The constructor
     */
    explicit Smb4KHardwareInterface(QObject *parent = 0);
    
    /**
     * The destructor
     */
    ~Smb4KHardwareInterface();
    
    /**
     * The static pointer to this class.
     * @returns a static pointer to this class
     */
    static Smb4KHardwareInterface *self();
    
    /**
     * This function checks the current network configuration.
     */
    void updateNetworkConfig();
    
    /**
     * This function returns TRUE if the system is online and FALSE otherwise.
     * @returns TRUE if the system is online.
     */
    bool isOnline() const;

#if defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
  protected:
    /**
     * Reimplemented from QObject to check for mounts and unmounts on operating
     * systems that are not fully supported by Solid, yet.
     */
    void timerEvent(QTimerEvent *e) override;
#endif
    
    /**
     * Inhibit shutdown and sleep.
     */
    void inhibit();
    
    /**
     * Uninhibit shutdown and sleep.
     */
    void uninhibit();
    
  Q_SIGNALS:
    /**
     * This signal is emitted when a network share is added to the system
     */
    void networkShareAdded();
    
    /**
     * This signal is emitted when a network share is removed from the system
     */
    void networkShareRemoved();
    
    /**
     * This signal is emitted when the network configuration was updated.
     */
    void networkConfigUpdated();
    
    /**
     * This signal is emitted when the online state changed.
     */
    void onlineStateChanged(bool online);
    
  protected Q_SLOTS:
    /**
     * This slot is called by the QNetworkConfigurationManager::updateCompleted()
     * signal and sets d->networkConfigUpdated to TRUE.
     */
    void slotNetworkConfigUpdated();
    
    /**
     * This slot is called when the state of the network connection changed. 
     * It is connected to the QNetworkSession::stateChanged() signal.
     */
    void slotConnectionStateChanged(QNetworkSession::State state);
    
    /**
     * This slot is called when a device was added to the system.
     * @param udi     the device UDI
     */
    void slotDeviceAdded(const QString &udi);
    
    /**
     * This slot is called when a device was removed from the system.
     * @param udi     the device UDI
     */
    void slotDeviceRemoved(const QString &udi);
    
  private:
    QScopedPointer<Smb4KHardwareInterfacePrivate> d;
};

#endif
