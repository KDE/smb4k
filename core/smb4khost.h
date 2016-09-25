/***************************************************************************
    smb4khost  -  Smb4K's container class for information about a host.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2015 by Alexander Reinholdt
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

#ifndef SMB4KHOST_H
#define SMB4KHOST_H

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QString>
#include <QScopedPointer>
#include <QUrl>

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
     * The default constructor. It takes the name of the host as only argument.
     * You have to set all other information with the other functions provided
     * by this class.
     *
     * @param name                The name of the host
     */
    explicit Smb4KHost(const QString &name);

    /**
     * The copy constructor. This constructor takes a Smb4KHost item as argument
     * and copies its values.
     *
     * @param host                The Smb4KHost object that is to be copied.
     */
    Smb4KHost(const Smb4KHost &host);

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
    void setHostName(const QString &name);

    /**
     * Returns the name of the host.
     *
     * @returns the host's name.
     */
    QString hostName() const; 

    /**
     * Returns the UNC (Uniform Naming Convention string) in the form //HOST.
     * 
     * This function should only be used for basic comparisons or for display
     * purposes. If you need to do sophisticated comparisons, use the url() 
     * function instead.
     *
     * Please note that this function returns a modified URL string (uppercase
     * hostname, etc.) and automatically strips a trailing slash if one is present.
     *
     * @returns the UNC.
     */
    QString unc() const;
                                                   
    /**
     * Sets the URL of the network item after some checks are passed.
     * 
     * @param url             The URL of the network item
     */
    void setURL(const QUrl &url);
    
    /**
     * Returns the URL (the full UNC) of the host item.
     * 
     * @returns the URL of the network item.
     */
    QUrl url() const;

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
    void setIP(const QString &ip);

    /**
     * Returns the IP address of the host. If the IP address was not
     * compatible with IPv4 and IPv6 or if no IP address was supplied,
     * an empty string is returned.
     *
     * @returns the host's IP address or an empty string.
     */
    QString ip() const;
    
    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     * 
     * @returns TRUE if the host's IP address is known.
     */
    bool hasIP() const;

    /**
     * Set the comment that was defined for the host.
     *
     * @param comment             The comment string
     */
    void setComment(const QString &comment);

    /**
     * Returns the comment that was defined or an empty string if there
     * was no comment.
     *
     * @returns the comment or an empty string.
     */
    QString comment() const;

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
    void setPort(int port);

    /**
     * Returns the port that is used in the UNC.
     *
     * @returns the port.
     */
    int port() const;

    /**
     * Compare another Smb4KHost object with this one an return TRUE if both carry
     * the same data.
     *
     * @param host            The Smb4KHost object that should be compared with this
     *                        one.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals(Smb4KHost *host) const;
    
    /**
     * Operator to check if two hosts are equal.
     */
    bool operator==(Smb4KHost host) const { return equals(&host); }

    /**
     * Set the authentication information for the host. This function will add
     * the authentication information to the URL of the host. Any previous
     * user information including the login will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo(Smb4KAuthInfo *authInfo);
    
    /**
     * Set the login name for the host.
     *
     * @param login               The login name
     */
    void setLogin(const QString &login);

    /**
     * Returns the login name.
     *
     * @returns the login name.
     */
    QString login() const;
    
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

  private:
    const QScopedPointer<Smb4KHostPrivate> d;
};

#endif
