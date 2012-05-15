/***************************************************************************
    smb4kauthinfo.h  -  This class provides a container for the
    authentication data.
                             -------------------
    begin                : Sa Feb 28 2004
    copyright            : (C) 2004-2012 by Alexander Reinholdt
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
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QScopedPointer>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KAuthInfoPrivate;

/**
 * This class provides a container for the authentication data.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */


class KDE_EXPORT Smb4KAuthInfo
{
  friend class Smb4KAuthInfoPrivate;
  
  public:
    /**
     * Enumeration that determines the type of network item the
     * authentication information is for.
     */
    enum Type { Host,
                Share,
                Default,
                Unknown };

    /**
     * Constructor for a host item.
     *
     * @param host      The Smb4KHost item.
     */
    Smb4KAuthInfo( const Smb4KHost *host );


    /**
     * Constructor for a share item.
     * 
     * In case the share is a 'homes' share, this constructor will automatically
     * use the Smb4KShare::homeUNC() function to set the UNC.
     *
     * @param share     The Smb4KShare item.
     */
    Smb4KAuthInfo( const Smb4KShare *share );

    /**
     * The empty constructor.
     */
    Smb4KAuthInfo();

    /**
     * The copy constructor.
     *
     * @param info      The Smb4KAuthInfo object that will be copied.
     */
    Smb4KAuthInfo( const Smb4KAuthInfo &info );

    /**
     * The destructor
     */
    ~Smb4KAuthInfo();

    /**
     * Set the host item. This overwrites all previous data that this object
     * might have carried including the password.
     *
     * @param host      The Smb4KHost item
     */
    void setHost( Smb4KHost *host );

    /**
     * Set the share item. This overwrites all previous data that this object
     * might have carried including the password.
     * 
     * In case the share is a 'homes' share, this function will automatically
     * use the Smb4KShare::homeUNC() function to set the UNC.
     *
     * @param share     The Smb4KShare item
     */
    void setShare( Smb4KShare *share );

    /**
     * Sets the workgroup name. This function should only be used if you neither can
     * use setHost() nor setShare().
     *
     * @param workgroup The name of the workgroup
     */
    void setWorkgroupName( const QString &workgroup );

    /**
     * Returns the name of the workgroup.
     *
     * @returns         The workgroup of the server/share for which this
     *                  authentication data is for.
     */
    QString workgroupName() const;
    
    /**
     * Returns the UNC in the form [smb:]//[USER:PASSWORD@]HOST/SHARE depending on
     * the format specified by @p options.
     *
     * @returns the UNC.
     */
    QString unc( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                   QUrl::RemoveUserInfo|
                                                   QUrl::RemovePort ) const;

    /**
     * Returns the host's UNC in the form [smb:]//[USER:PASSWORD@]HOST depending on
     * the format specified by @p options.
     *
     * @returns the UNC of the host.
     */
    QString hostUNC( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                       QUrl::RemoveUserInfo|
                                                       QUrl::RemovePort ) const;

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
     * Sets the login.
     * 
     * In case of a 'homes' share, this function will also set the share
     * name to @p login.
     *
     * @param login     The login for the server/share
     */
    void setLogin( const QString &login );

    /**
     * Returns the login name.
     *
     * @returns         The login
     */
    QString login() const;

    /**
     * Sets the password.
     *
     * @param passwd    The password for the server/share
     */
    void setPassword( const QString &passwd );

    /**
     * Returns the password.
     */
    QString password() const;

    /**
     * Returns the type.
     *
     * @returns the type.
     */
    Type type() const;

    /**
     * If the item is a homes share, this function returns TRUE. In
     * all other cases, this function returns FALSE.
     *
     * @returns TRUE if the item is a homes share.
     */
    bool isHomesShare() const;

    /**
     * This function sets the type of this authentication information to
     * "Default", i.e. it carries the default authentication information.
     */
    void useDefaultAuthInfo();

    /**
     * Compare another Smb4KAuthInfo object with this one an return TRUE if both carry
     * the same data.
     *
     * @param info            The Smb4KAuthInfo object that should be compared with this
     *                        one.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals( Smb4KAuthInfo *info ) const;
    
    /**
     * Operator to check if two authentication informations are equal.
     */
    bool operator==( Smb4KAuthInfo info ) const { return equals( &info ); }

    /**
     * Sets the URL of the share after some checks are passed.
     *
     * @param url             The URL of the network item
     */
    void setURL( const QUrl &url );

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
    void setIP( const QString &ip );

    /**
     * Returns the IP address
     *
     * @returns the IP address
     */
    QString ip() const;

  private:
    /**
     * Pointer to Smb4KAuthInfoPrivate class
     */
    const QScopedPointer<Smb4KAuthInfoPrivate> d;
};

#endif
