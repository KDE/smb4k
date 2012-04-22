/***************************************************************************
    smb4kshare  -  Smb4K's container class for information about a share.
                             -------------------
    begin                : Mo Jan 28 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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

#ifndef SMB4KSHARE_H
#define SMB4KSHARE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

// KDE includes
#include <kuser.h>
#include <kdemacros.h>

// application specific includes
#include "smb4kbasicnetworkitem.h"

// forward declarations
class Smb4KAuthInfo;

class KDE_EXPORT Smb4KShare : public Smb4KBasicNetworkItem
{
  public:
    /**
     * This constructor takes the host name @p hostName and the name of the
     * shared resource @p name. All other information has to be set by the
     * other functions this class provides.
     *
     * This constructor will also assemble the UNC from the provided arguments.
     *
     * @param hostName      The name of the host where the share is
     *                      located.
     *
     * @param name          The name of the share.
     */
    Smb4KShare( const QString &hostName,
                const QString &name );

    /**
     * This constructor takes the UNC @p unc (in the form [smb:]//[USER@]HOST/SHARE) as
     * only argument. It feeds the internal QUrl object with the string, that extracts
     * all parts from it. All other information has to be set by the other functions
     * this class provides.
     *
     * @param unc           The UNC in the form [smb:]//[USER@]HOST/SHARE.
     */
    Smb4KShare( const QString &unc );

    /**
     * This is the copy constructor. It takes another Smb4KShare item and copies all
     * its values.
     *
     * @param share         The Smb4KShare item that is to be copied
     */
    Smb4KShare( const Smb4KShare &share );

    /**
     * The empty constructor. You need to set all information by the functions that
     * are provided with this class.
     */
    Smb4KShare();

    /**
     * The destructor.
     */
    ~Smb4KShare();

    /**
     * Sets the name of the share (*not* the UNC).
     *
     * @param name          The share name
     */
    void setShareName( const QString &name );

    /**
     * Returns the name of the share, e.g. "Music", "Data", etc.
     *
     * @returns the name of the share.
     */
    QString shareName() const;

    /**
     * Sets the name of the host where the share is located.
     *
     * @param hostName      The host name
     */
    void setHostName( const QString &hostName );

    /**
     * Returns the name of the host where the share is located.
     *
     * @returns the host name
     */
    QString hostName() const { return m_url.host().toUpper(); }

    /**
     * Returns the UNC (Uniform Naming Convention string) address in the form 
     * [smb:]//[USER:PASS@]HOST[:PORT]/SHARE depending on the format specified by 
     * @p options.
     * 
     * Please note that this function returns a modified URL string (uppercase
     * hostname, etc.) and automatically strips a trailing slash if one is present.
     *
     * @returns the UNC.
     */
    QString unc( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                   QUrl::RemoveUserInfo|
                                                   QUrl::RemovePort ) const;
                                                   
    /**
     * In case of a 'homes' share, this function returns the UNC of the user's 
     * home repository in the form [smb:]//[USER@]HOST[:PORT]/USER depending 
     * on the format specified by @p options.
     * 
     * If the share is not a 'homes' share or no user name for the homes share
     * has been defined, this function returns an empty string.
     * 
     * Please note that this function returns a modified URL string (uppercase
     * hostname, etc.) and automatically strips a trailing slash if one is present.
     *
     * @returns the UNC.
     */
    QString homeUNC( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                       QUrl::RemoveUserInfo|
                                                       QUrl::RemovePort ) const;
                                                       
    /**
     * Sets the URL of the share after some checks are passed.
     * 
     * @param url             The URL of the network item
     */
    void setURL( const QUrl &url );
    
    /**
     * Returns the URL of the share.
     * 
     * @returns the URL of the share.
     */
    const QUrl &url() const { return m_url; }
                                                       
    /**
     * In case of a 'homes' share, this function returns the URL of the user's 
     * home repository.
     * 
     * If the share is not a 'homes' share or no user name for the homes share
     * has been defined, this function returns an empty string.
     * 
     * @returns the user's home repository's URL.
     */
    QUrl homeURL() const;

    /**
     * Returns the host's UNC in the form [smb:]//[USER@]HOST[:PORT] depending on
     * the format specified by @p options.
     * 
     * Please note that this function returns a modified URL string (uppercase
     * hostname) and automatically strips a trailing slash if one is present.
     *
     * @returns the UNC of the host.
     */
    QString hostUNC( QUrl::FormattingOptions options = QUrl::RemoveScheme|
                                                       QUrl::RemoveUserInfo|
                                                       QUrl::RemovePort ) const;

    /**
     * Set the workgroup where the host is located that offers this share.
     *
     * @param workgroup     The name of the workgroup
     */
    void setWorkgroupName( const QString &workgroup );

    /**
     * Returns the name of the workgroup where the host is located that
     * offers this share.
     *
     * @returns the workgroup name
     */
    const QString &workgroupName() const { return m_workgroup; }

    /**
     * Sets the type string (Disk, Print, IPC) of the share.
     *
     * @param type          The string defining the type of the share
     */
    void setTypeString( const QString &typeString );

    /**
     * Returns the type string of the share as reported by the server. If you are
     * looking for a translated type string, then use the translatedTypeString()
     * function.
     *
     * @returns the type of the share.
     */
    const QString &typeString() const { return m_type_string; }

    /**
     * Returns the translated type string of the share. You can use this
     * in the GUI.
     *
     * @returns a translated type string.
     */
    const QString translatedTypeString() const;

    /**
     * Sets the comment that was defined for the share.
     *
     * @param comment       The comment for the share
     */
    void setComment( const QString &comment );

    /**
     * Returns the comment that was defined for this share.
     *
     * @returns the comment.
     */
    const QString &comment() const { return m_comment; }

    /**
     * Set the IP address of the host where the share is located. @p ip will
     * only be accepted if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The host's IP address
     */
    void setHostIP( const QString &ip );

    /**
     * Returns the IP address of the host. If the IP address was not compatible
     * with IPv4 and IPv6 or if no IP address was supplied, an empty string is
     * returned.
     *
     * @returns the IP address of the host or an empty string.
     */
    const QString &hostIP() const { return m_host_ip; }

    /**
     * If the share is a hidden one, i.e. it ends with a '$', this function returns
     * TRUE and FALSE otherwise.
     *
     * @returns TRUE if this is a hidden share.
     */
    bool isHidden() const;

    /**
     * If the share is a printer this function returns TRUE and otherwise FALSE.
     *
     * @returns TRUE if the share is a printer.
     */
    bool isPrinter() const;

    /**
     * If the share is an IPC$ share this function returns TRUE and FALSE
     * otherwise.
     *
     * @returns TRUE if the share is an IPC share.
     */
    bool isIPC() const;

    /**
     * If the share is an ADMIN$ share this function returns TRUE and FALSE
     * otherwise.
     *
     * @returns TRUE if the share is an ADMIN share.
     */
    bool isADMIN() const;

    /**
     * Sets the path aka mount point of the share as gathered by the mounter.
     *
     * @param mountpoint      The mount point of the share.
     */
    void setPath( const QString &mountpoint );

    /**
     * Returns the path to the mounted share (aka the mount point) as it was gathered
     * by the mounter. It is a C-type string.
     *
     * @returns the path to the mounted share.
     */
    const QString &path() const { return m_path; }

    /**
     * Returns the canonical path to the mounted share. In contrast to the path()
     * function it will return the absolute path without symlinks. However, should
     * the share be inaccessible (i.e. the isInaccessible() returns TRUE), only
     * the "normal" path is returned.
     *
     * @returns the canonical path to the mounted share.
     */
    const QString canonicalPath() const;

    /**
     * Set @p in to TRUE if the share cannot be accessed by the user. This may be
     * because if strict permissions or because the remote server went offline. By
     * default it is assumed that the share is accessible.
     *
     * @param in              Tells if the share is inaccessible or not.
     */
    void setInaccessible( bool in );

    /**
     * Returns TRUE if the share is not accessible by the user and FALSE otherwise.
     *
     * @returns TRUE if the share is inaccessible.
     */
    bool isInaccessible() const { return m_inaccessible; }

    /**
     * If the share was mounted by another user, @p foreign should be set to TRUE.
     * By default it is assumed that the share is not foreign but owned by the
     * user.
     *
     * @param foreign         TRUE if the share is foreign and FALSE otherwise.
     */
    void setForeign( bool foreign );

    /**
     * Returns TRUE if the share was mounted and is owned by another user.
     *
     * @returns TRUE if this is a foreign share.
     */
    bool isForeign() const { return m_foreign; }

    /**
     * Enumeration for the file system
     */
    enum FileSystem{ CIFS,
                     SMBFS,
                     Unknown };

    /**
     * Set the file system with which the share is mounted.
     *
     * @param filesystem      The file system of the mounted share.
     */
    void setFileSystem( FileSystem filesystem );

    /**
     * Returns the file system of the share. If it wasn't set, FileSystem::Unknown
     * is returned.
     *
     * @returns the file system of the share.
     */
    FileSystem fileSystem() const { return m_filesystem; }

    /**
     * Returns the file system as string in capital letters. If no file system
     * was specified, an empty string is returned.
     *
     * @returns the file system string or an empty string.
     */
    const QString fileSystemString() const;

    /**
     * Sets the UID of the owner of this share.
     *
     * @param uid             The UID of the owner
     */
    void setUID( uid_t uid );

    /**
     * Returns the UID of the owner of this share or the UID of the user, if
     * the UID was not set.
     *
     * @returns the UID of the owner.
     */
    uid_t uid() const { return m_user.uid(); }

    /**
     * Returns the owner's login name. If the owner's UID was not set, the login
     * name of the user is returned.
     *
     * @returns the owner's login name.
     */
    const QString owner() const { return m_user.loginName(); }

    /**
     * Set the owning GID of this share.
     *
     * @param gid             The owning GID
     */
    void setGID( gid_t gid );

    /**
     * Returns the GID of the owner of this share or the GID of the user, if
     * the GID was not set.
     *
     * @returns the GID of the owner.
     */
    gid_t gid() const { return m_group.gid(); }

    /**
     * Returns the owning group name. If the owning GID was not set, the group
     * name of the user is returned.
     *
     * @returns the owning group name.
     */
    const QString group() const { return m_group.name(); }

    /**
     * Sets the value of the total disk space that is available on the share. If 
     * the disk usage could not be determined, @p size has to be set to 0.
     *
     * @param total           The total disk space that is available on the share
     */
    void setTotalDiskSpace( qulonglong size );

    /**
     * Returns the total disk space that is available on the share or 0 if the
     * total disk space was not set or could not be determined.
     *
     * @returns the total disk space or 0.
     */
    qulonglong totalDiskSpace() const { return m_total; }

    /**
     * Returns the total disk space in a human readable form with value and unit
     * (e.g. 10 KiB, 25 MiB, etc.) even if the total disk space was not set or could
     * not be determined. If the value is a valid one, you have to check by evaluating
     * the return value of the isInaccessible() function.
     *
     * @returns the total disk space in a human readable form.
     */
    QString totalDiskSpaceString() const;

    /**
     * Sets the value of the free disk space that is available on the share. If 
     * the free disk space could not be determined, @p size has to be set to 0.
     *
     * @param free            The free disk space that is available on the share
     */
    void setFreeDiskSpace( qulonglong size );

    /**
     * Returns the free disk space that is available on the share or 0 if the
     * free disk space was not set or could not be determined.
     *
     * @returns the free disk space or 0.
     */
    qulonglong freeDiskSpace() const { return m_free; }

    /**
     * Returns the free disk space in a human readable form with value and unit
     * (e.g. 10 KiB, 25 MiB, etc.) even if the free disk space was not set or could
     * not be determined. If the value is a valid one, you have to check by evaluating
     * the return value of the isInaccessible() function.
     *
     * @returns the free disk space in a human readable form.
     */
    QString freeDiskSpaceString() const;
    
    /**
     * Sets the value of the disk space that is used on the share. If the used 
     * disk space could not be determined, @p size has to be set to 0.
     *
     * @param free            The free disk space that is available on the share
     */
    void setUsedDiskSpace( qulonglong size );

    /**
     * Returns the used disk space that is used on the share or 0 if the
     * used disk space was not set or could not be determined.
     *
     * @returns the used disk space or 0.
     */
    qulonglong usedDiskSpace() const { return m_used; }

    /**
     * Returns the used disk space in a human readable form with value and unit
     * (e.g. 10 KiB, 25 MiB, etc.) even if the used disk space was not set or could
     * not be determined. If the value is a valid one, you have to check by evaluating
     * the return value of the isInaccessible() function.
     *
     * @returns the used disk space in a human readable form.
     */
    QString usedDiskSpaceString() const;

    /**
     * Returns the disk usage in percent.
     
     * @returns the disk usage in percent.
     */
    qreal diskUsage() const;

    /**
     * Returns the disk usage in a human readable form with value and unit,
     * for example 3.5 %, 67.0 % etc. If the usage was not set or could not
     * be determined, this funtion returns an empty string.
     *
     * @returns the disk usage in a human readable form or an empty string.
     */
    QString diskUsageString() const;

    /**
     * Enumeration for the checks.
     *
     * @param Full          Full comparison
     *
     * @param NetworkOnly   Only the network related values are compared (except the UNC!).
     *
     * @param LocalOnly     Only those values are compared that are of local importance
     *                      (including the UNC).
     */
    enum CheckFlags{ Full,
                     NetworkOnly,
                     LocalOnly };

    /**
     * Compare another Smb4KShare object with this one an return TRUE if both carry
     * the same data. Depending on the value of @p flag either all data will be compared
     * or only the one that is network related or the one that is of local importance.
     *
     * Please note, that the UNC won't be checked with the flag CheckFlags::NetworkOnly,
     * but only with either CheckFlags::Full or CheckFlags::LocalOnly.
     *
     * @param share           The Smb4KShare object that should be compared with this
     *                        one.
     *
     * @param flag            The flag that determines which values are compared.
     *
     * @returns TRUE if the data that was compared is the same.
     */
    bool equals( Smb4KShare *share,
                 CheckFlags flag );
                 
    /**
     * Operator to check if two shares are equal. This operator performs a full check.
     */
    bool operator==( Smb4KShare share ) const { return equals( &share, Full ); }

    /**
     * Returns TRUE if values that were checked according to @p flag are empty.
     * Modified booleans are not considered.
     *
     * Please note, that the UNC won't be checked with the flag CheckFlags::NetworkOnly,
     * but only with either CheckFlags::Full or CheckFlags::LocalOnly.
     *
     * @param flag            The flag that determines which values are checked.
     *
     * @returns TRUE if the item is empty.
     */
    bool isEmpty( CheckFlags flag = Full ) const;

    /**
     * If this share was mounted, set @p mounted to TRUE. This function will not
     * work with printer shares.
     *
     * @param mounted         Should be set to TRUE if the share was mounted.
     */
    void setIsMounted( bool mounted );

    /**
     * This function returns TRUE if the share has been mounted and FALSE
     * otherwise.
     *
     * @returns TRUE if this share was mounted.
     */
    bool isMounted() const { return m_is_mounted; }

    /**
     * This convenience function sets the mount related data. It is copied
     * from @p share.
     *
     * @param share           The share object from where the mount related
     *                        data should be copied.
     */
    void setMountData( Smb4KShare *share );

    /**
     * This convenience function resets the mount related data (mount point,
     * file system, etc.).
     */
    void resetMountData();

    /**
     * Returns TRUE if the share is or *was* a 'homes' share. That means that
     * this value is not changed when the share name is changed.
     *
     * @returns TRUE if this is or *was* a 'homes' share and FALSE otherwise.
     */
    bool isHomesShare() const;

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
    int port() const { return m_url.port(); }

    /**
     * Set the authentication information for the share. This function will add
     * the authentication information to the URL of the share. Any previous
     * user information including the login will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo( Smb4KAuthInfo *authInfo );
    
    /**
     * Set the login for the share. This function will add the login name
     * to the URL of the share.
     * 
     * @param login       The login name
     */
    void setLogin( const QString &login );
    
    /**
     * Returns the login.
     *
     * @returns the login.
     */
    QString login() const { return m_url.userName(); }
    
    /**
     * Set the password used for authentication.
     * 
     * @param passwd              The password
     */
    void setPassword( const QString &passwd );
    
    /**
     * Returns the password.
     * 
     * @returns the password.
     */
    QString password() const { return m_url.password(); }
    
    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     * 
     * @returns TRUE if the host's IP address is set and FALSE otherwise.
     */
    bool hasHostIP() const { return !m_host_ip.isEmpty(); }

  private:
    /**
     * The URL
     */
    QUrl m_url;
    
    /**
     * Set up the shares icon.
     */
    void setShareIcon();
    
    /**
     * The workgroup
     */
    QString m_workgroup;

    /**
     * The type of the share
     */
    QString m_type_string;

    /**
     * The comment
     */
    QString m_comment;

    /**
     * The host's IP address
     */
    QString m_host_ip;

    /**
     * The path to the mounted share
     */
    QString m_path;

    /**
     * Determines if the share is inaccessible
     */
    bool m_inaccessible;

    /**
     * Determines if the share is foreign
     */
    bool m_foreign;

    /**
     * The file system
     */
    FileSystem m_filesystem;

    /**
     * The UID of the owner
     */
    KUser m_user;

    /**
     * The owning GID
     */
    KUserGroup m_group;

    /**
     * The total disk space
     */
    qulonglong m_total;

    /**
     * The free disk space
     */
    qulonglong m_free;
    
    /**
     * The used disk space
     */
    qulonglong m_used;

    /**
     * Is mounted
     */
    bool m_is_mounted;

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
