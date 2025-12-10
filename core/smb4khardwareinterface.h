/*
    Provides an interface to the computer's hardware

    SPDX-FileCopyrightText: 2015-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KHARDWAREINTERFACE_H
#define SMB4KHARDWAREINTERFACE_H

// application specific includes
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QObject>
#include <QScopedPointer>
#include <QUrl>

class Smb4KHardwareInterfacePrivate;

/**
 * This class provides an interface to the computer's hardware.
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 2.0.0
 */

class SMB4KCORE_EXPORT Smb4KHardwareInterface : public QObject
{
    Q_OBJECT

    friend class Smb4KHardwareInterfacePrivate;

public:
    /**
     * The constructor
     */
    explicit Smb4KHardwareInterface(QObject *parent = nullptr);

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
     * This function returns TRUE if the system is online and FALSE otherwise.
     *
     * @returns TRUE if the system is online.
     */
    bool isOnline() const;

    /**
     * Inhibit shutdown and sleep.
     */
    void inhibit();

    /**
     * Uninhibit shutdown and sleep.
     */
    void uninhibit();

    /**
     * This function returns TRUE if the initial list of mountpoints
     * of all Samba shares is currently being imported.
     *
     * @returns TRUE if the the initial import is in progress
     */
    bool initialImportDone() const;

    /**
     * This function returns the list of all Samba shares mounted on
     * the system.
     *
     * @returns the list of all mounted Samba shares.
     */
    QStringList allMountPoints() const;

protected:
    /**
     * Reimplemented from QObject to check the online state and to check
     * for mounts and unmounts on operating systems that are not fully
     * supported by Solid, yet.
     */
    void timerEvent(QTimerEvent *event) override;

Q_SIGNALS:
    /**
     * This signal is emitted when a network share is added to the system
     *
     * @param mountpoint    The mountpoint of the share
     */
    void networkShareAdded(const QString &mountpoint);

    /**
     * This signal is emitted when a network share is removed from the system
     *
     * @param mountpoint    The mountpoint of the share
     */
    void networkShareRemoved(const QString &mountpoint);

    /**
     * This signal is emitted when the online state changed.
     * @param online    TRUE if the system went online and FALSE otherwise
     */
    void onlineStateChanged(bool online);

protected Q_SLOTS:
#if defined(Q_OS_LINUX)
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
#endif

    /**
     * This slot is called when the system prepares for sleep or has
     * been woken up.
     *
     * @param sleep     TRUE if the system is about to enter a sleep
     *                  state and FALSE otherwise
     */
    void slotSystemSleep(bool sleep);

private:
    /**
     * Check the online state and emit the @see onlineStateChanged() accordingly, if
     * @p emitSignal is set to TRUE.
     */
    void checkOnlineState(bool emitSignal = true);

    /**
     * Pointer to private class
     */
    QScopedPointer<Smb4KHardwareInterfacePrivate> d;
};

#endif
