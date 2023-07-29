/*
    This class carries custom options

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kcustomoptions.h"
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

class Smb4KCustomOptionsPrivate
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

Smb4KCustomOptions::Smb4KCustomOptions(Smb4KBasicNetworkItem *networkItem)
    : d(new Smb4KCustomOptionsPrivate)
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

Smb4KCustomOptions::Smb4KCustomOptions(const Smb4KCustomOptions &o)
    : d(new Smb4KCustomOptionsPrivate)
{
    *d = *o.d;
}

Smb4KCustomOptions::Smb4KCustomOptions()
    : d(new Smb4KCustomOptionsPrivate)
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

Smb4KCustomOptions::~Smb4KCustomOptions()
{
}

void Smb4KCustomOptions::setNetworkItem(Smb4KBasicNetworkItem *networkItem) const
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

Smb4KGlobal::NetworkItem Smb4KCustomOptions::type() const
{
    return d->type;
}

void Smb4KCustomOptions::setWorkgroupName(const QString &workgroup) const
{
    d->workgroup = workgroup;
}

QString Smb4KCustomOptions::workgroupName() const
{
    return d->workgroup;
}

void Smb4KCustomOptions::setUrl(const QUrl &url) const
{
    d->url = url;
}

QUrl Smb4KCustomOptions::url() const
{
    return d->url;
}

QString Smb4KCustomOptions::hostName() const
{
    return d->url.host().toUpper();
}

QString Smb4KCustomOptions::shareName() const
{
    if (d->url.path().startsWith(QStringLiteral("/"))) {
        return d->url.path().remove(0, 1);
    }

    return d->url.path();
}

void Smb4KCustomOptions::setIpAddress(const QString &ip) const
{
    d->ip.setAddress(ip);
}

QString Smb4KCustomOptions::ipAddress() const
{
    return d->ip.toString();
}

bool Smb4KCustomOptions::hasIpAddress() const
{
    return !d->ip.isNull();
}

QString Smb4KCustomOptions::displayString() const
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

void Smb4KCustomOptions::setRemount(int remount) const
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

int Smb4KCustomOptions::remount() const
{
    return d->remount;
}

void Smb4KCustomOptions::setUseUser(bool use) const
{
    d->useUser = use;
}

bool Smb4KCustomOptions::useUser() const
{
    return d->useUser;
}

void Smb4KCustomOptions::setUser(const KUser &user) const
{
    d->user = user;
}

KUser Smb4KCustomOptions::user() const
{
    return d->user;
}

void Smb4KCustomOptions::setUseGroup(bool use) const
{
    d->useGroup = use;
}

bool Smb4KCustomOptions::useGroup() const
{
    return d->useGroup;
}

void Smb4KCustomOptions::setGroup(const KUserGroup &group) const
{
    d->group = group;
}

KUserGroup Smb4KCustomOptions::group() const
{
    return d->group;
}

void Smb4KCustomOptions::setUseFileMode(bool use) const
{
    d->useFileMode = use;
}

bool Smb4KCustomOptions::useFileMode() const
{
    return d->useFileMode;
}

void Smb4KCustomOptions::setFileMode(const QString &mode) const
{
    d->fileMode = mode;
}

QString Smb4KCustomOptions::fileMode() const
{
    return d->fileMode;
}

void Smb4KCustomOptions::setUseDirectoryMode(bool use) const
{
    d->useDirectoryMode = use;
}

bool Smb4KCustomOptions::useDirectoryMode() const
{
    return d->useDirectoryMode;
}

void Smb4KCustomOptions::setDirectoryMode(const QString &mode) const
{
    d->directoryMode = mode;
}

QString Smb4KCustomOptions::directoryMode() const
{
    return d->directoryMode;
}

#if defined(Q_OS_LINUX)
void Smb4KCustomOptions::setCifsUnixExtensionsSupport(bool support) const
{
    d->cifsUnixExtensionsSupport = support;
}

bool Smb4KCustomOptions::cifsUnixExtensionsSupport() const
{
    return d->cifsUnixExtensionsSupport;
}

void Smb4KCustomOptions::setUseFileSystemPort(bool use) const
{
    d->useFileSystemPort = use;
}

bool Smb4KCustomOptions::useFileSystemPort() const
{
    return d->useFileSystemPort;
}

void Smb4KCustomOptions::setFileSystemPort(int port) const
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

int Smb4KCustomOptions::fileSystemPort() const
{
    return d->fileSystemPort;
}

void Smb4KCustomOptions::setUseMountProtocolVersion(bool use) const
{
    d->useMountProtocolVersion = use;
}

bool Smb4KCustomOptions::useMountProtocolVersion() const
{
    return d->useMountProtocolVersion;
}

void Smb4KCustomOptions::setMountProtocolVersion(int version) const
{
    d->mountProtocolVersion = version;
}

int Smb4KCustomOptions::mountProtocolVersion() const
{
    return d->mountProtocolVersion;
}

void Smb4KCustomOptions::setUseSecurityMode(bool use) const
{
    d->useSecurityMode = use;
}

bool Smb4KCustomOptions::useSecurityMode() const
{
    return d->useSecurityMode;
}

void Smb4KCustomOptions::setSecurityMode(int mode) const
{
    d->securityMode = mode;
}

int Smb4KCustomOptions::securityMode() const
{
    return d->securityMode;
}

void Smb4KCustomOptions::setUseWriteAccess(bool use) const
{
    d->useWriteAccess = use;
}

bool Smb4KCustomOptions::useWriteAccess() const
{
    return d->useWriteAccess;
}

void Smb4KCustomOptions::setWriteAccess(int access) const
{
    d->writeAccess = access;
}

int Smb4KCustomOptions::writeAccess() const
{
    return d->writeAccess;
}
#endif

void Smb4KCustomOptions::setProfile(const QString &profile) const
{
    d->profile = profile;
}

QString Smb4KCustomOptions::profile() const
{
    return d->profile;
}

void Smb4KCustomOptions::setUseClientProtocolVersions(bool use) const
{
    d->useClientProtocolVersions = use;
}

bool Smb4KCustomOptions::useClientProtocolVersions() const
{
    return d->useClientProtocolVersions;
}

void Smb4KCustomOptions::setMinimalClientProtocolVersion(int version) const
{
    d->minimalClientProtocolVersion = version;
}

int Smb4KCustomOptions::minimalClientProtocolVersion() const
{
    return d->minimalClientProtocolVersion;
}

void Smb4KCustomOptions::setMaximalClientProtocolVersion(int version) const
{
    d->maximalClientProtocolVersion = version;
}

int Smb4KCustomOptions::maximalClientProtocolVersion() const
{
    return d->maximalClientProtocolVersion;
}

void Smb4KCustomOptions::setUseSmbPort(bool use) const
{
    d->useSmbPort = use;
}

bool Smb4KCustomOptions::useSmbPort() const
{
    return d->useSmbPort;
}

void Smb4KCustomOptions::setSmbPort(int port) const
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

int Smb4KCustomOptions::smbPort() const
{
    return d->smbPort;
}

void Smb4KCustomOptions::setUseKerberos(bool use) const
{
    d->useKerberos = use;
}

bool Smb4KCustomOptions::useKerberos() const
{
    return d->useKerberos;
}

void Smb4KCustomOptions::setMACAddress(const QString &macAddress) const
{
    QRegularExpression expression(QStringLiteral("..\\:..\\:..\\:..\\:..\\:.."));

    if (expression.match(macAddress).hasMatch() || macAddress.isEmpty()) {
        d->macAddress = macAddress;
    }
}

QString Smb4KCustomOptions::macAddress() const
{
    return d->macAddress;
}

void Smb4KCustomOptions::setWOLSendBeforeNetworkScan(bool send) const
{
    d->wakeOnLanBeforeFirstScan = send;
}

bool Smb4KCustomOptions::wolSendBeforeNetworkScan() const
{
    return d->wakeOnLanBeforeFirstScan;
}

void Smb4KCustomOptions::setWOLSendBeforeMount(bool send) const
{
    d->wakeOnLanBeforeMount = send;
}

bool Smb4KCustomOptions::wolSendBeforeMount() const
{
    return d->wakeOnLanBeforeMount;
}

QMap<QString, QString> Smb4KCustomOptions::customOptions() const
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

bool Smb4KCustomOptions::hasOptions(bool withoutRemountOnce) const
{
    //
    // NOTE: This function does not honor the workgroup, the url,
    // the ip address, the type and the profile, because these things
    // are not custom options.
    //

    // Perform remounts
    if ((!withoutRemountOnce && d->remount != Smb4KCustomOptions::UndefinedRemount) || d->remount == Smb4KCustomOptions::RemountAlways) {
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

void Smb4KCustomOptions::update(Smb4KCustomOptions *options)
{
    d->ip.setAddress(options->ipAddress());
    d->remount = options->remount();
    d->useUser = options->useUser();
    d->user = options->user();
    d->useGroup = options->useGroup();
    d->group = options->group();
    d->useFileMode = options->useFileMode();
    d->fileMode = options->fileMode();
    d->useDirectoryMode = options->useDirectoryMode();
    d->directoryMode = options->directoryMode();
#if defined(Q_OS_LINUX)
    d->cifsUnixExtensionsSupport = options->cifsUnixExtensionsSupport();
    d->useFileSystemPort = options->useFileSystemPort();
    d->fileSystemPort = options->fileSystemPort();
    d->useMountProtocolVersion = options->useMountProtocolVersion();
    d->mountProtocolVersion = options->mountProtocolVersion();
    d->useSecurityMode = options->useSecurityMode();
    d->securityMode = options->securityMode();
    d->useWriteAccess = options->useWriteAccess();
    d->writeAccess = options->writeAccess();
#endif
    d->useClientProtocolVersions = options->useClientProtocolVersions();
    d->minimalClientProtocolVersion = options->minimalClientProtocolVersion();
    d->maximalClientProtocolVersion = options->maximalClientProtocolVersion();
    d->profile = options->profile();
    d->useSmbPort = options->useSmbPort();
    d->smbPort = options->smbPort();
    d->useKerberos = options->useKerberos();
    d->macAddress = options->macAddress();
    d->wakeOnLanBeforeFirstScan = options->wolSendBeforeNetworkScan();
    d->wakeOnLanBeforeMount = options->wolSendBeforeMount();
}

Smb4KCustomOptions &Smb4KCustomOptions::operator=(const Smb4KCustomOptions &other)
{
    *d = *other.d;
    return *this;
}
