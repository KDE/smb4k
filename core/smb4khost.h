/***************************************************************************
    smb4khost  -  Smb4K's container class for information about a host.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KHOST_H
#define SMB4KHOST_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>

// KDE includes
#include <kdemacros.h>

// application specific includes
#include <smb4kbasicnetworkitem.h>

// forward declarations
class Smb4KAuthInfo;


/**
 * This class is a container that carries information about a host found in
 * the network neighborhood. It is part of the core classes of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KHost : public Smb4KBasicNetworkItem
{
  public:
    /**
     * The default constructor. It takes the name of the host as only argument.
     * You have to set all other information with the other functions provided
     * by this class.
     *
     * @param name                The name of the host
     */
    Smb4KHost( const QString &name );

    /**
     * The copy constructor. This constructor takes a Smb4KHost item as argument
     * and copies its values.
     *
     * @param host                The Smb4KHost object that is to be copied.
     */
    Smb4KHost( const Smb4KHost &host );

    /**
     * The empty constructor. It does not take any argument and you have to set
     * all information by the other functions provided by this class.
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
    void setHostName( const QString &name );

    /**
     * Returns the name of the host.
     *
     * @returns the host's name.
     */
    QString hostName() const { return item_url.host().toUpper(); }

    /**
     * This function sets the UNC (Uniform Naming Convention string). This
     * has to conform to the following scheme: [smb:]//[USER@]HOST[:PORT].
     *
     * The UNC may contain the protocol, i.e. "smb://". If a wrong protocol or a
     * mal-formatted UNC is passed, this function will return immediately without
     * doing anything.
     *
     * @param unc           The UNC of the share
     */
    void setUNC( const QString &unc );

    /**
     * Returns the UNC of the server in the form [smb:]//[USER@]HOST[:PORT] depending on
     * the format specified by @p options.
     *
     * @returns the UNC.
     */
    QString unc( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                   QUrl::RemoveUserInfo|
                                                   QUrl::RemovePort ) const;
                                                   
    /**
     * Sets the URL of the network item after some checks are passed.
     * 
     * @param url             The URL of the network item
     */
    void setURL( const QUrl &url );
    
    /**
     * Returns the URL (the full UNC) of the host item.
     * 
     * @returns the URL of the network item.
     */
    const QUrl &url() const { return item_url; }

    /**
     * Set the workgroup where this host is located.
     *
     * @param workgroup           The workgroup name
     */
    void setWorkgroupName( const QString &workgroup );

    /**
     * Returns the name of the workgroup where this host is located.
     *
     * @returns the workgroup name.
     */
    const QString &workgroupName() const { return m_workgroup; }

    /**
     * Set the IP address of this host. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * When this function is invoked, it sets the m_ip_checked property
     * to TRUE so that ipChecked() retruns TRUE.
     *
     * @param ip                  The IP address of this host.
     */
    void setIP( const QString &ip );

    /**
     * Returns the IP address of the host. If the IP address was not
     * compatible with IPv4 and IPv6 or if no IP address was supplied,
     * an empty string is returned.
     *
     * @returns the host's IP address or an empty string.
     */
    const QString &ip() const { return m_ip; }

    /**
     * Returns TRUE if the IP address has already been checked (i.e. set)
     * and false otherwise. It does not matter if actually there is an
     * IP address present. The only thing that is important is that
     * setIP() was called.
     *
     * @returns TRUE if the IP address has been checked.
     */
    bool ipChecked() const { return m_ip_checked; }

    /**
     * Set the comment that was defined for the host.
     *
     * @param comment             The comment string
     */
    void setComment( const QString &comment );

    /**
     * Returns the comment that was defined or an empty string if there
     * was no comment.
     *
     * @returns the comment or an empty string.
     */
    const QString &comment() const { return m_comment; }

    /**
     * Set the "Server" and the "OS" (operating system) strings as
     * they are reported by the host.
     *
     * When this function is invoked, it will set the m_info_checked
     * property to TRUE, so that infoChecked() returns TRUE.
     *
     * @param serverString        The "Server" string
     *
     * @param osString            The "OS" string
     */
    void setInfo( const QString &serverString = QString(),
                  const QString &osString = QString() );

    /**
     * With this function you can manually reset the "info is checked" flag,
     * i.e. it will be set to FALSE. In addition the server string and the
     * operating system string will be cleared.
     */
    void resetInfo();

    /**
     * Returns TRUE if the infomation has already been checked (i.e. set)
     * and FALSE otherwise. It does not matter if actually there is some
     * information present. The only thing that is important is that
     * setInfo() was called.
     *
     * @returns TRUE if the infomation has been checked.
     */
    bool infoChecked() const { return m_info_checked; }

    /**
     * Returns the "Server" string as reported by the host.
     *
     * @returns the "Server" string.
     */
    const QString &serverString() const { return m_server_string; }

    /**
     * Returns the "OS" (operating system) string as reported by the
     * host.
     *
     * @returns the OS string.
     */
    const QString &osString() const { return m_os_string; }

    /**
     * Set this host to be a master browser.
     *
     * @param master              Set this to TRUE if the host is a master
     *                            browser.
     */
    void setIsMasterBrowser( bool master );

    /**
     * Returns TRUE if the host is a master browser and FALSE otherwise.
     *
     * @returns TRUE if the host is a master browser.
     */
    bool isMasterBrowser() const { return m_is_master; }

    /**
     * Returns TRUE if the item is empty and FALSE otherwise. An item is not
     * empty if at least one string (workgroup name, master name, etc.) has been
     * set. A modified boolean will not be considered.
     *
     * @returns TRUE if the item is empty.
     */
    bool isEmpty() const;

    /**
     * Set the port for the use in the UNC.
     *
     * @param port            The port
     */
    void setPort( int port );

    /**
     * Returns the port that is used in the UNC.
     *
     * @returns the port.
     */
    int port() const { return item_url.port(); }

    /**
     * Compare another Smb4KHost object with this one an return TRUE if both carry
     * the same data.
     *
     * @param host            The Smb4KHost object that should be compared with this
     *                        one.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals( Smb4KHost *host ) const;
    
    /**
     * Operator to check if two hosts are equal.
     */
    bool operator==( Smb4KHost host ) { return equals( &host ); }

    /**
     * Set the authentication information for the host. This function will add
     * the authentication information to the URL of the host. Any previous
     * user information including the login will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo( Smb4KAuthInfo *authInfo );
    
    /**
     * Set the login name for the host.
     *
     * @param login               The login name
     */
    void setLogin( const QString &login );

    /**
     * Returns the login name.
     *
     * @returns the login name.
     */
    QString login() const { return item_url.userName(); }
    
    /**
     * Returns the password.
     * 
     * @returns the password.
     */
    QString password() const { return item_url.password(); }
    
    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     * 
     * @returns TRUE if the host's IP address is known.
     */
    bool hasIP() const { return !m_ip.isEmpty(); }

  private:
    /**
     * The workgroup the host is in
     */
    QString m_workgroup;

    /**
     * The host's IP address
     */
    QString m_ip;

    /**
     * The comment
     */
    QString m_comment;

    /**
     * The server string
     */
    QString m_server_string;

    /**
     * The OS string
     */
    QString m_os_string;

    /**
     * Have we checked for info yet?
     */
    bool m_info_checked;

    /**
     * Have we already checked for the IP address?
     */
    bool m_ip_checked;

    /**
     * Determines if the host is a master browser
     */
    bool m_is_master;

    /**
     * This function checks if the given IP address is either
     * compatible with IPv4 or IPv6. If it is not, an empty string
     * is returned.
     *
     * @param ip              The IP address that needs to be checked.
     *
     * @returns the IP address or an empty string if the IP address
     * is not compatible with either IPv4 or IPv6.
     */
    const QString &ipIsValid( const QString &ip );
};

#endif
