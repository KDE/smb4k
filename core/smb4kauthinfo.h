/***************************************************************************
    This class provides a container for the authentication data.
                             -------------------
    begin                : Sa Feb 28 2004
    copyright            : (C) 2004-2019 by Alexander Reinholdt
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

#ifndef SMB4KAUTHINFO_H
#define SMB4KAUTHINFO_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobal.h"

// Qt includes
#include <QString>
#include <QScopedPointer>
#include <QUrl>

// forward declarations
class Smb4KAuthInfoPrivate;

using namespace Smb4KGlobal;

/**
 * This class provides a container for the authentication data.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class Q_DECL_EXPORT Smb4KAuthInfo
{
  friend class Smb4KAuthInfoPrivate;
  
  public:
    /**
     * Constructor
     * 
     * @param item      The network item
     */
    explicit Smb4KAuthInfo(Smb4KBasicNetworkItem *item);

    /**
     * The empty constructor.
     */
    Smb4KAuthInfo();

    /**
     * The copy constructor.
     *
     * @param info      The Smb4KAuthInfo object that will be copied.
     */
    Smb4KAuthInfo(const Smb4KAuthInfo &info);

    /**
     * The destructor
     */
    ~Smb4KAuthInfo();
    
    /**
     * Sets the workgroup name.
     *
     * @param workgroup The name of the workgroup
     */
    void setWorkgroupName(const QString &workgroup);

    /**
     * Returns the name of the workgroup.
     *
     * @returns         The workgroup of the server/share for which this
     *                  authentication data is for.
     */
    QString workgroupName() const;
    
    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const;

    /**
     * Returns the share name.
     *
     * @returns the share name.
     */
    QString shareName() const;

    /**
     * Sets the username.
     * 
     * In case of a 'homes' share, this function will also set the share
     * name to @p username.
     *
     * @param username  The login for the server/share
     */
    void setUserName(const QString &username);

    /**
     * Returns the username.
     *
     * @returns         The username
     */
    QString userName() const;

    /**
     * Sets the password.
     *
     * @param passwd    The password for the server/share
     */
    void setPassword(const QString &passwd);

    /**
     * Returns the password.
     */
    QString password() const;

    /**
     * Returns the type.
     *
     * @returns the type.
     */
    Smb4KGlobal::NetworkItem type() const;

    /**
     * If the item is a homes share, this function returns TRUE. In
     * all other cases, this function returns FALSE.
     *
     * @returns TRUE if the item is a homes share.
     */
    bool isHomesShare() const;

    /**
     * Sets the URL of the share after some checks are passed.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QUrl &url);
    
    /**
     * Sets the URL of the share.
     *
     * @param url             The URL of the network item
     */
    void setUrl(const QString &url);

    /**
     * Returns the URL of the network item
     *
     * @returns the URL
     */
    QUrl url() const;

    /**
     * Sets the IP address for this authentication information object
     *
     * @param ip          The IP address
     */
    void setIpAddress(const QString &ip);

    /**
     * Returns the IP address
     *
     * @returns the IP address
     */
    QString ipAddress() const;
    
    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

  private:
    /**
     * Pointer to Smb4KAuthInfoPrivate class
     */
    const QScopedPointer<Smb4KAuthInfoPrivate> d;
};

#endif
