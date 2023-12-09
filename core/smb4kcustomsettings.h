/*
    This class carries custom settings

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCUSTOMOPTIONS_H
#define SMB4KCUSTOMOPTIONS_H

// application specific includes
#include "smb4kbasicnetworkitem.h"
#include "smb4kglobalenums.h"

// Qt includes
#include <QScopedPointer>
#include <QUrl>

// KDE includes
#include <KUser>

// forward declarations
class Smb4KCustomSettingsPrivate;

using namespace Smb4KGlobal;

/**
 * This class stores the custom settings defined for a certain host
 * or share.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class Q_DECL_EXPORT Smb4KCustomSettings
{
    friend class Smb4KCustomSettingsPrivate;

public:
    /**
     * Constructor fora network item
     */
    explicit Smb4KCustomSettings(Smb4KBasicNetworkItem *networkItem);

    /**
     * Copy constructor
     */
    Smb4KCustomSettings(const Smb4KCustomSettings &options);

    /**
     * Empty constructor
     */
    Smb4KCustomSettings();

    /**
     * Destructor
     */
    ~Smb4KCustomSettings();

    /**
     * Set the network item. If you already set a network item before,
     * this function will do nothing.
     *
     * @param networkItem   The network item
     */
    void setNetworkItem(Smb4KBasicNetworkItem *networkItem) const;

    /**
     * Returns the type of the network item for that the options
     * are defined
     *
     * @returns the type of the network item
     */
    Smb4KGlobal::NetworkItem type() const;

    /**
     * Sets the workgroup name.
     *
     * @param workgroup       The workgroup name
     */
    void setWorkgroupName(const QString &workgroup) const;

    /**
     * Returns the workgroup name.
     *
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Sets the URL of the network item
     *
     * @param url             The URL
     */
    void setUrl(const QUrl &url) const;

    /**
     * Returns the URL of the network item
     *
     * @returns the URL
     */
    QUrl url() const;

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
    void setIpAddress(const QString &ip) const;

    /**
     * Returns the IP address of the network item
     *
     * @returns the IP address
     */
    QString ipAddress() const;

    /**
     * Returns TRUE if the IP address is set and FALSE otherwise.
     *
     * @returns TRUE if the IP address is known.
     */
    bool hasIpAddress() const;

    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

    /**
     * Remount enumeration
     *
     * @param RemountOnce       Remount the share only next time the application
     *                          is started.
     * @param RemountAlways     Remount the share every time the application is
     *                          started.
     * @param UndefinedRemount  No remount behavior is undefined.
     */
    enum Remount { RemountOnce, RemountAlways, UndefinedRemount };

    /**
     * If the network item is of type Share, set if it should be remounted.
     * If the network item is not of type Share, this function does nothing.
     *
     * @param remount       One entry of the Remount enumeration
     */
    void setRemount(int remount) const;

    /**
     * Returns if the network item should be remounted.
     *
     * @returns if the network item should be remounted.
     */
    int remount() const;

    /**
     * Set if the information about the user that is to be owner of the share
     * should be used when mounting or not.
     *
     * @param usage            Boolean that determines whether the uid should be
     *                          used.
     */
    void setUseUser(bool use) const;

    /**
     * Use the information about the user that is to be owner of the share.
     *
     * @returns TRUE if the uid should be used when mounting.
     */
    bool useUser() const;

    /**
     * Set the user who owns the share.
     * @param user    The user
     */
    void setUser(const KUser &user) const;

    /**
     * Returns the user who owns the share.
     * @returns the user
     */
    KUser user() const;

    /**
     * Set if the information about the group that is to be owner of the share
     * should be used when mounting or not.
     *
     * @param use      Boolean that determines whether the gid should be used.
     */
    void setUseGroup(bool use) const;

    /**
     * Use the information about the group that is to be owner of the share.
     *
     * @returns TRUE if the gid should be used when mounting.
     */
    bool useGroup() const;

    /**
     * Set the group that owns the share.
     *
     * @param group   The group
     */
    void setGroup(const KUserGroup &group) const;

    /**
     * Returns the group that owns the share.
     *
     * @returns the group
     */
    KUserGroup group() const;

    /**
     * Set if the file mode should be used.
     *
     * @param use     Boolean that determines whether the file mode should be used.
     */
    void setUseFileMode(bool use) const;

    /**
     * Returns if the file mode should be used.
     *
     * @return TRUE if the file mode should be used
     */
    bool useFileMode() const;

    /**
     * Set the file mode that should be used. The value must be defined in octal.
     *
     * @param mode    The file mode
     */
    void setFileMode(const QString &mode) const;

    /**
     * Returns the file mode that should be used. The value is returned in octal.
     *
     * @returns the file mode
     */
    QString fileMode() const;

    /**
     * Set if the directory mode should be used.
     *
     * @param use     Boolean that determines whether the directory mode should be used.
     */
    void setUseDirectoryMode(bool use) const;

    /**
     * Returns if the directory mode should be used.
     *
     * @return TRUE if the file directory should be used
     */
    bool useDirectoryMode() const;

    /**
     * Set the directory mode that should be used. The value must be defined in octal.
     *
     * @param mode    The directory mode
     */
    void setDirectoryMode(const QString &mode) const;

    /**
     * Returns the directory mode that should be used. The value is returned in octal.
     *
     * @returns the directory mode
     */
    QString directoryMode() const;

#if defined(Q_OS_LINUX)
    /**
     * Set if the server supports the CIFS Unix Extensions. If this setting is set,
     * the parameters that are not needed in the case of support are cleared.
     *
     * @param supports          Boolean that determines if the server supports
     *                          the CIFS Unix extensions.
     */
    void setCifsUnixExtensionsSupport(bool support) const;

    /**
     * The server supports the CIFS Unix extensions.
     *
     * @returns TRUE if the server supports the CIFS Unix Extensions.
     */
    bool cifsUnixExtensionsSupport() const;

    /**
     * Set if the filesystem port should be used
     *
     * @param use             Boolean that determines if the filesystem port should
     *                        be used
     */
    void setUseFileSystemPort(bool use) const;

    /**
     * Returns if the filesystem port should be used.
     *
     * @returns TRUE if the filesystem port should be used
     */
    bool useFileSystemPort() const;

    /**
     * Set the port that is to be used with mounting for a single share or all
     * shares of a host.
     *
     * @param port            The file system port
     */
    void setFileSystemPort(int port) const;

    /**
     * Returns the file system port. The default value is 445.
     *
     * @returns the file system port
     */
    int fileSystemPort() const;

    /**
     * Set if the SMB protocol version for mounting should be set.
     *
     * @param use             Boolean that determines if the SMB protocol version
     *                        for mounting should be set
     */
    void setUseMountProtocolVersion(bool use) const;

    /**
     * Returns if the SMB protocol version for mounting should be set.
     *
     * @returns TRUE if the SMB protocol version for mounting should be set.
     */
    bool useMountProtocolVersion() const;

    /**
     * Set the SMB protocol version for mounting.
     *
     * @param version         The protocol version used for mounting
     */
    void setMountProtocolVersion(int version) const;

    /**
     * Returns the SMB protocol version for mounting.
     *
     * @returns the SMB protocol version
     */
    int mountProtocolVersion() const;

    /**
     * Set if the security mode should be used.
     *
     * @param use             Boolean that determines if the security mode should
     *                        be used
     */
    void setUseSecurityMode(bool use) const;

    /**
     * Returns if the security mode should be used
     *
     * @returns TRUE if the security mode should be used
     */
    bool useSecurityMode() const;

    /**
     * Set the security mode for mounting.
     *
     * @param mode            The security mode
     */
    void setSecurityMode(int mode) const;

    /**
     * Returns the security mode for mounting a specific share.
     *
     * @returns the security mode
     */
    int securityMode() const;

    /**
     * Set if the write access setting should be used.
     *
     * @param use             Boolean that determines if the write access setting
     *                        should be used
     */
    void setUseWriteAccess(bool use) const;

    /**
     * Returns if the write access setting should be used
     *
     * @returns TRUE if the write access setting should be used
     */
    bool useWriteAccess() const;

    /**
     * Set the write access for either a single share or all shares of a host.
     *
     * @param access          The write access
     */
    void setWriteAccess(int access) const;

    /**
     * Returns the write access for the share.
     *
     * @returns the write access
     */
    int writeAccess() const;
#endif

    /**
     * Set the profile this custom settings object belongs to. The profile is
     * meant to distinguish between several network environments, like home
     * and work.
     *
     * @param profile         The profile name
     */
    void setProfile(const QString &profile) const;

    /**
     * Returns the name of the profile this custom settings object belongs to.
     *
     * @returns the profile name
     */
    QString profile() const;

    /**
     * Set whether the client minimal and maximal protocol versions should
     * be set/used.
     *
     * @param use             TRUE, if the protocol versions should used
     */
    void setUseClientProtocolVersions(bool use) const;

    /**
     * Returns whether the client minimal and maximal protocol versions
     * should be set/used.
     *
     * @returns TRUE if the client protocol versions should be set/used.
     */
    bool useClientProtocolVersions() const;

    /**
     * Set the minimal client protocol version to use.
     *
     * @param version         The SMB protocol version
     */
    void setMinimalClientProtocolVersion(int version) const;

    /**
     * Return the minimal client protocol version to use.
     *
     * @returns the minimal client protocol version to use.
     */
    int minimalClientProtocolVersion() const;

    /**
     * Set the maximal client protocol version to use.
     *
     * @param version         The SMB protocol version
     */
    void setMaximalClientProtocolVersion(int version) const;

    /**
     * Return the maximal client protocol version to use.
     *
     * @returns the maximal client protocol version to use.
     */
    int maximalClientProtocolVersion() const;

    /**
     * Set whether the SMB port should be used.
     *
     * @param use             True, if the SMB port should be used
     */
    void setUseSmbPort(bool use) const;

    /**
     * Returns whether the SMB port should be used.
     *
     * @returns TRUE if the SMB port should be used.
     */
    bool useSmbPort() const;

    /**
     * Set the SMB port to use with this host or share.
     *
     * @param port            The SMB port
     */
    void setSmbPort(int port) const;

    /**
     * Returns the SMB port. The default value is 139.
     *
     * @returns the SMB port
     */
    int smbPort() const;

    /**
     * Sets the useage of Kerberos for this network item.
     *
     * @param use               Boolean that determines the useage of Kerberos
     */
    void setUseKerberos(bool use) const;

    /**
     * Returns the usage of Kerberos for this network item.
     *
     * @returns the usage of Kerberos
     */
    bool useKerberos() const;

    /**
     * This function sets the MAC address of a host. In case the options
     * represent a share this is the MAC address of the host that shares
     * the resource.
     *
     * @param macAddress        The MAC address of the host
     */
    void setMACAddress(const QString &macAddress) const;

    /**
     * This function returns the MAC address of the host or an empty string if
     * no MAC address was defined.
     *
     * @returns the MAC address of the host.
     */
    QString macAddress() const;

    /**
     * Set whether a magic WOL packet should be send to the host that this
     * network item represents or where this network item is located before scanning
     * the entire network.
     *
     * @param send              Boolean that determines if a magic WOL packet
     *                          is to be sent.
     */
    void setWOLSendBeforeNetworkScan(bool send) const;

    /**
     * Send a magic WOL packet to the host that this network item represents
     * or where this network item is located before scanning the entire network.
     *
     * @returns TRUE if a magic WOL packet should be send on first network
     * scan.
     */
    bool wolSendBeforeNetworkScan() const;

    /**
     * Set whether a magic WOL packet should be send to the host that this
     * network item represents or where this network item is located before a
     * mount attempt.
     *
     * @param send              Boolean that determines if a magic WOL packet
     *                          is to be sent.
     */
    void setWOLSendBeforeMount(bool send) const;

    /**
     * Send a magic WOL packet to the host that this network item represents
     * or where this network item is located before a mount attempt.
     *
     * @returns TRUE if a magic WOL packet should be send on first network
     * scan.
     */
    bool wolSendBeforeMount() const;

    /**
     * This function returns all custom settings in a sorted map. The URL,
     * workgroup and IP address must be retrieved separately if needed.
     *
     * Note that all entries that are set and valid are returned here. This
     * also comprises default values (e.g. the default SMB port). If you need
     * to check if a certain value is a custom option or not, you need to implement
     * this check.
     *
     * @returns all valid entries.
     */
    QMap<QString, QString> customSettings() const;

    /**
     * Check if there are options defined. If @p withoutRemountOnce is set,
     * this function will ignore the setting Smb4KCustomSettings::RemountOnce.
     *
     * @returns TRUE if there are options defined and FALSE otherwise
     */
    bool hasOptions(bool withoutRemountOnce = false) const;

    /**
     * Update this custom settings object. You cannot change the workgroup,
     * URL and type with this function.
     *
     * @param options             The options that are used to update this object
     */
    void update(Smb4KCustomSettings *options);

    /**
     * Copy assignment operator
     */
    Smb4KCustomSettings &operator=(const Smb4KCustomSettings &other);

private:
    QScopedPointer<Smb4KCustomSettingsPrivate> d;
};

Q_DECLARE_METATYPE(Smb4KCustomSettings)

#endif
