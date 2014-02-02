/***************************************************************************
    smb4kcustomoptions - This class carries custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2013 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONS_H
#define SMB4KCUSTOMOPTIONS_H

// application specific includes
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kglobal.h"

// Qt includes
#include <QtCore/QScopedPointer>

// KDE includes
#include <kdemacros.h>
#include <kurl.h>

// forward declarations
class Smb4KCustomOptionsPrivate;

using namespace Smb4KGlobal;

/**
 * This class stored the custom options defined for a certain host
 * or share.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class KDE_EXPORT Smb4KCustomOptions
{
  friend class Smb4KCustomOptionsPrivate;
  
  public:
    /**
     * Remount enumeration
     */
    enum Remount { DoRemount,
                   NoRemount,
                   UndefinedRemount };
                  
    /**
     * ProtocolHint enumeration
     */
    enum ProtocolHint { Automatic,
                        RAP,
                        RPC,
                        ADS,
                        UndefinedProtocolHint };

#ifndef Q_OS_FREEBSD
    /**
     * The WriteAccess enumeration
     */
    enum WriteAccess { ReadWrite,
                       ReadOnly,
                       UndefinedWriteAccess };

    /**
     * The SecurityMode enumeration
     */
    enum SecurityMode { NoSecurityMode,
                        Krb5,
                        Krb5i,
                        Ntlm,
                        Ntlmi,
                        Ntlmv2,
                        Ntlmv2i,
                        Ntlmssp,
                        Ntlmsspi,
                        UndefinedSecurityMode };
#endif
                       
    /**
     * The Kerberos enumeration
     */
    enum Kerberos { UseKerberos,
                    NoKerberos,
                    UndefinedKerberos };
                  
    /**
     * Constructor for a host
     */
    explicit Smb4KCustomOptions( Smb4KHost *host );
                
    /**
     * Constructor for a share
     */
    explicit Smb4KCustomOptions( Smb4KShare *share );
    
    /**
     * Copy constructor
     */
    Smb4KCustomOptions( const Smb4KCustomOptions &options );
    
    /**
     * Empty constructor
     */
    Smb4KCustomOptions();
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptions();
    
    /**
     * Sets the host object. If you already set a network item before,
     * this function will do nothing.
     * 
     * @param host          The host object
     */
    void setHost( Smb4KHost *host );
    
    /**
     * Sets the share object. If you already set a host item before,
     * you can overwrite it with this function if the host names and
     * workgroup names match. This way you can propagate options that 
     * were defined for a host to one of its shares.
     * 
     * @param share         The host object
     */
    void setShare( Smb4KShare *share );
    
    /**
     * Returns the type of the network item for that the options
     * are defined
     * 
     * @returns the type of the network item
     */
    Smb4KGlobal::NetworkItem type() const;
    
    /**
     * If the network item is of type Share, set if it should be remounted.
     * If the network item is not of type Share, this function does nothing.
     * 
     * @param remount       One entry of the Remount enumeration
     */
    void setRemount( Remount remount );
    
    /**
     * Returns if the network item should be remounted.
     * 
     * @returns if the network item should be remounted.
     */
    Remount remount() const;
    
    /**
     * Set the profile this custom options object belongs to. The profile is 
     * meant to distinguish between several network environments, like home
     * and work.
     * 
     * @param profile         The profile name
     */
    void setProfile( const QString &profile );
    
    /**
     * Returns the name of the profile this custom options object belongs to.
     * 
     * @returns the profile name
     */
    QString profile() const;
    
    /**
     * Sets the workgroup name.
     * 
     * @param workgroup       The workgroup name
     */
    void setWorkgroupName( const QString &workgroup );
    
    /**
     * Returns the workgroup name.
     * 
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Sets the UNC/URL of the network item
     * 
     * @param url             The URL
     */
    void setURL( const KUrl &url );
    
    /**
     * Sets the UNC/URL of the network item
     *
     * @param url             The URL of the network item
     */
    void setURL( const QString &url );
    
    /**
     * Returns the URL of the network item
     * 
     * @returns the URL
     */
    KUrl url() const;

    /**
     * Returns the UNC in the form //HOST/Share.
     * 
     * This function should only be used for basic comparisons or for display
     * purposes. If you need to do sophisticated comparisons, use the url() 
     * function instead.
     *
     * @returns the UNC.
     */
    QString unc() const;

    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const;

    /**
     * Returns the share name, if appropriate, or an empty string.
     *
     * @returns the share name
     */
    QString shareName() const;
                                                   
    /**
     * Sets the IP address for the network item
     * 
     * @param ip              The IP address
     */
    void setIP( const QString &ip );
    
    /**
     * Returns the IP address of the network item
     * 
     * @returns the IP address
     */
    QString ip() const;
    
    /**
     * Set the SMB port to use with this host or share.
     * 
     * @param port            The SMB port
     */
    void setSMBPort( int port );
    
    /**
     * Returns the SMB port. The default value is 139.
     * 
     * @returns the SMB port
     */
    int smbPort() const;
    
#ifndef Q_OS_FREEBSD
    /**
     * Set the port that is to be used with mounting for a single share or all
     * shares of a host.
     * 
     * @param port            The file system port
     */
    void setFileSystemPort( int port );
    
    /**
     * Returns the file system port. The default value is 445.
     * 
     * @returns the file system port
     */
    int fileSystemPort() const;
    
    /**
     * Set the write access for either a single share or all shares of a host. 
     * 
     * @param access          The write access
     */
    void setWriteAccess( WriteAccess access );
    
    /**
     * Returns the write access for the share.
     * 
     * @returns the write access
     */
    WriteAccess writeAccess() const;

    /**
     * Set the security mode for mounting.
     *
     * @param mode            The security mode
     */
    void setSecurityMode( SecurityMode mode );

    /**
     * Returns the security mode for mounting a specific share.
     *
     * @returns the security mode
     */
    SecurityMode securityMode() const;
#endif

    /**
     * Set the protocol hint for this network item.
     * 
     * @param protocol        The protocol hint
     */
    void setProtocolHint( ProtocolHint protocol );
    
    /**
     * Returns the protocol hint for this item.
     * 
     * @returns the protocol hint
     */
    ProtocolHint protocolHint() const;
    
    /**
     * Sets the useage of Kerberos for this network item.
     * 
     * @param kerberos          Kerberos usage
     */
    void setUseKerberos( Kerberos kerberos );
    
    /**
     * Returns the usage of Kerberos for this network item.
     * 
     * @returns the usage of Kerberos
     */
    Kerberos useKerberos() const;
    
    /**
     * Set the user id you want to use.
     * 
     * @param uid               The user id
     */
    void setUID( K_UID uid );
    
    /**
     * Returns the user id that is to be used.
     * 
     * @returns the user id
     */
    K_UID uid() const;
    
    /**
     * This function returns the name of the owner as defined by the given
     * UID.
     * 
     * @returns the owner's name
     */
    QString owner() const; 
    
    /**
     * Set the group id you want to use.
     * 
     * @param gid               The group id
     */
    void setGID( K_GID gid );
    
    /**
     * Returns the group id that is to be used.
     * 
     * @returns the group id
     */
    K_GID gid() const;
    
    /**
     * This function returns the name of the group as defined by the given
     * GID.
     * 
     * @returns the owner's name
     */
    QString group() const;
    
    /**
     * This function sets the MAC address of a host. In case the options 
     * represent a share this is the MAC address of the host that shares 
     * the resource.
     * 
     * @param macAddress        The MAC address of the host
     */
    void setMACAddress( const QString &macAddress );
    
    /**
     * This function returns the MAC address of the host or an empty string if 
     * no MAC address was defined.
     *
     * @returns the MAC address of the host.
     */
    QString macAddress() const;
    
    /**
     * Set whether a magic WOL package should be send to the host that this 
     * network item represents or where this network item is located before scanning 
     * the entire network.
     * 
     * @param send              Boolean that determines if a magic WOL package
     *                          is to be sent.
     */
    void setWOLSendBeforeNetworkScan( bool send );
    
    /**
     * Send a magic WOL package to the host that this network item represents
     * or where this network item is located before scanning the entire network.
     * 
     * @returns TRUE if a magic WOL package should be send on first network
     * scan.
     */
    bool wolSendBeforeNetworkScan() const;
    
    /**
     * Set whether a magic WOL package should be send to the host that this 
     * network item represents or where this network item is located before a 
     * mount attempt.
     * 
     * @param send              Boolean that determines if a magic WOL package
     *                          is to be sent.
     */
    void setWOLSendBeforeMount( bool send );
    
    /**
     * Send a magic WOL package to the host that this network item represents
     * or where this network item is located before a mount attempt.
     * 
     * @returns TRUE if a magic WOL package should be send on first network
     * scan.
     */
    bool wolSendBeforeMount() const;
    
    /**
     * This function returns all custom options in a sorted map. The UNC,
     * workgroup and IP address must be retrieved separately if needed.
     *
     * Note that all entries that are set and valid are returned here. This
     * also comprises default values (e.g. the default SMB port). If you need 
     * to check if a certain value is a custom option or not, you need to implement 
     * this check.
     *
     * @returns all valid entries.
     */
    QMap<QString,QString> customOptions() const;
    
    /**
     * Check if the custom options @p options are equal to those defined here.
     * 
     * @param options             The options that are to be compared to the
     *                            ones defined here
     */
    bool equals( Smb4KCustomOptions *options ) const;
    
    /**
     * Operator to check if two custom options objects are equal.
     */
    bool operator==( Smb4KCustomOptions options ) const { return equals( &options ); }
    
    /**
     * Check if the custom options are empty.
     * 
     * @returns TRUE if the custom options object is empty
     */
    bool isEmpty();

  private:
    const QScopedPointer<Smb4KCustomOptionsPrivate> d;
};

#endif

