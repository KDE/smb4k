/*
    This class carries custom settings

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcustomsettings.h"
#include "smb4khost.h"
#include "smb4ksettings.h"
#include "smb4kshare.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QDebug>
#include <QHostAddress>
#include <QRegularExpression>

// KDE includes
#include <KLocalizedString>

class Smb4KCustomSettingsPrivate
{
public:
    QString workgroup;
    QUrl url;
    QHostAddress ip;
    NetworkItem type;
    int remount;
    bool useUser;
    KUser user;
    bool useGroup;
    KUserGroup group;
    bool useFileMode;
    QString fileMode;
    bool useDirectoryMode;
    QString directoryMode;
#if defined(Q_OS_LINUX)
    bool cifsUnixExtensionsSupport;
    bool useFileSystemPort;
    int fileSystemPort;
    bool useMountProtocolVersion;
    int mountProtocolVersion;
    bool useSecurityMode;
    int securityMode;
    bool useWriteAccess;
    int writeAccess;
#endif
    QString profile;
    bool useClientProtocolVersions;
    int minimalClientProtocolVersion;
    int maximalClientProtocolVersion;
    bool useSmbPort;
    int smbPort;
    bool useKerberos;
    QString macAddress;
    bool wakeOnLanBeforeFirstScan;
    bool wakeOnLanBeforeMount;
};

Smb4KCustomSettings::Smb4KCustomSettings(Smb4KBasicNetworkItem *networkItem)
    : d(new Smb4KCustomSettingsPrivate)
{
    d->type = networkItem->type();
    d->url = networkItem->url();
    d->remount = UndefinedRemount;
    d->useFileMode = Smb4KMountSettings::useFileMode();
    d->fileMode = Smb4KMountSettings::fileMode();
    d->useDirectoryMode = Smb4KMountSettings::useDirectoryMode();
    d->directoryMode = Smb4KMountSettings::directoryMode();
#if defined(Q_OS_LINUX)
    d->cifsUnixExtensionsSupport = Smb4KMountSettings::cifsUnixExtensionsSupport();
    d->useMountProtocolVersion = Smb4KMountSettings::useSmbProtocolVersion();
    d->mountProtocolVersion = Smb4KMountSettings::smbProtocolVersion();
    d->useSecurityMode = Smb4KMountSettings::useSecurityMode();
    d->securityMode = Smb4KMountSettings::securityMode();
    d->useWriteAccess = Smb4KMountSettings::useWriteAccess();
    d->writeAccess = Smb4KMountSettings::writeAccess();
#endif

    switch (d->type) {
    case Host: {
        Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

        if (host) {
            d->workgroup = host->workgroupName();
            d->ip.setAddress(host->ipAddress());
            d->useUser = Smb4KMountSettings::useUserId();
            d->user = KUser((K_UID)Smb4KMountSettings::userId().toInt());
            d->useGroup = Smb4KMountSettings::useGroupId();
            d->group = KUserGroup((K_GID)Smb4KMountSettings::groupId().toInt());
            d->useSmbPort = Smb4KSettings::useRemoteSmbPort();
            d->smbPort = host->port() != -1 ? host->port() : Smb4KSettings::remoteSmbPort();
#if defined(Q_OS_LINUX)
            d->useFileSystemPort = Smb4KMountSettings::useRemoteFileSystemPort();
            d->fileSystemPort = Smb4KMountSettings::remoteFileSystemPort();
#endif
        }
        break;
    }
    case Share: {
        Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

        if (share) {
            d->workgroup = share->workgroupName();
            d->ip.setAddress(share->hostIpAddress());
            d->useUser = Smb4KMountSettings::useUserId();
            d->user = share->user();
            d->useGroup = Smb4KMountSettings::useGroupId();
            d->group = share->group();
            d->useSmbPort = Smb4KSettings::useRemoteSmbPort();
            d->smbPort = Smb4KSettings::remoteSmbPort();
#if defined(Q_OS_LINUX)
            d->useFileSystemPort = Smb4KMountSettings::useRemoteFileSystemPort();
            d->fileSystemPort = share->port() != -1 ? share->port() : Smb4KMountSettings::remoteFileSystemPort();
#endif
        }
        break;
    }
    default: {
        break;
    }
    }

    d->useClientProtocolVersions = Smb4KSettings::useClientProtocolVersions();
    d->minimalClientProtocolVersion = Smb4KSettings::minimalClientProtocolVersion();
    d->maximalClientProtocolVersion = Smb4KSettings::maximalClientProtocolVersion();
    d->useKerberos = Smb4KSettings::useKerberos();
    d->wakeOnLanBeforeFirstScan = false;
    d->wakeOnLanBeforeMount = false;
}

Smb4KCustomSettings::Smb4KCustomSettings(const Smb4KCustomSettings &o)
    : d(new Smb4KCustomSettingsPrivate)
{
    *d = *o.d;
}

Smb4KCustomSettings::Smb4KCustomSettings()
    : d(new Smb4KCustomSettingsPrivate)
{
    d->type = UnknownNetworkItem;
    d->remount = UndefinedRemount;
    d->useUser = Smb4KMountSettings::useUserId();
    d->user = KUser((K_UID)Smb4KMountSettings::userId().toInt());
    d->useGroup = Smb4KMountSettings::useGroupId();
    d->group = KUserGroup((K_GID)Smb4KMountSettings::groupId().toInt());
    d->useFileMode = Smb4KMountSettings::useFileMode();
    d->fileMode = Smb4KMountSettings::fileMode();
    d->useDirectoryMode = Smb4KMountSettings::useDirectoryMode();
    d->directoryMode = Smb4KMountSettings::directoryMode();
#if defined(Q_OS_LINUX)
    d->cifsUnixExtensionsSupport = Smb4KMountSettings::cifsUnixExtensionsSupport();
    d->useFileSystemPort = Smb4KMountSettings::useRemoteFileSystemPort();
    d->fileSystemPort = Smb4KMountSettings::remoteFileSystemPort();
    d->useMountProtocolVersion = Smb4KMountSettings::useSmbProtocolVersion();
    d->mountProtocolVersion = Smb4KMountSettings::smbProtocolVersion();
    d->useSecurityMode = Smb4KMountSettings::useSecurityMode();
    d->securityMode = Smb4KMountSettings::securityMode();
    d->useWriteAccess = Smb4KMountSettings::useWriteAccess();
    d->writeAccess = Smb4KMountSettings::writeAccess();
#endif
    d->useClientProtocolVersions = Smb4KSettings::useClientProtocolVersions();
    d->minimalClientProtocolVersion = Smb4KSettings::minimalClientProtocolVersion();
    d->maximalClientProtocolVersion = Smb4KSettings::maximalClientProtocolVersion();
    d->useSmbPort = Smb4KSettings::useRemoteSmbPort();
    d->smbPort = Smb4KSettings::remoteSmbPort();
    d->useKerberos = Smb4KSettings::useKerberos();
    d->wakeOnLanBeforeFirstScan = false;
    d->wakeOnLanBeforeMount = false;
}

Smb4KCustomSettings::~Smb4KCustomSettings()
{
}

void Smb4KCustomSettings::setNetworkItem(Smb4KBasicNetworkItem *networkItem) const
{
    if (networkItem && d->type == UnknownNetworkItem) {
        d->type = networkItem->type();
        d->url = networkItem->url();

        switch (d->type) {
        case Host: {
            Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

            if (host) {
                d->workgroup = host->workgroupName();
                d->smbPort = host->port() != -1 ? host->port() : d->smbPort;
                d->ip.setAddress(host->ipAddress());
            }
            break;
        }
        case Share: {
            Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

            if (share) {
                d->workgroup = share->workgroupName();
#if defined(Q_OS_LINUX)
                d->fileSystemPort = share->port() != -1 ? share->port() : d->fileSystemPort;
#endif
                d->user = share->user();
                d->group = share->group();
                d->ip.setAddress(share->hostIpAddress());
            }
            break;
        }
        default: {
            break;
        }
        }
    }
}

Smb4KGlobal::NetworkItem Smb4KCustomSettings::type() const
{
    return d->type;
}

void Smb4KCustomSettings::setWorkgroupName(const QString &workgroup) const
{
    d->workgroup = workgroup;
}

QString Smb4KCustomSettings::workgroupName() const
{
    return d->workgroup;
}

void Smb4KCustomSettings::setUrl(const QUrl &url) const
{
    d->url = url;
}

QUrl Smb4KCustomSettings::url() const
{
    return d->url;
}

QString Smb4KCustomSettings::hostName() const
{
    return d->url.host().toUpper();
}

QString Smb4KCustomSettings::shareName() const
{
    if (d->url.path().startsWith(QStringLiteral("/"))) {
        return d->url.path().remove(0, 1);
    }

    return d->url.path();
}

void Smb4KCustomSettings::setIpAddress(const QString &ip) const
{
    d->ip.setAddress(ip);
}

QString Smb4KCustomSettings::ipAddress() const
{
    return d->ip.toString();
}

bool Smb4KCustomSettings::hasIpAddress() const
{
    return !d->ip.isNull();
}

QString Smb4KCustomSettings::displayString() const
{
    QString string;

    switch (d->type) {
    case Host: {
        string = hostName();
        break;
    }
    case Share: {
        string = i18n("%1 on %2", shareName(), hostName());
        break;
    }
    default: {
        break;
    }
    }

    return string;
}

void Smb4KCustomSettings::setRemount(int remount) const
{
    switch (d->type) {
    case Share: {
        if (d->remount != remount) {
            d->remount = remount;
        }
        break;
    }
    default: {
        d->remount = UndefinedRemount;
        break;
    }
    }
}

int Smb4KCustomSettings::remount() const
{
    return d->remount;
}

void Smb4KCustomSettings::setUseUser(bool use) const
{
    d->useUser = use;
}

bool Smb4KCustomSettings::useUser() const
{
    return d->useUser;
}

void Smb4KCustomSettings::setUser(const KUser &user) const
{
    d->user = user;
}

KUser Smb4KCustomSettings::user() const
{
    return d->user;
}

void Smb4KCustomSettings::setUseGroup(bool use) const
{
    d->useGroup = use;
}

bool Smb4KCustomSettings::useGroup() const
{
    return d->useGroup;
}

void Smb4KCustomSettings::setGroup(const KUserGroup &group) const
{
    d->group = group;
}

KUserGroup Smb4KCustomSettings::group() const
{
    return d->group;
}

void Smb4KCustomSettings::setUseFileMode(bool use) const
{
    d->useFileMode = use;
}

bool Smb4KCustomSettings::useFileMode() const
{
    return d->useFileMode;
}

void Smb4KCustomSettings::setFileMode(const QString &mode) const
{
    d->fileMode = mode;
}

QString Smb4KCustomSettings::fileMode() const
{
    return d->fileMode;
}

void Smb4KCustomSettings::setUseDirectoryMode(bool use) const
{
    d->useDirectoryMode = use;
}

bool Smb4KCustomSettings::useDirectoryMode() const
{
    return d->useDirectoryMode;
}

void Smb4KCustomSettings::setDirectoryMode(const QString &mode) const
{
    d->directoryMode = mode;
}

QString Smb4KCustomSettings::directoryMode() const
{
    return d->directoryMode;
}

#if defined(Q_OS_LINUX)
void Smb4KCustomSettings::setCifsUnixExtensionsSupport(bool support) const
{
    d->cifsUnixExtensionsSupport = support;
}

bool Smb4KCustomSettings::cifsUnixExtensionsSupport() const
{
    return d->cifsUnixExtensionsSupport;
}

void Smb4KCustomSettings::setUseFileSystemPort(bool use) const
{
    d->useFileSystemPort = use;
}

bool Smb4KCustomSettings::useFileSystemPort() const
{
    return d->useFileSystemPort;
}

void Smb4KCustomSettings::setFileSystemPort(int port) const
{
    d->fileSystemPort = port;

    switch (d->type) {
    case Share: {
        d->url.setPort(port);
        break;
    }
    default: {
        break;
    }
    }
}

int Smb4KCustomSettings::fileSystemPort() const
{
    return d->fileSystemPort;
}

void Smb4KCustomSettings::setUseMountProtocolVersion(bool use) const
{
    d->useMountProtocolVersion = use;
}

bool Smb4KCustomSettings::useMountProtocolVersion() const
{
    return d->useMountProtocolVersion;
}

void Smb4KCustomSettings::setMountProtocolVersion(int version) const
{
    d->mountProtocolVersion = version;
}

int Smb4KCustomSettings::mountProtocolVersion() const
{
    return d->mountProtocolVersion;
}

void Smb4KCustomSettings::setUseSecurityMode(bool use) const
{
    d->useSecurityMode = use;
}

bool Smb4KCustomSettings::useSecurityMode() const
{
    return d->useSecurityMode;
}

void Smb4KCustomSettings::setSecurityMode(int mode) const
{
    d->securityMode = mode;
}

int Smb4KCustomSettings::securityMode() const
{
    return d->securityMode;
}

void Smb4KCustomSettings::setUseWriteAccess(bool use) const
{
    d->useWriteAccess = use;
}

bool Smb4KCustomSettings::useWriteAccess() const
{
    return d->useWriteAccess;
}

void Smb4KCustomSettings::setWriteAccess(int access) const
{
    d->writeAccess = access;
}

int Smb4KCustomSettings::writeAccess() const
{
    return d->writeAccess;
}
#endif

void Smb4KCustomSettings::setProfile(const QString &profile) const
{
    d->profile = profile;
}

QString Smb4KCustomSettings::profile() const
{
    return d->profile;
}

void Smb4KCustomSettings::setUseClientProtocolVersions(bool use) const
{
    d->useClientProtocolVersions = use;
}

bool Smb4KCustomSettings::useClientProtocolVersions() const
{
    return d->useClientProtocolVersions;
}

void Smb4KCustomSettings::setMinimalClientProtocolVersion(int version) const
{
    d->minimalClientProtocolVersion = version;
}

int Smb4KCustomSettings::minimalClientProtocolVersion() const
{
    return d->minimalClientProtocolVersion;
}

void Smb4KCustomSettings::setMaximalClientProtocolVersion(int version) const
{
    d->maximalClientProtocolVersion = version;
}

int Smb4KCustomSettings::maximalClientProtocolVersion() const
{
    return d->maximalClientProtocolVersion;
}

void Smb4KCustomSettings::setUseSmbPort(bool use) const
{
    d->useSmbPort = use;
}

bool Smb4KCustomSettings::useSmbPort() const
{
    return d->useSmbPort;
}

void Smb4KCustomSettings::setSmbPort(int port) const
{
    d->smbPort = port;

    switch (d->type) {
    case Host: {
        d->url.setPort(port);
        break;
    }
    default: {
        break;
    }
    }
}

int Smb4KCustomSettings::smbPort() const
{
    return d->smbPort;
}

void Smb4KCustomSettings::setUseKerberos(bool use) const
{
    d->useKerberos = use;
}

bool Smb4KCustomSettings::useKerberos() const
{
    return d->useKerberos;
}

void Smb4KCustomSettings::setMACAddress(const QString &macAddress) const
{
    QRegularExpression expression(QStringLiteral("..\\:..\\:..\\:..\\:..\\:.."));

    if (expression.match(macAddress).hasMatch() || macAddress.isEmpty()) {
        d->macAddress = macAddress;
    }
}

QString Smb4KCustomSettings::macAddress() const
{
    return d->macAddress;
}

void Smb4KCustomSettings::setWakeOnLanSendBeforeNetworkScan(bool send) const
{
    d->wakeOnLanBeforeFirstScan = send;
}

bool Smb4KCustomSettings::wakeOnLanSendBeforeNetworkScan() const
{
    return d->wakeOnLanBeforeFirstScan;
}

void Smb4KCustomSettings::setWakeOnLanSendBeforeMount(bool send) const
{
    d->wakeOnLanBeforeMount = send;
}

bool Smb4KCustomSettings::wakeOnLanSendBeforeMount() const
{
    return d->wakeOnLanBeforeMount;
}

QMap<QString, QString> Smb4KCustomSettings::customSettings() const
{
    QMap<QString, QString> entries;

    //
    // Remounting
    //
    entries.insert(QStringLiteral("remount"), QString::number(d->remount));

    //
    // User
    //
    entries.insert(QStringLiteral("use_user"), QString::number(d->useUser));
    entries.insert(QStringLiteral("uid"), d->user.userId().toString());

    //
    // Group
    //
    entries.insert(QStringLiteral("use_group"), QString::number(d->useGroup));
    entries.insert(QStringLiteral("gid"), d->group.groupId().toString());

    //
    // File mode
    //
    entries.insert(QStringLiteral("use_file_mode"), QString::number(d->useFileMode));
    entries.insert(QStringLiteral("file_mode"), d->fileMode);

    //
    // Directory mode
    //
    entries.insert(QStringLiteral("use_directory_mode"), QString::number(d->useDirectoryMode));
    entries.insert(QStringLiteral("directory_mode"), d->directoryMode);

#if defined(Q_OS_LINUX)
    //
    // Unix CIFS extensions supported
    //
    entries.insert(QStringLiteral("cifs_unix_extensions_support"), QString::number(d->cifsUnixExtensionsSupport));

    //
    // File system port
    //
    entries.insert(QStringLiteral("use_filesystem_port"), QString::number(d->useFileSystemPort));
    entries.insert(QStringLiteral("filesystem_port"), QString::number(fileSystemPort()));

    //
    // Mount protocol version
    //
    entries.insert(QStringLiteral("use_smb_mount_protocol_version"), QString::number(d->useMountProtocolVersion));
    entries.insert(QStringLiteral("smb_mount_protocol_version"), QString::number(d->mountProtocolVersion));

    //
    // Security mode
    //
    entries.insert(QStringLiteral("use_security_mode"), QString::number(d->useSecurityMode));
    entries.insert(QStringLiteral("security_mode"), QString::number(d->securityMode));

    //
    // Write access
    //
    entries.insert(QStringLiteral("use_write_access"), QString::number(d->useWriteAccess));
    entries.insert(QStringLiteral("write_access"), QString::number(d->writeAccess));
#endif

    //
    // Client protocol versions
    //
    entries.insert(QStringLiteral("use_client_protocol_versions"), QString::number(d->useClientProtocolVersions));
    entries.insert(QStringLiteral("minimal_client_protocol_version"), QString::number(d->minimalClientProtocolVersion));
    entries.insert(QStringLiteral("maximal_client_protocol_version"), QString::number(d->maximalClientProtocolVersion));

    //
    // SMB port used by the client
    //
    entries.insert(QStringLiteral("use_smb_port"), QString::number(d->useSmbPort));
    entries.insert(QStringLiteral("smb_port"), QString::number(smbPort()));

    //
    // Usage of Kerberos
    //
    entries.insert(QStringLiteral("kerberos"), QString::number(d->useKerberos));

    //
    // MAC address
    //
    entries.insert(QStringLiteral("mac_address"), d->macAddress);

    //
    // Wake-On_LAN settings
    //
    entries.insert(QStringLiteral("wol_send_before_first_scan"), QString::number(d->wakeOnLanBeforeFirstScan));
    entries.insert(QStringLiteral("wol_send_before_mount"), QString::number(d->wakeOnLanBeforeMount));

    return entries;
}

bool Smb4KCustomSettings::hasCustomSettings(bool withoutRemountOnce) const
{
    // NOTE: This function does not honor the workgroup, the url,
    // the ip address, the type and the profile, because these things
    // are not custom settings.

    // Perform remounts
    if ((!withoutRemountOnce && d->remount != Smb4KCustomSettings::UndefinedRemount) || d->remount == Smb4KCustomSettings::RemountAlways) {
        return true;
    }

    // Use user information
    if (d->useUser != Smb4KMountSettings::useUserId()) {
        return true;
    }

    // User information
    if (d->user.userId().toString() != Smb4KMountSettings::userId()) {
        return true;
    }

    // Use group information
    if (d->useGroup != Smb4KMountSettings::useGroupId()) {
        return true;
    }

    // Group information
    if (d->group.groupId().toString() != Smb4KMountSettings::groupId()) {
        return true;
    }

    // Use file mask
    if (d->useFileMode != Smb4KMountSettings::useFileMode()) {
        return true;
    }

    if (d->fileMode != Smb4KMountSettings::fileMode()) {
        return true;
    }

    if (d->useDirectoryMode != Smb4KMountSettings::useDirectoryMode()) {
        return true;
    }

    if (d->directoryMode != Smb4KMountSettings::directoryMode()) {
        return true;
    }
#if defined(Q_OS_LINUX)
    // CIFS Unix extension support
    if (d->cifsUnixExtensionsSupport != Smb4KMountSettings::cifsUnixExtensionsSupport()) {
        return true;
    }

    // Use filesystem port
    if (d->useFileSystemPort != Smb4KMountSettings::useRemoteFileSystemPort()) {
        return true;
    }

    // File system port (used for mounting)
    if (d->fileSystemPort != Smb4KMountSettings::remoteFileSystemPort()) {
        return true;
    }

    // Use SMB mount protocol version
    if (d->useMountProtocolVersion != Smb4KMountSettings::useSmbProtocolVersion()) {
        return true;
    }

    // SMB mount protocol version
    if (d->mountProtocolVersion != Smb4KMountSettings::smbProtocolVersion()) {
        return true;
    }

    // Use security mode
    if (d->useSecurityMode != Smb4KMountSettings::useSecurityMode()) {
        return true;
    }

    // Security mode
    if (d->securityMode != Smb4KMountSettings::securityMode()) {
        return true;
    }

    // Use write access
    if (d->useWriteAccess != Smb4KMountSettings::useWriteAccess()) {
        return true;
    }

    // Write access
    if (d->writeAccess != Smb4KMountSettings::writeAccess()) {
        return true;
    }
#endif

    // Use client protocol versions
    if (d->useClientProtocolVersions != Smb4KSettings::useClientProtocolVersions()) {
        return true;
    }

    // Minimal client protocol version
    if (d->minimalClientProtocolVersion != Smb4KSettings::minimalClientProtocolVersion()) {
        return true;
    }

    // Maximal client protocol version
    if (d->maximalClientProtocolVersion != Smb4KSettings::maximalClientProtocolVersion()) {
        return true;
    }

    // Use SMB port
    if (d->useSmbPort != Smb4KSettings::useRemoteSmbPort()) {
        return true;
    }

    // SMB port
    if (d->smbPort != Smb4KSettings::remoteSmbPort()) {
        return true;
    }

    // Kerberos
    if (d->useKerberos != Smb4KSettings::useKerberos()) {
        return true;
    }

    // MAC address
    if (!d->macAddress.isEmpty()) {
        return true;
    }

    // Send WOL packets before first scan
    if (d->wakeOnLanBeforeFirstScan) {
        return true;
    }

    // Send WOL packets before mount
    if (d->wakeOnLanBeforeMount) {
        return true;
    }

    return false;
}

void Smb4KCustomSettings::update(Smb4KCustomSettings *customSettings)
{
    // NOTE: Do not update the URL and the type

    d->workgroup = customSettings->workgroupName();
    d->ip.setAddress(customSettings->ipAddress());
    d->remount = customSettings->remount();
    d->useUser = customSettings->useUser();
    d->user = customSettings->user();
    d->useGroup = customSettings->useGroup();
    d->group = customSettings->group();
    d->useFileMode = customSettings->useFileMode();
    d->fileMode = customSettings->fileMode();
    d->useDirectoryMode = customSettings->useDirectoryMode();
    d->directoryMode = customSettings->directoryMode();
#if defined(Q_OS_LINUX)
    d->cifsUnixExtensionsSupport = customSettings->cifsUnixExtensionsSupport();
    d->useFileSystemPort = customSettings->useFileSystemPort();
    d->fileSystemPort = customSettings->fileSystemPort();
    d->useMountProtocolVersion = customSettings->useMountProtocolVersion();
    d->mountProtocolVersion = customSettings->mountProtocolVersion();
    d->useSecurityMode = customSettings->useSecurityMode();
    d->securityMode = customSettings->securityMode();
    d->useWriteAccess = customSettings->useWriteAccess();
    d->writeAccess = customSettings->writeAccess();
#endif
    d->profile = customSettings->profile();
    d->useClientProtocolVersions = customSettings->useClientProtocolVersions();
    d->minimalClientProtocolVersion = customSettings->minimalClientProtocolVersion();
    d->maximalClientProtocolVersion = customSettings->maximalClientProtocolVersion();
    d->useSmbPort = customSettings->useSmbPort();
    d->smbPort = customSettings->smbPort();
    d->useKerberos = customSettings->useKerberos();
    d->macAddress = customSettings->macAddress();
    d->wakeOnLanBeforeFirstScan = customSettings->wakeOnLanSendBeforeNetworkScan();
    d->wakeOnLanBeforeMount = customSettings->wakeOnLanSendBeforeMount();
}

Smb4KCustomSettings &Smb4KCustomSettings::operator=(const Smb4KCustomSettings &other)
{
    *d = *other.d;
    return *this;
}
