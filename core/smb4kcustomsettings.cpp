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
#include <QPair>
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
    QString profile;

    QPair<int, bool> remount;
    QPair<bool, bool> useUser;
    QPair<KUser, bool> user;
    QPair<bool, bool> useGroup;
    QPair<KUserGroup, bool> group;
    QPair<bool, bool> useFileMode;
    QPair<QString, bool> fileMode;
    QPair<bool, bool> useDirectoryMode;
    QPair<QString, bool> directoryMode;
#if defined(Q_OS_LINUX)
    QPair<bool, bool> cifsUnixExtensionsSupport;
    QPair<bool, bool> useFileSystemPort;
    QPair<int, bool> fileSystemPort;
    QPair<bool, bool> useMountProtocolVersion;
    QPair<int, bool> mountProtocolVersion;
    QPair<bool, bool> useSecurityMode;
    QPair<int, bool> securityMode;
    QPair<bool, bool> useWriteAccess;
    QPair<int, bool> writeAccess;
#endif
    QPair<bool, bool> useClientProtocolVersions;
    QPair<int, bool> minimalClientProtocolVersion;
    QPair<int, bool> maximalClientProtocolVersion;
    QPair<bool, bool> useSmbPort;
    QPair<int, bool> smbPort;
    QPair<bool, bool> useKerberos;
    QPair<QString, bool> macAddress;
    QPair<bool, bool> wakeOnLanBeforeFirstScan;
    QPair<bool, bool> wakeOnLanBeforeMount;
};

Smb4KCustomSettings::Smb4KCustomSettings(Smb4KBasicNetworkItem *networkItem)
    : d(new Smb4KCustomSettingsPrivate)
{
    d->type = networkItem->type();

    setUrl(networkItem->url());
    setProfile(Smb4KSettings::activeProfile());

    setRemount(UndefinedRemount);
    setUseFileMode(Smb4KMountSettings::useFileMode());
    setFileMode(Smb4KMountSettings::fileMode());
    setUseDirectoryMode(Smb4KMountSettings::useDirectoryMode());
    setDirectoryMode(Smb4KMountSettings::directoryMode());
#if defined(Q_OS_LINUX)
    setCifsUnixExtensionsSupport(Smb4KMountSettings::cifsUnixExtensionsSupport());
    setUseMountProtocolVersion(Smb4KMountSettings::useSmbProtocolVersion());
    setMountProtocolVersion(Smb4KMountSettings::smbProtocolVersion());
    setUseSecurityMode(Smb4KMountSettings::useSecurityMode());
    setSecurityMode(Smb4KMountSettings::securityMode());
    setUseWriteAccess(Smb4KMountSettings::useWriteAccess());
    setWriteAccess(Smb4KMountSettings::writeAccess());
#endif

    switch (d->type) {
    case Host: {
        Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

        if (host) {
            setWorkgroupName(host->workgroupName());
            setIpAddress(host->ipAddress());
            setUseUser(Smb4KMountSettings::useUserId());
            setUser(KUser((K_UID)Smb4KMountSettings::userId().toInt()));
            setUseGroup(Smb4KMountSettings::useGroupId());
            setGroup(KUserGroup((K_GID)Smb4KMountSettings::groupId().toInt()));
            setUseSmbPort(Smb4KSettings::useRemoteSmbPort());
            setSmbPort(host->port() != -1 ? host->port() : Smb4KSettings::remoteSmbPort());
#if defined(Q_OS_LINUX)
            setUseFileSystemPort(Smb4KMountSettings::useRemoteFileSystemPort());
            setFileSystemPort(Smb4KMountSettings::remoteFileSystemPort());
#endif
        }
        break;
    }
    case Share: {
        Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

        if (share) {
            setWorkgroupName(share->workgroupName());
            setIpAddress(share->hostIpAddress());
            setUseUser(Smb4KMountSettings::useUserId());
            setUser(share->user());
            setUseGroup(Smb4KMountSettings::useGroupId());
            setGroup(share->group());
            setUseSmbPort(Smb4KSettings::useRemoteSmbPort());
            setSmbPort(Smb4KSettings::remoteSmbPort());
#if defined(Q_OS_LINUX)
            setUseFileSystemPort(Smb4KMountSettings::useRemoteFileSystemPort());
            setFileSystemPort(share->port() != -1 ? share->port() : Smb4KMountSettings::remoteFileSystemPort());
#endif
        }
        break;
    }
    default: {
        break;
    }
    }

    setUseClientProtocolVersions(Smb4KSettings::useClientProtocolVersions());
    setMinimalClientProtocolVersion(Smb4KSettings::minimalClientProtocolVersion());
    setMaximalClientProtocolVersion(Smb4KSettings::maximalClientProtocolVersion());
    setUseKerberos(Smb4KSettings::useKerberos());
    setMacAddress(QString());
    setWakeOnLanSendBeforeNetworkScan(false);
    setWakeOnLanSendBeforeMount(false);
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

    setUrl(QUrl());
    setProfile(Smb4KSettings::activeProfile());

    setRemount(UndefinedRemount);
    setUseUser(Smb4KMountSettings::useUserId());
    setUser(KUser((K_UID)Smb4KMountSettings::userId().toInt()));
    setUseGroup(Smb4KMountSettings::useGroupId());
    setGroup(KUserGroup((K_GID)Smb4KMountSettings::groupId().toInt()));
    setUseFileMode(Smb4KMountSettings::useFileMode());
    setFileMode(Smb4KMountSettings::fileMode());
    setUseDirectoryMode(Smb4KMountSettings::useDirectoryMode());
    setDirectoryMode(Smb4KMountSettings::directoryMode());
#if defined(Q_OS_LINUX)
    setCifsUnixExtensionsSupport(Smb4KMountSettings::cifsUnixExtensionsSupport());
    setUseFileSystemPort(Smb4KMountSettings::useRemoteFileSystemPort());
    setFileSystemPort(Smb4KMountSettings::remoteFileSystemPort());
    setUseMountProtocolVersion(Smb4KMountSettings::useSmbProtocolVersion());
    setMountProtocolVersion(Smb4KMountSettings::smbProtocolVersion());
    setUseSecurityMode(Smb4KMountSettings::useSecurityMode());
    setSecurityMode(Smb4KMountSettings::securityMode());
    setUseWriteAccess(Smb4KMountSettings::useWriteAccess());
    setWriteAccess(Smb4KMountSettings::writeAccess());
#endif
    setUseClientProtocolVersions(Smb4KSettings::useClientProtocolVersions());
    setMinimalClientProtocolVersion(Smb4KSettings::minimalClientProtocolVersion());
    setMaximalClientProtocolVersion(Smb4KSettings::maximalClientProtocolVersion());
    setUseSmbPort(Smb4KSettings::useRemoteSmbPort());
    setSmbPort(Smb4KSettings::remoteSmbPort());
    setUseKerberos(Smb4KSettings::useKerberos());
    setMacAddress(QString());
    setWakeOnLanSendBeforeNetworkScan(false);
    setWakeOnLanSendBeforeMount(false);
}

Smb4KCustomSettings::~Smb4KCustomSettings()
{
}

void Smb4KCustomSettings::setNetworkItem(Smb4KBasicNetworkItem *networkItem) const
{
    if (networkItem && d->type == UnknownNetworkItem) {
        d->type = networkItem->type();

        setUrl(networkItem->url());

        switch (d->type) {
        case Host: {
            Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);

            if (host) {
                setWorkgroupName(host->workgroupName());
                setSmbPort(host->port() != -1 ? host->port() : Smb4KSettings::remoteSmbPort());
                setIpAddress(host->ipAddress());
            }
            break;
        }
        case Share: {
            Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);

            if (share) {
                setWorkgroupName(share->workgroupName());
#if defined(Q_OS_LINUX)
                setFileSystemPort(share->port() != -1 ? share->port() : Smb4KMountSettings::remoteFileSystemPort());
#endif
                setUser(share->user());
                setGroup(share->group());
                setIpAddress(share->hostIpAddress());
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

void Smb4KCustomSettings::setProfile(const QString &profile) const
{
    d->profile = profile;
}

QString Smb4KCustomSettings::profile() const
{
    return d->profile;
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
        d->remount = {remount, (remount != UndefinedRemount)};
        break;
    }
    default: {
        d->remount = {UndefinedRemount, false};
        break;
    }
    }
}

int Smb4KCustomSettings::remount() const
{
    return d->remount.first;
}

void Smb4KCustomSettings::setUseUser(bool use) const
{
    d->useUser = {use, (use != Smb4KMountSettings::useUserId())};
}

bool Smb4KCustomSettings::useUser() const
{
    return d->useUser.first;
}

void Smb4KCustomSettings::setUser(const KUser &user) const
{
    d->user = {user, (user.userId().toString() != Smb4KMountSettings::userId())};
}

KUser Smb4KCustomSettings::user() const
{
    return d->user.first;
}

void Smb4KCustomSettings::setUseGroup(bool use) const
{
    d->useGroup = {use, (use != Smb4KMountSettings::useGroupId())};
}

bool Smb4KCustomSettings::useGroup() const
{
    return d->useGroup.first;
}

void Smb4KCustomSettings::setGroup(const KUserGroup &group) const
{
    d->group = {group, (group.groupId().toString() != Smb4KMountSettings::groupId())};
}

KUserGroup Smb4KCustomSettings::group() const
{
    return d->group.first;
}

void Smb4KCustomSettings::setUseFileMode(bool use) const
{
    d->useFileMode = {use, (use != Smb4KMountSettings::useFileMode())};
}

bool Smb4KCustomSettings::useFileMode() const
{
    return d->useFileMode.first;
}

void Smb4KCustomSettings::setFileMode(const QString &mode) const
{
    d->fileMode = {mode, (mode != Smb4KMountSettings::fileMode())};
}

QString Smb4KCustomSettings::fileMode() const
{
    return d->fileMode.first;
}

void Smb4KCustomSettings::setUseDirectoryMode(bool use) const
{
    d->useDirectoryMode = {use, (use != Smb4KMountSettings::useDirectoryMode())};
}

bool Smb4KCustomSettings::useDirectoryMode() const
{
    return d->useDirectoryMode.first;
}

void Smb4KCustomSettings::setDirectoryMode(const QString &mode) const
{
    d->directoryMode = {mode, (mode != Smb4KMountSettings::directoryMode())};
}

QString Smb4KCustomSettings::directoryMode() const
{
    return d->directoryMode.first;
}

#if defined(Q_OS_LINUX)
void Smb4KCustomSettings::setCifsUnixExtensionsSupport(bool support) const
{
    d->cifsUnixExtensionsSupport = {support, (support != Smb4KMountSettings::cifsUnixExtensionsSupport())};
}

bool Smb4KCustomSettings::cifsUnixExtensionsSupport() const
{
    return d->cifsUnixExtensionsSupport.first;
}

void Smb4KCustomSettings::setUseFileSystemPort(bool use) const
{
    d->useFileSystemPort = {use, (use != Smb4KMountSettings::useRemoteFileSystemPort())};
}

bool Smb4KCustomSettings::useFileSystemPort() const
{
    return d->useFileSystemPort.first;
}

void Smb4KCustomSettings::setFileSystemPort(int port) const
{
    d->fileSystemPort = {port, (port != Smb4KMountSettings::remoteFileSystemPort())};

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
    return d->fileSystemPort.first;
}

void Smb4KCustomSettings::setUseMountProtocolVersion(bool use) const
{
    d->useMountProtocolVersion = {use, (use != Smb4KMountSettings::useSmbProtocolVersion())};
}

bool Smb4KCustomSettings::useMountProtocolVersion() const
{
    return d->useMountProtocolVersion.first;
}

void Smb4KCustomSettings::setMountProtocolVersion(int version) const
{
    d->mountProtocolVersion = {version, (version != Smb4KMountSettings::smbProtocolVersion())};
}

int Smb4KCustomSettings::mountProtocolVersion() const
{
    return d->mountProtocolVersion.first;
}

void Smb4KCustomSettings::setUseSecurityMode(bool use) const
{
    d->useSecurityMode = {use, (use != Smb4KMountSettings::useSecurityMode())};
}

bool Smb4KCustomSettings::useSecurityMode() const
{
    return d->useSecurityMode.first;
}

void Smb4KCustomSettings::setSecurityMode(int mode) const
{
    d->securityMode = {mode, (mode != Smb4KMountSettings::securityMode())};
}

int Smb4KCustomSettings::securityMode() const
{
    return d->securityMode.first;
}

void Smb4KCustomSettings::setUseWriteAccess(bool use) const
{
    d->useWriteAccess = {use, (use != Smb4KMountSettings::useWriteAccess())};
}

bool Smb4KCustomSettings::useWriteAccess() const
{
    return d->useWriteAccess.first;
}

void Smb4KCustomSettings::setWriteAccess(int access) const
{
    d->writeAccess = {access, (access != Smb4KMountSettings::writeAccess())};
}

int Smb4KCustomSettings::writeAccess() const
{
    return d->writeAccess.first;
}
#endif

void Smb4KCustomSettings::setUseClientProtocolVersions(bool use) const
{
    d->useClientProtocolVersions = {use, (use != Smb4KSettings::useClientProtocolVersions())};
}

bool Smb4KCustomSettings::useClientProtocolVersions() const
{
    return d->useClientProtocolVersions.first;
}

void Smb4KCustomSettings::setMinimalClientProtocolVersion(int version) const
{
    d->minimalClientProtocolVersion = {version, (version != Smb4KSettings::minimalClientProtocolVersion())};
}

int Smb4KCustomSettings::minimalClientProtocolVersion() const
{
    return d->minimalClientProtocolVersion.first;
}

void Smb4KCustomSettings::setMaximalClientProtocolVersion(int version) const
{
    d->maximalClientProtocolVersion = {version, (version != Smb4KSettings::maximalClientProtocolVersion())};
}

int Smb4KCustomSettings::maximalClientProtocolVersion() const
{
    return d->maximalClientProtocolVersion.first;
}

void Smb4KCustomSettings::setUseSmbPort(bool use) const
{
    d->useSmbPort = {use, (use != Smb4KSettings::useRemoteSmbPort())};
}

bool Smb4KCustomSettings::useSmbPort() const
{
    return d->useSmbPort.first;
}

void Smb4KCustomSettings::setSmbPort(int port) const
{
    d->smbPort = {port, (port != Smb4KSettings::remoteSmbPort())};

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
    return d->smbPort.first;
}

void Smb4KCustomSettings::setUseKerberos(bool use) const
{
    d->useKerberos = {use, (use != Smb4KSettings::useKerberos())};
}

bool Smb4KCustomSettings::useKerberos() const
{
    return d->useKerberos.first;
}

void Smb4KCustomSettings::setMacAddress(const QString &macAddress) const
{
    QRegularExpression expression(QStringLiteral("..\\:..\\:..\\:..\\:..\\:.."));

    if (expression.match(macAddress).hasMatch() || macAddress.isEmpty()) {
        d->macAddress = {macAddress, !macAddress.isEmpty()};
    }
}

QString Smb4KCustomSettings::macAddress() const
{
    return d->macAddress.first;
}

void Smb4KCustomSettings::setWakeOnLanSendBeforeNetworkScan(bool send) const
{
    d->wakeOnLanBeforeFirstScan = {send, true};
}

bool Smb4KCustomSettings::wakeOnLanSendBeforeNetworkScan() const
{
    return d->wakeOnLanBeforeFirstScan.first;
}

void Smb4KCustomSettings::setWakeOnLanSendBeforeMount(bool send) const
{
    d->wakeOnLanBeforeMount = {send, true};
}

bool Smb4KCustomSettings::wakeOnLanSendBeforeMount() const
{
    return d->wakeOnLanBeforeMount.first;
}

QMap<QString, QString> Smb4KCustomSettings::customSettings() const
{
    QMap<QString, QString> entries;

    // Remounting
    if (d->remount.second && (d->remount.first != UndefinedRemount)) {
        entries.insert(QStringLiteral("remount"), QString::number(d->remount.first));
    }

    // User
    if (d->useUser.second && (d->useUser.first != Smb4KMountSettings::useUserId())) {
        entries.insert(QStringLiteral("use_user"), QString::number(d->useUser.first));
    }

    if (d->user.second && (d->user.first.userId().toString() != Smb4KMountSettings::userId())) {
        entries.insert(QStringLiteral("uid"), d->user.first.userId().toString());
    }

    // Group
    if (d->useGroup.second && (d->useGroup.first != Smb4KMountSettings::useGroupId())) {
        entries.insert(QStringLiteral("use_group"), QString::number(d->useGroup.first));
    }

    if (d->group.second && (d->group.first.groupId().toString() != Smb4KMountSettings::groupId())) {
        entries.insert(QStringLiteral("gid"), d->group.first.groupId().toString());
    }

    // File mode
    if (d->useFileMode.second && (d->useFileMode.first != Smb4KMountSettings::useFileMode())) {
        entries.insert(QStringLiteral("use_file_mode"), QString::number(d->useFileMode.first));
    }

    if (d->fileMode.second && (d->fileMode.first != Smb4KMountSettings::fileMode())) {
        entries.insert(QStringLiteral("file_mode"), d->fileMode.first);
    }

    // Directory mode
    if (d->useDirectoryMode.second && (d->useDirectoryMode.first != Smb4KMountSettings::useDirectoryMode())) {
        entries.insert(QStringLiteral("use_directory_mode"), QString::number(d->useDirectoryMode.first));
    }

    if (d->directoryMode.second && (d->directoryMode.first != Smb4KMountSettings::directoryMode())) {
        entries.insert(QStringLiteral("directory_mode"), d->directoryMode.first);
    }

#if defined(Q_OS_LINUX)
    // Unix CIFS extensions supported
    if (d->cifsUnixExtensionsSupport.second && (d->cifsUnixExtensionsSupport.first != Smb4KMountSettings::cifsUnixExtensionsSupport())) {
        entries.insert(QStringLiteral("cifs_unix_extensions_support"), QString::number(d->cifsUnixExtensionsSupport.first));
    }

    // File system port
    if (d->useFileSystemPort.second && (d->useFileSystemPort.first != Smb4KMountSettings::useRemoteFileSystemPort())) {
        entries.insert(QStringLiteral("use_filesystem_port"), QString::number(d->useFileSystemPort.first));
    }

    if (d->fileSystemPort.second && (d->fileSystemPort.first != Smb4KMountSettings::remoteFileSystemPort())) {
        entries.insert(QStringLiteral("filesystem_port"), QString::number(d->fileSystemPort.first));
    }

    // Mount protocol version
    if (d->useMountProtocolVersion.second && (d->useMountProtocolVersion.first != Smb4KMountSettings::useSmbProtocolVersion())) {
        entries.insert(QStringLiteral("use_smb_mount_protocol_version"), QString::number(d->useMountProtocolVersion.first));
    }

    if (d->mountProtocolVersion.second && (d->mountProtocolVersion.first != Smb4KMountSettings::smbProtocolVersion())) {
        entries.insert(QStringLiteral("smb_mount_protocol_version"), QString::number(d->mountProtocolVersion.first));
    }

    // Security mode
    if (d->useSecurityMode.second && (d->useSecurityMode.first != Smb4KMountSettings::useSecurityMode())) {
        entries.insert(QStringLiteral("use_security_mode"), QString::number(d->useSecurityMode.first));
    }

    if (d->securityMode.second && (d->securityMode.first != Smb4KMountSettings::securityMode())) {
        entries.insert(QStringLiteral("security_mode"), QString::number(d->securityMode.first));
    }

    // Write access
    if (d->useWriteAccess.second && (d->useWriteAccess.first != Smb4KMountSettings::useWriteAccess())) {
        entries.insert(QStringLiteral("use_write_access"), QString::number(d->useWriteAccess.first));
    }

    if (d->writeAccess.second && (d->writeAccess.first != Smb4KMountSettings::writeAccess())) {
        entries.insert(QStringLiteral("write_access"), QString::number(d->writeAccess.first));
    }
#endif

    // Client protocol versions
    if (d->useClientProtocolVersions.second && (d->useClientProtocolVersions.first != Smb4KSettings::useClientProtocolVersions())) {
        entries.insert(QStringLiteral("use_client_protocol_versions"), QString::number(d->useClientProtocolVersions.first));
    }

    if (d->minimalClientProtocolVersion.second && (d->minimalClientProtocolVersion.first != Smb4KSettings::minimalClientProtocolVersion())) {
        entries.insert(QStringLiteral("minimal_client_protocol_version"), QString::number(d->minimalClientProtocolVersion.first));
    }

    if (d->maximalClientProtocolVersion.second && (d->maximalClientProtocolVersion.first != Smb4KSettings::maximalClientProtocolVersion())) {
        entries.insert(QStringLiteral("maximal_client_protocol_version"), QString::number(d->maximalClientProtocolVersion.first));
    }

    // SMB port used by the client
    if (d->useSmbPort.second && (d->useSmbPort.first != Smb4KSettings::useRemoteSmbPort())) {
        entries.insert(QStringLiteral("use_smb_port"), QString::number(d->useSmbPort.first));
    }

    if (d->smbPort.second && (d->smbPort.first != Smb4KSettings::remoteSmbPort())) {
        entries.insert(QStringLiteral("smb_port"), QString::number(d->smbPort.first));
    }

    // Usage of Kerberos
    if (d->useKerberos.second && (d->useKerberos.first != Smb4KSettings::useKerberos())) {
        entries.insert(QStringLiteral("kerberos"), QString::number(d->useKerberos.first));
    }

    // MAC address
    if (d->macAddress.second && !d->macAddress.first.isEmpty()) {
        entries.insert(QStringLiteral("mac_address"), d->macAddress.first);
    }

    // Wake-On_LAN settings
    if (d->wakeOnLanBeforeFirstScan.second && (d->wakeOnLanBeforeFirstScan.first != false)) {
        entries.insert(QStringLiteral("wol_send_before_first_scan"), QString::number(d->wakeOnLanBeforeFirstScan.first));
    }

    if (d->wakeOnLanBeforeMount.second && (d->wakeOnLanBeforeMount.first != false)) {
        entries.insert(QStringLiteral("wol_send_before_mount"), QString::number(d->wakeOnLanBeforeMount.first));
    }

    return entries;
}

bool Smb4KCustomSettings::hasCustomSettings(bool withoutRemountOnce) const
{
    // NOTE: This function does not honor the workgroup, the url,
    // the ip address, the type and the profile, because these things
    // are not custom settings.

    // Perform remounts
    if (d->remount.second && (!withoutRemountOnce || d->remount.first == RemountAlways)) {
        return true;
    }

    // User
    if (d->useUser.second && (d->useUser.first != Smb4KMountSettings::useUserId())) {
        return true;
    }

    if (d->user.second && (d->user.first.userId().toString() != Smb4KMountSettings::userId())) {
        return true;
    }

    // Group
    if (d->useGroup.second && (d->useGroup.first != Smb4KMountSettings::useGroupId())) {
        return true;
    }

    if (d->group.second && (d->group.first.groupId().toString() != Smb4KMountSettings::groupId())) {
        return true;
    }

    // File mode
    if (d->useFileMode.second && (d->useFileMode.first != Smb4KMountSettings::useFileMode())) {
        return true;
    }

    if (d->fileMode.second && (d->fileMode.first != Smb4KMountSettings::fileMode())) {
        return true;
    }

    // Directory mode
    if (d->useDirectoryMode.second && (d->useDirectoryMode.first != Smb4KMountSettings::useDirectoryMode())) {
        return true;
    }

    if (d->directoryMode.second && (d->directoryMode.first != Smb4KMountSettings::directoryMode())) {
        return true;
    }

#if defined(Q_OS_LINUX)
    // Unix CIFS extensions supported
    if (d->cifsUnixExtensionsSupport.second && (d->cifsUnixExtensionsSupport.first != Smb4KMountSettings::cifsUnixExtensionsSupport())) {
        return true;
    }

    // File system port
    if (d->useFileSystemPort.second && (d->useFileSystemPort.first != Smb4KMountSettings::useRemoteFileSystemPort())) {
        return true;
    }

    if (d->fileSystemPort.second && (d->fileSystemPort.first != Smb4KMountSettings::remoteFileSystemPort())) {
        return true;
    }

    // Mount protocol version
    if (d->useMountProtocolVersion.second && (d->useMountProtocolVersion.first != Smb4KMountSettings::useSmbProtocolVersion())) {
        return true;
    }

    if (d->mountProtocolVersion.second && (d->mountProtocolVersion.first != Smb4KMountSettings::smbProtocolVersion())) {
        return true;
    }

    // Security mode
    if (d->useSecurityMode.second && (d->useSecurityMode.first != Smb4KMountSettings::useSecurityMode())) {
        return true;
    }

    if (d->securityMode.second && (d->securityMode.first != Smb4KMountSettings::securityMode())) {
        return true;
    }

    // Write access
    if (d->useWriteAccess.second && (d->useWriteAccess.first != Smb4KMountSettings::useWriteAccess())) {
        return true;
    }

    if (d->writeAccess.second && (d->writeAccess.first != Smb4KMountSettings::writeAccess())) {
        return true;
    }
#endif

    // Client protocol versions
    if (d->useClientProtocolVersions.second && (d->useClientProtocolVersions.first != Smb4KSettings::useClientProtocolVersions())) {
        return true;
    }

    if (d->minimalClientProtocolVersion.second && (d->minimalClientProtocolVersion.first != Smb4KSettings::minimalClientProtocolVersion())) {
        return true;
    }

    if (d->maximalClientProtocolVersion.second && (d->maximalClientProtocolVersion.first != Smb4KSettings::maximalClientProtocolVersion())) {
        return true;
    }

    // SMB port used by the client
    if (d->useSmbPort.second && (d->useSmbPort.first != Smb4KSettings::useRemoteSmbPort())) {
        return true;
    }

    if (d->smbPort.second && (d->smbPort.first != Smb4KSettings::remoteSmbPort())) {
        return true;
    }

    // Usage of Kerberos
    if (d->useKerberos.second && (d->useKerberos.first != Smb4KSettings::useKerberos())) {
        return true;
    }

    // MAC address
    if (d->macAddress.second && !d->macAddress.first.isEmpty()) {
        return true;
    }

    // Wake-On_LAN settings
    if (d->wakeOnLanBeforeFirstScan.second && (d->wakeOnLanBeforeFirstScan.first != false)) {
        return true;
    }

    if (d->wakeOnLanBeforeMount.second && (d->wakeOnLanBeforeMount.first != false)) {
        return true;
    }

    return false;
}

void Smb4KCustomSettings::update(Smb4KCustomSettings *customSettings)
{
    // NOTE: Do not update the URL and the type

    d->workgroup = customSettings->workgroupName();
    d->ip.setAddress(customSettings->ipAddress());
    d->profile = customSettings->profile();

    setRemount(customSettings->remount());
    setUseUser(customSettings->useUser());
    setUser(customSettings->user());
    setUseGroup(customSettings->useGroup());
    setGroup(customSettings->group());
    setUseFileMode(customSettings->useFileMode());
    setFileMode(customSettings->fileMode());
    setUseDirectoryMode(customSettings->useDirectoryMode());
    setDirectoryMode(customSettings->directoryMode());
#if defined(Q_OS_LINUX)
    setCifsUnixExtensionsSupport(customSettings->cifsUnixExtensionsSupport());
    setUseFileSystemPort(customSettings->useFileSystemPort());
    setFileSystemPort(customSettings->fileSystemPort());
    setUseMountProtocolVersion(customSettings->useMountProtocolVersion());
    setMountProtocolVersion(customSettings->mountProtocolVersion());
    setUseSecurityMode(customSettings->useSecurityMode());
    setSecurityMode(customSettings->securityMode());
    setUseWriteAccess(customSettings->useWriteAccess());
    setWriteAccess(customSettings->writeAccess());
#endif
    setUseClientProtocolVersions(customSettings->useClientProtocolVersions());
    setMinimalClientProtocolVersion(customSettings->minimalClientProtocolVersion());
    setMaximalClientProtocolVersion(customSettings->maximalClientProtocolVersion());
    setUseSmbPort(customSettings->useSmbPort());
    setSmbPort(customSettings->smbPort());
    setUseKerberos(customSettings->useKerberos());
    setMacAddress(customSettings->macAddress());
    setWakeOnLanSendBeforeNetworkScan(customSettings->wakeOnLanSendBeforeNetworkScan());
    setWakeOnLanSendBeforeMount(customSettings->wakeOnLanSendBeforeMount());
}

Smb4KCustomSettings &Smb4KCustomSettings::operator=(const Smb4KCustomSettings &other)
{
    *d = *other.d;
    return *this;
}
