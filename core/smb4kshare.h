/***************************************************************************
    smb4kshare  -  Smb4K's container class for information about a share.
                             -------------------
    begin                : Mo Jan 28 2008
    copyright            : (C) 2008-2016 by Alexander Reinholdt
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

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QString>
#include <QStringList>
#include <QtGlobal>
#include <QScopedPointer>
#include <QUrl>

// KDE includes
#include <KCoreAddons/KUser>

// forward declarations
class Smb4KAuthInfo;
class Smb4KSharePrivate;

class Q_DECL_EXPORT Smb4KShare : public Smb4KBasicNetworkItem
{
  friend class Smb4KSharePrivate;
  
  public:
    /**
     * This constructor takes the host name @p hostName and the name of the
     * shared resource @p shareName. All other information has to be set by the
     * other functions this class provides.
     *
     * This constructor will also assemble the UNC from the provided arguments.
     *
     * @param hostName      The name of the host where the share is
     *                      located.
     *
     * @param shareName     The name of the share.
     */
    Smb4KShare(const QString &hostName, const QString &shareName);

    /**
     * This constructor takes the UNC @p unc (in the form [smb:]//[USER@]HOST/SHARE) as
     * only argument. It feeds the internal url object with the string, that extracts
     * all parts from it. All other information has to be set by the other functions
     * this class provides.
     *
     * @param unc           The UNC in the form [smb:]//[USER@]HOST/SHARE.
     */
    explicit Smb4KShare(const QString &unc);

    /**
     * This is the copy constructor. It takes another Smb4KShare item and copies all
     * its values.
     *
     * @param share         The Smb4KShare item that is to be copied
     */
    Smb4KShare(const Smb4KShare &share);

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
    void setShareName(const QString &name);

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
    void setHostName(const QString &hostName);

    /**
     * Returns the name of the host where the share is located.
     *
     * @returns the host name
     */
    QString hostName() const;

    /**
     * Returns the UNC (Uniform Naming Convention string) in the form //HOST/Share.
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
     * In case of a 'homes' share, this function returns the UNC (Uniform 
     * Naming Convention string) of the user's home repository in the form 
     * //HOST/User.
     * 
     * If the share is not a 'homes' share or no user name for the homes share
     * has been defined, this function returns an empty string.
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
    QString homeUNC() const;
    
    /**
     * Sets the URL of the share after some checks are passed.
     * 
     * @param url             The URL of the network item
     */
    void setURL(const QUrl &url);
    
    /**
     * Returns the URL of the share.
     * 
     * @returns the URL of the share.
     */
    QUrl url() const;
                                                       
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
     * Returns the host's UNC (Uniform Naming Convention string) in the form //HOST.
     * 
     * This function should only be used for basic comparisons or for display
     * purposes. If you need to do sophisticated comparisons, use the url() 
     * function instead.
     * 
     * Please note that this function returns a modified URL string (uppercase
     * hostname) and automatically strips a trailing slash if one is present.
     *
     * @returns the UNC of the host.
     */
    QString hostUNC() const;
    
    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

    /**
     * Set the workgroup where the host is located that offers this share.
     *
     * @param workgroup     The name of the workgroup
     */
    void setWorkgroupName(const QString &workgroup);

    /**
     * Returns the name of the workgroup where the host is located that
     * offers this share.
     *
     * @returns the workgroup name
     */
    QString workgroupName() const;

    /**
     * Sets the type string (Disk, Print, IPC) of the share.
     *
     * @param type          The string defining the type of the share
     */
    void setTypeString(const QString &typeString);

    /**
     * Returns the type string of the share as reported by the server. If you are
     * looking for a translated type string, then use the translatedTypeString()
     * function.
     *
     * @returns the type of the share.
     */
    QString typeString() const;

    /**
     * Returns the translated type string of the share. You can use this
     * in the GUI.
     *
     * @returns a translated type string.
     */
    QString translatedTypeString() const;

    /**
     * Sets the comment that was defined for the share.
     *
     * @param comment       The comment for the share
     */
    void setComment(const QString &comment);

    /**
     * Returns the comment that was defined for this share.
     *
     * @returns the comment.
     */
    QString comment() const;

    /**
     * Set the IP address of the host where the share is located. @p ip will
     * only be accepted if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The host's IP address
     */
    void setHostIP(const QString &ip);

    /**
     * Returns the IP address of the host. If the IP address was not compatible
     * with IPv4 and IPv6 or if no IP address was supplied, an empty string is
     * returned.
     *
     * @returns the IP address of the host or an empty string.
     */
    QString hostIP() const;
    
    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     * 
     * @returns TRUE if the host's IP address is set and FALSE otherwise.
     */
    bool hasHostIP() const;

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
     * Sets the path aka mount point of the share as gathered by the mounter.
     *
     * @param mountpoint      The mount point of the share.
     */
    void setPath(const QString &mountpoint);

    /**
     * Returns the path to the mounted share (aka the mount point) as it was gathered
     * by the mounter. It is a C-type string.
     *
     * @returns the path to the mounted share.
     */
    QString path() const;

    /**
     * Returns the canonical path to the mounted share. In contrast to the path()
     * function it will return the absolute path without symlinks. However, should
     * the share be inaccessible (i.e. the isInaccessible() returns TRUE), only
     * the "normal" path is returned.
     *
     * @returns the canonical path to the mounted share.
     */
    QString canonicalPath() const;

    /**
     * Set @p in to TRUE if the share cannot be accessed by the user. This may be
     * because if strict permissions or because the remote server went offline. By
     * default it is assumed that the share is accessible.
     *
     * @param in              Tells if the share is inaccessible or not.
     */
    void setInaccessible(bool in);

    /**
     * Returns TRUE if the share is not accessible by the user and FALSE otherwise.
     *
     * @returns TRUE if the share is inaccessible.
     */
    bool isInaccessible() const;

    /**
     * If the share was mounted by another user, @p foreign should be set to TRUE.
     * By default it is assumed that the share is not foreign but owned by the
     * user.
     *
     * @param foreign         TRUE if the share is foreign and FALSE otherwise.
     */
    void setForeign(bool foreign);

    /**
     * Returns TRUE if the share was mounted and is owned by another user.
     *
     * @returns TRUE if this is a foreign share.
     */
    bool isForeign() const;

    /**
     * Returns the file system as string in capital letters. If no file system
     * was specified, an empty string is returned.
     *
     * @returns the file system string or an empty string.
     */
    QString fileSystemString() const;

    /**
     * Sets the owner of this share.
     * @param user             The UID of the owner
     */
    void setUser(const KUser &user);

    /**
     * Returns the owner of this share or the current user, if
     * the owner was not set using @see setUser().
     * @returns the owner.
     */
    KUser user() const;
    
    /**
     * Set the group that owns this share.
     * @param group            The owning GID
     */
    void setGroup(const KUserGroup &group);

    /**
     * Returns the group that owns this share or the current group, if
     * the group was not set using @see setGroup().
     * @returns the group.
     */
    KUserGroup group() const;

    /**
     * Sets the value of the total disk space that is available on the share. If 
     * the disk usage could not be determined, @p size has to be set to 0.
     *
     * @param total           The total disk space that is available on the share
     */
    void setTotalDiskSpace(qulonglong size);

    /**
     * Returns the total disk space that is available on the share or 0 if the
     * total disk space was not set or could not be determined.
     *
     * @returns the total disk space or 0.
     */
    qulonglong totalDiskSpace() const;

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
    void setFreeDiskSpace(qulonglong size);

    /**
     * Returns the free disk space that is available on the share or 0 if the
     * free disk space was not set or could not be determined.
     *
     * @returns the free disk space or 0.
     */
    qulonglong freeDiskSpace() const;
    
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
    void setUsedDiskSpace(qulonglong size);

    /**
     * Returns the used disk space that is used on the share or 0 if the
     * used disk space was not set or could not be determined.
     *
     * @returns the used disk space or 0.
     */
    qulonglong usedDiskSpace() const;

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
     * be determined, this function returns an empty string.
     *
     * @returns the disk usage in a human readable form or an empty string.
     */
    QString diskUsageString() const;

    /**
     * Enumeration for the checks.
     *
     * @enum Full               Full comparison
     * @enum NetworkOnly        Only the network related values are compared
     * @enum MinimalNetworkOnly Only the absolutely necessary network related values
     *                          are compared
     * @enum LocalOnly          Only those values are compared that are of local 
     *                          importance
     * @enum MinimalLocalOnly   Only those values are compared that have local importance
     *                          and are absolutely necessary for a comparison
     */
    enum CheckFlags{ Full,
                     NetworkOnly,
                     MinimalNetworkOnly,
                     LocalOnly,
                     MinimalLocalOnly };

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
    bool equals(Smb4KShare *share, CheckFlags flag) const;
                 
    /**
     * Operator to check if two shares are equal. This operator performs a full check.
     */
    bool operator==(Smb4KShare share) const { return equals(&share, Full); }

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
    bool isEmpty(CheckFlags flag = Full) const;

    /**
     * If this share was mounted, set @p mounted to TRUE. This function will not
     * work with printer shares.
     *
     * @param mounted         Should be set to TRUE if the share was mounted.
     */
    void setMounted(bool mounted);

    /**
     * This function returns TRUE if the share has been mounted and FALSE
     * otherwise.
     *
     * @returns TRUE if this share was mounted.
     */
    bool isMounted() const;

    /**
     * This convenience function sets the mount related data that is copied
     * from @p share.
     *
     * @param share           The share object from where the mount related
     *                        data should be copied.
     */
    void setMountData(Smb4KShare *share);
    
    /**
     * This convenience function resets the mount related data.
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
    void setPort(int port);

    /**
     * Returns the port that is used in the UNC.
     *
     * @returns the port.
     */
    int port() const;

    /**
     * Set the authentication information for the share. This function will add
     * the authentication information to the URL of the share. Any previous
     * user information including the login will be overwritten.
     *
     * @param authInfo    The authentication information
     */
    void setAuthInfo(Smb4KAuthInfo *authInfo);
    
    /**
     * Set the login for the share. This function will add the login name
     * to the URL of the share.
     * 
     * @param login       The login name
     */
    void setLogin(const QString &login);
    
    /**
     * Returns the login.
     *
     * @returns the login.
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
    /**
     * Set up the shares icon.
     */
    void setShareIcon();

    const QScopedPointer<Smb4KSharePrivate> d;
};

#endif
