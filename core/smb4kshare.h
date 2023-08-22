/*
    Smb4K's container class for information about a share.

    SPDX-FileCopyrightText: 2008-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSHARE_H
#define SMB4KSHARE_H

// application specific includes
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QHostAddress>
#include <QScopedPointer>
#include <QString>
#include <QStringList>
#include <QtGlobal>

// KDE includes
#include <KUser>

// forward declarations
class Smb4KAuthInfo;
class Smb4KSharePrivate;

class Q_DECL_EXPORT Smb4KShare : public Smb4KBasicNetworkItem
{
    friend class Smb4KSharePrivate;

public:
    /**
     * This constructor takes the URL @p url as argument.
     *
     * @param url           The URL
     */
    explicit Smb4KShare(const QUrl &url);

    /**
     * This is the copy constructor.
     *
     * @param share         The Smb4KShare item that is to be copied
     */
    Smb4KShare(const Smb4KShare &share);

    /**
     * The empty constructor.
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
     * In case of a 'homes' share, this function returns the URL of the user's
     * home repository.
     *
     * If the share is not a 'homes' share or no user name for the homes share
     * has been defined, this function returns an empty string.
     *
     * @returns the user's home repository's URL.
     */
    QUrl homeUrl() const;

    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     *
     * @param showHomesShare  Show the name of the users home share instead of 'homes'
     *                        in case this is a homes share. Setting this argument on a
     *                        non-homes share does nothing.
     *
     * @returns the display string.
     */
    QString displayString(bool showHomeShare = false) const;

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
     * Set the type of the share as reported by the server.
     *
     * @param type          The type of the string
     */
    void setShareType(Smb4KGlobal::ShareType type);

    /**
     * Returns the type of the share as reported by the server. If you are
     * looking for a translated type string, then use the shareTypeString()
     * function.
     *
     * @returns the type of the share.
     */
    Smb4KGlobal::ShareType shareType() const;

    /**
     * Returns the translated type string of the share. You can use this
     * in the GUI.
     *
     * @returns a translated type string.
     */
    QString shareTypeString() const;

    /**
     * Set the IP address of the host where the share is located. @p ip will
     * only be accepted if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The host's IP address
     */
    void setHostIpAddress(const QString &ip);

    /**
     * Set the IP address of the host where the share is located. @p ip will only be accepted
     * if it is compatible with either IPv4 or IPv6.
     *
     * @param ip              The host's IP address
     */
    void setHostIpAddress(const QHostAddress &address);

    /**
     * Returns the IP address of the host. If the IP address was not compatible
     * with IPv4 and IPv6 or if no IP address was supplied, an empty string is
     * returned.
     *
     * @returns the IP address of the host or an empty string.
     */
    QString hostIpAddress() const;

    /**
     * Returns TRUE if the host's IP address is set and FALSE otherwise.
     *
     * @returns TRUE if the host's IP address is set and FALSE otherwise.
     */
    bool hasHostIpAddress() const;

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
     * Returns TRUE if the share is mounted and not accessible by the user.
     *
     * @returns TRUE if the share is mounted and inaccessible.
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
     * Returns TRUE if the share is mounted and is owned by another user.
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
     * @param size           The total disk space
     */
    void setTotalDiskSpace(qint64 size);

    /**
     * Returns the total disk space that is available on the share or 0 if the
     * total disk space was not set or could not be determined.
     *
     * @returns the total disk space or 0.
     */
    qint64 totalDiskSpace() const;

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
     * @param size            The free disk space
     */
    void setFreeDiskSpace(qint64 size);

    /**
     * Returns the free disk space that is available on the share or 0 if the
     * free disk space was not set or could not be determined.
     *
     * @returns the free disk space or 0.
     */
    qint64 freeDiskSpace() const;

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
     * Returns the used disk space that is used on the share or 0 if the
     * used disk space was not set or could not be determined.
     *
     * @returns the used disk space or 0.
     */
    qint64 usedDiskSpace() const;

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
     * Set the user name for the share.
     *
     * @param name              The user name
     */
    void setUserName(const QString &name);

    /**
     * Returns the user name.
     *
     * @returns the user name.
     */
    QString userName() const;

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

    /**
     * Updates the share item if the workgroup name and the UNC of @p share and
     * of this item is equal. Otherwise it does nothing.
     * @param share           The share object that is used to update
     *                        this object
     */
    void update(Smb4KShare *share);

    /**
     * Copy assignment operator
     */
    Smb4KShare &operator=(const Smb4KShare &other);

private:
    const QScopedPointer<Smb4KSharePrivate> d;

    /**
     * Set up the shares icon.
     */
    void setShareIcon();
};

Q_DECLARE_METATYPE(Smb4KShare)

#endif
