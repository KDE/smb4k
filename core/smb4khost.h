/*
    Smb4K's container class for information about a host.

    SPDX-FileCopyrightText: 2008-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KHOST_H
#define SMB4KHOST_H

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QHostAddress>
#include <QScopedPointer>
#include <QString>

// forward declarations
class Smb4KAuthInfo;
class Smb4KHostPrivate;

/**
 * This class is a container that carries information about a host found in
 * the network neighborhood. It is part of the core classes of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KHost : public Smb4KBasicNetworkItem
{
    friend class Smb4KHostPrivate;

public:
    /**
     * This constructor takes the URL @p url as argument.
     *
     * @param url           The URL
     */
    explicit Smb4KHost(const QUrl &url);

    /**
     * The copy constructor.
     *
     * @param host                The Smb4KHost object that is to be copied.
     */
    Smb4KHost(const Smb4KHost &host);

    /**
     * The empty constructor.
     */
    Smb4KHost();

    /**
     * The destructor.
     */
    ~Smb4KHost();

    /**
     * Set the name of the host.
     *
     * @param name                The name of the host
     */
    void setHostName(const QString &name);

    /**
     * Returns the name of the host.
     *
     * @returns the host's name.
     */
    QString hostName() const;

    /**
     * Set the workgroup where this host is located.
     *
     * @param workgroup           The workgroup name
     */
    void setWorkgroupName(const QString &workgroup);

    /**
     * Returns the name of the workgroup where this host is located.
     *
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Set the IP address of this host. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param ip                  The IP address of this host.
     */
    void setIpAddress(const QString &ip);

    /**
     * Set the IP address of this host. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param ip                  The IP address of this host.
     */
    void setIpAddress(const QHostAddress &address);

    /**
     * Returns the IP address of the host. If the IP address was not
     * compatible with IPv4 and IPv6 or if no IP address was supplied,
     * an empty string is returned.
     *
     * @returns the host's IP address or an empty string.
     */
    QString ipAddress() const;

    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     *
     * @returns TRUE if the host's IP address is known.
     */
    bool hasIpAddress() const;

    /**
     * Set this host to be a master browser.
     *
     * @param master              Set this to TRUE if the host is a master
     *                            browser.
     */
    void setIsMasterBrowser(bool master);

    /**
     * Returns TRUE if the host is a master browser and FALSE otherwise.
     *
     * @returns TRUE if the host is a master browser.
     */
    bool isMasterBrowser() const;

    /**
     * Set the port for the use in the URL.
     *
     * @param port            The port
     */
    void setPort(int port);

    /**
     * Returns the port that is used in the URL.
     *
     * @returns the port.
     */
    int port() const;

    /**
     * Set the user name for the host.
     *
     * @param name          The login name
     */
    void setUserName(const QString &name);

    /**
     * Returns the user name.
     *
     * @returns the user name.
     */
    QString userName() const;

    /**
     * Set the password used for authentication.
     *
     * @param passwd              The password
     */
    void setPassword(const QString &passwd);

    /**
     * Returns the password.
     *
     * @returns the password.
     */
    QString password() const;

    /**
     * Updates the host item if the workgroup and host name of @p host and
     * of this item is equal. Otherwise it does nothing.
     * @param host            The share object that is used to update
     *                        this object
     */
    void update(Smb4KHost *host);

private:
    const QScopedPointer<Smb4KHostPrivate> d;
};

#endif
