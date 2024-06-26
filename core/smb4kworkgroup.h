/*
    Smb4K's container class for information about a workgroup.

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KWORKGROUP_H
#define SMB4KWORKGROUP_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kcore_export.h"

// Qt includes
#include <QHostAddress>
#include <QScopedPointer>
#include <QString>

// forward declarations
class Smb4KWorkgroupPrivate;

/**
 * This class is a container that carries information about a workgroup or
 * domain found in the network neighborhood. It is part of the core classes
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class SMB4KCORE_EXPORT Smb4KWorkgroup : public Smb4KBasicNetworkItem
{
    friend class Smb4KWorkgroupPrivate;

public:
    /**
     * The default constructor. It takes the name of the workgroup as only
     * argument. You have to set all other information by the other functions
     * provided by this class.
     *
     * @param name            The name of the workgroup or domain.
     */
    explicit Smb4KWorkgroup(const QString &name);

    /**
     * The copy constructor. This constructor takes another Smb4KWorkgroup item
     * as argument and copies its values.
     *
     * @param workgroup       The Smb4KWorkgroup item that is to be copied.
     */
    Smb4KWorkgroup(const Smb4KWorkgroup &workgroup);

    /**
     * The empty constructor. It does not take any argument and you have to
     * set all information by the other functions provided by this class.
     */
    Smb4KWorkgroup();

    /**
     * The destructor.
     */
    ~Smb4KWorkgroup();

    /**
     * Sets the name of the workgroup.
     *
     * @param name            The name of the workgroup
     */
    void setWorkgroupName(const QString &name);

    /**
     * This function returns the name of the workgroup.
     *
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Sets the name of the master browser of this workgroup or domain.
     *
     * @param masterName      The name of the master browser
     */
    void setMasterBrowserName(const QString &name);

    /**
     * Returns the name of the master browser of this workgroup or domain.
     *
     * @returns the name of the master browser.
     */
    QString masterBrowserName() const;

    /**
     * Returns TRUE if there is a master browser and FALSE otherwise. No master
     * browser might be defined, if the DNS-SD method is used.
     *
     * @returns TRUE if there is a master browser.
     */
    bool hasMasterBrowser() const;

    /**
     * Set the IP address of the master browser. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The master browser's IP address
     */
    void setMasterBrowserIpAddress(const QString &ip);

    /**
     * Set the IP address of the master browser. @p address will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param address         The master browser's IP address
     */
    void setMasterBrowserIpAddress(const QHostAddress &address);

    /**
     * Returns the IP address of the master browser of this workgroup
     * or domain. If the IP address was not compatible with IPv4 and
     * IPv6 or if no IP address was supplied, an empty string is returned.
     *
     * @returns the IP address of the master browser or an empty string.
     */
    QString masterBrowserIpAddress() const;

    /**
     * Returns TRUE if the workgroup/domain master browsers IP address is set and
     * FALSE otherwise.
     *
     * @returns TRUE if the master browsers IP address is known.
     */
    bool hasMasterBrowserIpAddress() const;

    /**
     * Updates the workgroup item if the workgroup name of @p workgroup and
     * of this item is equal. Otherwise it does nothing.
     * @param workgroup       The workgroup object that is used to update
     *                        this object
     */
    void update(Smb4KWorkgroup *workgroup);

    /**
     * Copy assignment operator
     */
    Smb4KWorkgroup &operator=(const Smb4KWorkgroup &other);

private:
    const QScopedPointer<Smb4KWorkgroupPrivate> d;
};

Q_DECLARE_METATYPE(Smb4KWorkgroup)

#endif
