/***************************************************************************
    smb4ksambaoptionsinfo  -  This is a container class that carries
    various information of extra options for a specific host.
                             -------------------
    begin                : Mi Okt 18 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

#ifndef SMB4KSAMBAOPTIONSINFO_H
#define SMB4KSAMBAOPTIONSINFO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>
#include <QMap>
#include <QUrl>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KHost;
class Smb4KShare;


/**
 * This class provides a container for all extra options that the user defined
 * for a certain share.
 *
 * @author  Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KSambaOptionsInfo
{
  public:
    /**
     * Type enumeration
     */
    enum Type { Share,
                Host,
                Unknown };

    /**
     * Enumeration for the protocol
     */
    enum Protocol{ Automatic,
                   RPC,
                   RAP,
                   ADS,
                   UndefinedProtocol };

    /**
     * Enumeration for the write access
     */
    enum WriteAccess{ ReadWrite,
                      ReadOnly,
                      UndefinedWriteAccess };

    /**
     * Enumeration for the use of Kerberos
     */
    enum Kerberos{ UseKerberos,
                   NoKerberos,
                   UndefinedKerberos };

    /**
     * Enumeration for use with remounting
     */
    enum Remount{ DoRemount,
                  NoRemount,
                  UndefinedRemount };

    /**
     * Constructor. It takes a Smb4KHost object and extracts all needed (and
     * available) data. All missing information has to be set with the functions
     * provided by this class.
     *
     * @param host              The Smb4KHost object representing the host.
     */
    Smb4KSambaOptionsInfo( Smb4KHost *host );

    /**
     * Constructor. It takes a Smb4KShare object and extracts all needed (and
     * available) data. All missing information has to be set with the functions
     * provided by this class.
     *
     * @param share             The Smb4KShare object representing a share.
     */
    Smb4KSambaOptionsInfo( Smb4KShare *share );

    /**
     * The copy constructor. It takes another Smb4KSambaOptionsInfo object @p info
     * and copies its values.
     *
     * @param info              A Smb4KShareOptionsInfo object
     */
    Smb4KSambaOptionsInfo( const Smb4KSambaOptionsInfo &info );

    /**
     * The empty constructor. You have set all needed values with the functions
     * provided by this class.
     */
    Smb4KSambaOptionsInfo();

    /**
     * The destructor.
     */
    ~Smb4KSambaOptionsInfo();

    /**
     * Sets the "should be remounted" flag.
     *
     * @param remount           A value from the Remount enumeration that defines
     *                          whether a remount should take place or not.
     */
    void setRemount( Smb4KSambaOptionsInfo::Remount remount );

    /**
     * Returns TRUE if the share is to be remounted and FALSE otherwise.
     *
     * @returns a value from the Remount enumeration that determines whether the
     * share should be remounted or not.
     */
    Smb4KSambaOptionsInfo::Remount remount() const { return m_remount; }

    /**
     * This function returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const { return m_url.host().toUpper(); }

    /**
     * This function returns the share name if the item represents a share or an
     * empty string if it is a host item.
     *
     * @returns the share name.
     */
    QString shareName() const;

    /**
     * This function sets the UNC (Uniform Naming Convention string). This
     * has to conform to the following scheme: //[USER@]HOST[:PORT]/SHARE.
     *
     * The UNC may contain the protocol, i.e. "smb://". If a wrong protocol is passed,
     * this function will return immediately without doing anything.
     *
     * @param unc           The UNC of the share
     */
    void setUNC( const QString &unc );

    /**
     * Returns the UNC in the form [smb:]//[USER@]HOST[:PORT]/SHARE depending on
     * the format specified by @p options.
     *
     * @returns the UNC.
     */
    QString unc( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                   QUrl::RemoveUserInfo|
                                                   QUrl::RemovePort ) const;

    /**
     * Returns the host's UNC in the form [smb:]//[USER@]HOST[:PORT] depending on
     * the format specified by @p options.
     *
     * @returns the UNC of the host.
     */
    QString hostUNC( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                       QUrl::RemoveUserInfo|
                                                       QUrl::RemovePort ) const;

    /**
     * This function sets the port that should be used when querying this share.
     *
     * @param port              The port number
     */
    void setPort( int port );

    /**
     * This function returns the port that should be used when working with this
     * share. Please note, that it will be returned as an integer. If no port has been
     * defined, -1 will be returned.
     *
     * @returns                 the port number
     */
    int port() const { return m_url.port(); }

    /**
     * Set the protocol that is to be used with the net command.
     *
     * @param protocol          One item from the Protocol enumeration defining
     *                          the protocol that should be used.
     */
    void setProtocol( Smb4KSambaOptionsInfo::Protocol protocol );

    /**
     * Returns the protocol that is to be used with the net command.
     *
     * @returns the protocol that is to be used with the net command.
     */
    Smb4KSambaOptionsInfo::Protocol protocol() const { return m_protocol; }

    /**
     * Set the 'Use Kerberos' flag.
     *
     * @param kerberos          One item from the Kerberos enueration defining
     *                          if Kerberos should be used or not.
     */
    void setUseKerberos( Smb4KSambaOptionsInfo::Kerberos kerberos );

    /**
     * This functions returns TRUE if the user wants to use Kerberos and
     * otherwise it returns FALSE.
     *
     * @returns                 TRUE if Kerberos should be used and FALSE
     *                          otherwise.
     */
    Smb4KSambaOptionsInfo::Kerberos useKerberos() const { return m_kerberos; }

    /**
     * With this function you can set the UID you want to use for this item.
     * However, it makes only sense with shares.
     *
     * @param uid               The UID
     */
    void setUID( uid_t uid );

    /**
     * This functions returns the UID defined for this item.
     *
     * Please note that the function will always return a UID (by default the
     * user's UID). If you want to check if a UID set, use uidIsSet() function.
     *
     * @returns                 the UID.
     */
    uid_t uid() const { return m_uid; }

    /**
     * This function returns TRUE if the UID was set.
     *
     * @returns TRUE if the UID was set.
     */
    bool uidIsSet() const { return m_uid_set; }

    /**
     * With this function you can set the GID you want to use for this item.
     * However, it makes only sense with shares.
     *
     * @param gid               The GID
     */
    void setGID( gid_t gid );

    /**
     * This functions returns the GID defined for this item.
     *
     * Please note that the function will always return a GID (by default the
     * user's GID). If you want to check if a GID set, use uidIsSet() function.
     *
     * @returns                 the GID.
     */
    gid_t gid() const { return m_gid; }

    /**
     * This function returns TRUE if the GID was set.
     *
     * @returns TRUE if the GID was set.
     */
    bool gidIsSet() const { return m_uid_set; }

    /**
     * This function returns the type of the network item for which the options
     * have been defined.
     *
     * @returns the type according to the Type enumeration.
     */
    Type type() const { return m_type; }

#ifndef __FreeBSD__
    /**
     * Set if the share is to be mounted read-write or read-only.
     *
     * Note: This function is not available und FreeBSD.
     *
     * @param write_access      One of the values of the Type enumeration.
     */
    void setWriteAccess( Smb4KSambaOptionsInfo::WriteAccess write_access );

    /**
     * This functions returns TRUE if the user wants to mount a share read-write
     * otherwise it returns FALSE.
     *
     * Note: This function is not available und FreeBSD.
     *
     * @returns                 TRUE if read-write and FALSE otherwise.
     */
    Smb4KSambaOptionsInfo::WriteAccess writeAccess() const { return m_write_access; }
#endif

    /**
     * Set if this item has custom options. Please note that @p has_options
     * should even be set to FALSE if you have a share that is scheduled for
     * remount, but all other options have default values.
     *
     * @param has_options       Should be TRUE if this option has custom
     *                          options and otherwise FALSE.
     */
    void setHasCustomOptions( bool has_options );

    /**
     * This function returns TRUE if the item has custom options. Please note
     * that it will even return FALSE if the item represents a share that is scheduled
     * for remount, but all other options have default values. So, to check if this
     * item has no custom options and is also not scheduled for remount, you need
     * to do a check like this:
     *
     * @code
     *  ...
     *  if ( item->remount() && item->hasCustomOptions() )
     *  {
     *    // do something
     *  }
     *  ...
     * @endcode
     *
     * @returns                 TRUE if the item has custom options and FALSE otherwise.
     */
    bool hasCustomOptions() const { return m_has_custom_options; }

    /**
     * Set the workgroup for this item.
     *
     * @param workgroup         The workgroup for this item.
     */
    void setWorkgroupName( const QString &workgroup );

    /**
     * This function returns the workgroup of this item.
     *
     * @returns                 the workgroup of this item.
     */
    const QString &workgroupName() const { return m_workgroup; }

    /**
     * Set the IP address for this item. This function checks if the
     * IP address complies either with IPv4 or IPv6 and only sets it
     * if the check passes. In case the IP address is invalid, the
     * internal IP address object is cleared.
     *
     * @param ip                The IP address for this item.
     */
    void setIP( const QString &ip );

    /**
     * This function returns the valid IP address of this item.
     *
     * @returns                 the IP address.
     */
    const QString &ip() const { return m_ip; }

    /**
     * This function updates all values of an Smb4KSambaOptionsInfo object
     * except the UNC, workgroup and IP address.
     *
     * @param info              The object that is used for the update.
     */
    void update( Smb4KSambaOptionsInfo *info );

    /**
     * This function returns all custom options in a sorted map. The UNC,
     * workgroup and IP address must be retrieved separately if needed.
     *
     * Note that all entries that are not defined for a certain item have
     * empty values.
     *
     * @returns all custom entries.
     */
    QMap<QString,QString> entries();

    /**
     * Set the profile for which this custom options are to be used.
     *
     * @param name              The name of the profile.
     */
    void setProfile( const QString &name );

    /**
     * Return the name of the profile for which the options are to be
     * used.
     *
     * @returns the name of the profile for which the options should be
     * used.
     */
    const QString &profile() const { return m_profile; }

  private:
    /**
     * The URL
     */
    QUrl m_url;

    /**
     * The type of this item
     */
    Type  m_type;

    /**
     * Should be remounted?
     */
    Remount m_remount;

#ifndef __FreeBSD__
    /**
     * Mount read-write or read-only?
     */
    WriteAccess m_write_access;
#endif

    /**
     * The protocol
     */
    Protocol m_protocol;

    /**
     * Use Kerberos or not
     */
    Kerberos m_kerberos;

    /**
     * The UID
     */
    uid_t m_uid;

    /**
     * Is UID set?
     */
    bool m_uid_set;

    /**
     * The GID
     */
    gid_t m_gid;

    /**
     * Is GID set?
     */
    bool m_gid_set;

    /**
     * Carries this item custom options?
     */
    bool m_has_custom_options;

    /**
     * The workgroup
     */
    QString m_workgroup;

    /**
     * The IP address
     */
    QString m_ip;

    /**
     * The profile name
     */
    QString m_profile;
};

#endif
