/***************************************************************************
    Smb4K's container class for information about a workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2021 by Alexander Reinholdt
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

#ifndef SMB4KWORKGROUP_H
#define SMB4KWORKGROUP_H

// application specific includes
#include "smb4kbasicnetworkitem.h"

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

class Q_DECL_EXPORT Smb4KWorkgroup : public Smb4KBasicNetworkItem
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
    SMB4K_DEPRECATED explicit Smb4KWorkgroup(const QString &name);

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

private:
    const QScopedPointer<Smb4KWorkgroupPrivate> d;
};

#endif
