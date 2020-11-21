/***************************************************************************
    This class carries custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2020 by Alexander Reinholdt
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

// application specific includes
#include "smb4kcustomoptions.h"
#include "smb4ksettings.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QDebug>
#include <QHostAddress>

// KDE includes
#include <KCoreAddons/KUser>
#include <KI18n/KLocalizedString>


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
    QString mac;
    bool wakeOnLanBeforeFirstScan;
    bool wakeOnLanBeforeMount;
};


Smb4KCustomOptions::Smb4KCustomOptions(Smb4KHost *host)
: d(new Smb4KCustomOptionsPrivate)
{
  d->workgroup = host->workgroupName();
  d->url = host->url();
  d->type = Host;
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
  d->smbPort = host->port() != -1 ? host->port() : Smb4KSettings::remoteSmbPort();
  d->useKerberos = Smb4KSettings::useKerberos();
  d->ip.setAddress(host->ipAddress());
  d->wakeOnLanBeforeFirstScan = false;
  d->wakeOnLanBeforeMount = false;
}

Smb4KCustomOptions::Smb4KCustomOptions(Smb4KShare *share)
: d(new Smb4KCustomOptionsPrivate)
{
  d->url = share->url();
  d->workgroup = share->workgroupName();
  d->type = Share;
  d->remount = UndefinedRemount;
  d->useUser = Smb4KMountSettings::useUserId();
  d->user = share->user();
  d->useGroup = Smb4KMountSettings::useGroupId();
  d->group = share->group();
  d->useFileMode = Smb4KMountSettings::useFileMode();
  d->fileMode = Smb4KMountSettings::fileMode();
  d->useDirectoryMode = Smb4KMountSettings::useDirectoryMode();
  d->directoryMode = Smb4KMountSettings::directoryMode();
#if defined(Q_OS_LINUX)
  d->cifsUnixExtensionsSupport = Smb4KMountSettings::cifsUnixExtensionsSupport();
  d->useFileSystemPort = Smb4KMountSettings::useRemoteFileSystemPort();
  d->fileSystemPort = share->port() != -1 ? share->port() : Smb4KMountSettings::remoteFileSystemPort();
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
  d->ip.setAddress(share->hostIpAddress());
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


void Smb4KCustomOptions::setHost(Smb4KHost *host)
{
  //
  // Set all variables that can be extracted from the host item
  //
  if (host)
  {
    switch (d->type)
    {
      case UnknownNetworkItem:
      {
        d->workgroup = host->workgroupName();
        d->url = host->url();
        d->type = Host;
        d->smbPort = host->port() != -1 ? host->port() : d->smbPort;
        d->ip.setAddress(host->ipAddress());
        break;
      }
      default:
      {
        break;
      }
    }
  }
}


void Smb4KCustomOptions::setShare(Smb4KShare *share)
{
  //
  // Set all variables that can be extracted from the share item
  // 
  if (share)
  {
    switch (d->type)
    {
      case UnknownNetworkItem:
      {
        d->url = share->url();
        d->workgroup = share->workgroupName();
        d->type = Share;
#if defined(Q_OS_LINUX)
        d->fileSystemPort = share->port() != -1 ? share->port() : d->fileSystemPort;
#endif
        d->user = share->user();
        d->group = share->group();
        d->ip.setAddress(share->hostIpAddress());
        break;
      }
      case Host:
      {
        if (QString::compare(d->url.toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath),
                             share->url().toString(QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::RemovePath),
                             Qt::CaseInsensitive) == 0)
        {
          d->url = share->url();
          d->type = Share;
#if defined(Q_OS_LINUX)
          d->fileSystemPort = share->port() != -1 ? share->port() : d->fileSystemPort;
#endif
          d->user = share->user();
          d->group = share->group();
          d->ip.setAddress(share->hostIpAddress());
        }
        break;
      }
      default:
      {
        break;
      }
    }    
  }
}


Smb4KGlobal::NetworkItem Smb4KCustomOptions::type() const
{
  return d->type;
}


void Smb4KCustomOptions::setWorkgroupName(const QString &workgroup)
{
  d->workgroup = workgroup;
}


QString Smb4KCustomOptions::workgroupName() const
{
  return d->workgroup;
}


void Smb4KCustomOptions::setUrl(const QUrl &url)
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
  if (d->url.path().startsWith('/'))
  {
    return d->url.path().remove(0, 1);
  }

  return d->url.path();
}


void Smb4KCustomOptions::setIpAddress(const QString &ip)
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
  
  switch (d->type)
  {
    case Host:
    {
      string = hostName();
      break;
    }
    case Share:
    {
      string = i18n("%1 on %2", shareName(), hostName());
      break;
    }
    default:
    {
      break;
    }
  }
  
  return string;
}


void Smb4KCustomOptions::setRemount(int remount)
{
  switch (d->type)
  {
    case Share:
    {
      d->remount = remount;
      break;
    }
    default:
    {
      d->remount = UndefinedRemount;
      break;
    }
  }
}


int Smb4KCustomOptions::remount() const
{
  return d->remount;
}


void Smb4KCustomOptions::setUseUser(bool use)
{
  d->useUser = use;
}


bool Smb4KCustomOptions::useUser() const
{
  return d->useUser;
}


void Smb4KCustomOptions::setUser(const KUser &user)
{
  d->user = user;
}


KUser Smb4KCustomOptions::user() const
{
  return d->user;
}


void Smb4KCustomOptions::setUseGroup(bool use)
{
  d->useGroup = use;
}


bool Smb4KCustomOptions::useGroup() const
{
  return d->useGroup;
}


void Smb4KCustomOptions::setGroup(const KUserGroup& group)
{
  d->group = group;
}


KUserGroup Smb4KCustomOptions::group() const
{
  return d->group;
}


void Smb4KCustomOptions::setUseFileMode(bool use)
{
  d->useFileMode = use;
}


bool Smb4KCustomOptions::useFileMode() const
{
  return d->useFileMode;
}


void Smb4KCustomOptions::setFileMode(const QString& mode)
{
  d->fileMode = mode;
}


QString Smb4KCustomOptions::fileMode() const
{
  return d->fileMode;
}


void Smb4KCustomOptions::setUseDirectoryMode(bool use)
{
  d->useDirectoryMode = use;
}


bool Smb4KCustomOptions::useDirectoryMode() const
{
  return d->useDirectoryMode;
}


void Smb4KCustomOptions::setDirectoryMode(const QString& mode)
{
  d->directoryMode = mode;
}


QString Smb4KCustomOptions::directoryMode() const
{
  return d->directoryMode;
}


#if defined(Q_OS_LINUX)
void Smb4KCustomOptions::setCifsUnixExtensionsSupport(bool support)
{
  d->cifsUnixExtensionsSupport = support;
}


bool Smb4KCustomOptions::cifsUnixExtensionsSupport() const
{
  return d->cifsUnixExtensionsSupport;
}


void Smb4KCustomOptions::setUseFileSystemPort(bool use)
{
  d->useFileSystemPort = use;
}


bool Smb4KCustomOptions::useFileSystemPort() const
{
  return d->useFileSystemPort;
}


void Smb4KCustomOptions::setFileSystemPort(int port)
{
  d->fileSystemPort = port;
  
  switch (d->type)
  {
    case Share:
    {
      d->url.setPort(port);
      break;
    }
    default:
    {
      break;
    }
  }
}


int Smb4KCustomOptions::fileSystemPort() const
{
  return d->fileSystemPort;
}


void Smb4KCustomOptions::setUseMountProtocolVersion(bool use)
{
  d->useMountProtocolVersion = use;
}


bool Smb4KCustomOptions::useMountProtocolVersion() const
{
  return d->useMountProtocolVersion;
}


void Smb4KCustomOptions::setMountProtocolVersion(int version)
{
  d->mountProtocolVersion = version;
}


int Smb4KCustomOptions::mountProtocolVersion() const
{
  return d->mountProtocolVersion;
}


void Smb4KCustomOptions::setUseSecurityMode(bool use)
{
  d->useSecurityMode = use;
}


bool Smb4KCustomOptions::useSecurityMode() const
{
  return d->useSecurityMode;
}


void Smb4KCustomOptions::setSecurityMode(int mode)
{
  d->securityMode = mode;
}


int Smb4KCustomOptions::securityMode() const
{
  return d->securityMode;
}


void Smb4KCustomOptions::setUseWriteAccess(bool use)
{
  d->useWriteAccess = use;
}


bool Smb4KCustomOptions::useWriteAccess() const
{
  return d->useWriteAccess;
}


void Smb4KCustomOptions::setWriteAccess(int access)
{
  d->writeAccess = access;
}


int Smb4KCustomOptions::writeAccess() const
{
  return d->writeAccess;
}
#endif


void Smb4KCustomOptions::setProfile(const QString &profile)
{
  d->profile = profile;
}


QString Smb4KCustomOptions::profile() const
{
  return d->profile;
}

void Smb4KCustomOptions::setUseClientProtocolVersions(bool use)
{
  d->useClientProtocolVersions = use;
}


bool Smb4KCustomOptions::useClientProtocolVersions() const
{
  return d->useClientProtocolVersions;
}


void Smb4KCustomOptions::setMinimalClientProtocolVersion(int version)
{
  d->minimalClientProtocolVersion = version;
}


int Smb4KCustomOptions::minimalClientProtocolVersion() const
{
  return d->minimalClientProtocolVersion;
}


void Smb4KCustomOptions::setMaximalClientProtocolVersion(int version)
{
  d->maximalClientProtocolVersion = version;
}


int Smb4KCustomOptions::maximalClientProtocolVersion() const
{
  return d->maximalClientProtocolVersion;
}


void Smb4KCustomOptions::setUseSmbPort(bool use)
{
  d->useSmbPort = use;
}


bool Smb4KCustomOptions::useSmbPort() const
{
  return d->useSmbPort;
}


void Smb4KCustomOptions::setSmbPort(int port)
{
  d->smbPort = port;

  switch (d->type)
  {
    case Host:
    {
      d->url.setPort(port);
      break;
    }
    default:
    {
      break;
    }
  }
}


int Smb4KCustomOptions::smbPort() const
{
  return d->smbPort;
}


void Smb4KCustomOptions::setUseKerberos(bool use)
{
  d->useKerberos = use;
}


bool Smb4KCustomOptions::useKerberos() const
{
  return d->useKerberos;
}


void Smb4KCustomOptions::setMACAddress(const QString &macAddress)
{
  QRegExp exp("..\\:..\\:..\\:..\\:..\\:..");
  
  if (exp.exactMatch(macAddress))
  {
    d->mac = macAddress;
  }
}


QString Smb4KCustomOptions::macAddress() const
{
  return d->mac;
}


void Smb4KCustomOptions::setWOLSendBeforeNetworkScan(bool send)
{
  d->wakeOnLanBeforeFirstScan = send;
}


bool Smb4KCustomOptions::wolSendBeforeNetworkScan() const
{
  return d->wakeOnLanBeforeFirstScan;
}


void Smb4KCustomOptions::setWOLSendBeforeMount(bool send)
{
  d->wakeOnLanBeforeMount = send;
}


bool Smb4KCustomOptions::wolSendBeforeMount() const
{
  return d->wakeOnLanBeforeMount;
}


QMap<QString, QString> Smb4KCustomOptions::customOptions() const
{
  QMap<QString,QString> entries;
  
  //
  // Remounting
  // 
  entries.insert("remount", QString("%1").arg(d->remount));
  
  //
  // User
  // 
  entries.insert("use_user", QString("%1").arg(d->useUser));
  entries.insert("uid", d->user.userId().toString());
  
  //
  // Group
  // 
  entries.insert("use_group", QString("%1").arg(d->useGroup));
  entries.insert("gid", d->group.groupId().toString());
  
  //
  // File mode
  // 
  entries.insert("use_file_mode", QString("%1").arg(d->useFileMode));
  entries.insert("file_mode", d->fileMode);
  
  //
  // Directory mode
  // 
  entries.insert("use_directory_mode", QString("%1").arg(d->useDirectoryMode));
  entries.insert("directory_mode", d->directoryMode);
  
#if defined(Q_OS_LINUX)
  //
  // Unix CIFS extensions supported
  // 
  entries.insert("cifs_unix_extensions_support", QString("%1").arg(d->cifsUnixExtensionsSupport));
  
  //
  // File system port
  // 
  entries.insert("use_filesystem_port", QString("%1").arg(d->useFileSystemPort));
  entries.insert("filesystem_port", QString("%1").arg(fileSystemPort()));
  
  // 
  // Mount protocol version
  // 
  entries.insert("use_smb_mount_protocol_version", QString("%1").arg(d->useMountProtocolVersion));
  entries.insert("smb_mount_protocol_version", QString("%1").arg(d->mountProtocolVersion));
  
  //
  // Security mode
  // 
  entries.insert("use_security_mode", QString("%1").arg(d->useSecurityMode));
  entries.insert("security_mode", QString("%1").arg(d->securityMode));
  
  //
  // Write access
  // 
  entries.insert("use_write_access", QString("%1").arg(d->useWriteAccess));
  entries.insert("write_access", QString("%1").arg(d->writeAccess));
#endif
 
  //
  // Client protocol versions
  // 
  entries.insert("use_client_protocol_versions", QString("%1").arg(d->useClientProtocolVersions));
  entries.insert("minimal_client_protocol_version", QString("%1").arg(d->minimalClientProtocolVersion));
  entries.insert("maximal_client_protocol_version", QString("%1").arg(d->maximalClientProtocolVersion));
  
  // 
  // SMB port used by the client 
  // 
  entries.insert("use_smb_port", QString("%1").arg(d->useSmbPort));
  entries.insert("smb_port", QString("%1").arg(smbPort()));
  
  // 
  // Usage of Kerberos
  // 
  entries.insert("kerberos", QString("%1").arg(d->useKerberos));
  
  //
  // MAC address
  // 
  entries.insert("mac_address", d->mac);
  
  //
  // Wake-On_LAN settings
  // 
  entries.insert("wol_send_before_first_scan", QString("%1").arg(d->wakeOnLanBeforeFirstScan));
  entries.insert("wol_send_before_mount", QString("%1").arg(d->wakeOnLanBeforeMount));
  
  return entries;
}


bool Smb4KCustomOptions::hasOptions() const
{
  //
  // NOTE: This function does not honor the workgroup, the url,
  // the ip address, the type and the profile, because these things 
  // are not custom options.
  // 
  // Perform remount
  if (d->remount != Smb4KCustomOptions::UndefinedRemount)
  {
    return true;
  }
  
  // Use user information
  if (d->useUser != Smb4KMountSettings::useUserId())
  {
    return true;
  }
  
  // User information
  if (d->user.userId().toString() != Smb4KMountSettings::userId())
  {
    return true;
  }
  
  // Use group information
  if (d->useGroup != Smb4KMountSettings::useGroupId())
  {
    return true;
  }
  
  // Group information
  if (d->group.groupId().toString() != Smb4KMountSettings::groupId())
  {
    return true;
  }
  
  // Use file mask
  if (d->useFileMode != Smb4KMountSettings::useFileMode())
  {
    return true;
  }
  
  if (d->fileMode != Smb4KMountSettings::fileMode())
  {
    return true;
  }
  
  if (d->useDirectoryMode != Smb4KMountSettings::useDirectoryMode())
  {
    return true;
  }
  
  if (d->directoryMode != Smb4KMountSettings::directoryMode())
  {
    return true;
  }
#if defined(Q_OS_LINUX)
  // CIFS Unix extension support
  if (d->cifsUnixExtensionsSupport != Smb4KMountSettings::cifsUnixExtensionsSupport())
  {
    return true;
  }
  
  // Use filesystem port
  if (d->useFileSystemPort != Smb4KMountSettings::useRemoteFileSystemPort())
  {
    return true;
  }
  
  // File system port (used for mounting)
  if (d->fileSystemPort != Smb4KMountSettings::remoteFileSystemPort())
  {
    return true;
  }
  
  // Use SMB mount protocol version
  if (d->useMountProtocolVersion != Smb4KMountSettings::useSmbProtocolVersion())
  {
    return true;
  }
  
  // SMB mount protocol version
  if (d->mountProtocolVersion != Smb4KMountSettings::smbProtocolVersion())
  {
    return true;
  }
  
  // Use security mode
  if (d->useSecurityMode != Smb4KMountSettings::useSecurityMode())
  {
    return true;
  }
  
  // Security mode
  if (d->securityMode != Smb4KMountSettings::securityMode())
  {
    return true;
  }
  
  // Use write access
  if (d->useWriteAccess != Smb4KMountSettings::useWriteAccess())
  {
    return true;
  }

  // Write access
  if (d->writeAccess != Smb4KMountSettings::writeAccess())
  {
    return true;
  }
#endif

  // Use client protocol versions
  if (d->useClientProtocolVersions != Smb4KSettings::useClientProtocolVersions())
  {
    return true;
  }
  
  // Minimal client protocol version
  if (d->minimalClientProtocolVersion != Smb4KSettings::minimalClientProtocolVersion())
  {
    return true;
  }
  
  // Maximal client protocol version
  if (d->maximalClientProtocolVersion != Smb4KSettings::maximalClientProtocolVersion())
  {
    return true;
  }

  // Use SMB port
  if (d->useSmbPort != Smb4KSettings::useRemoteSmbPort())
  {
    return true;
  }

  // SMB port
  if (d->smbPort != Smb4KSettings::remoteSmbPort())
  {
    return true;
  }

  // Kerberos
  if (d->useKerberos != Smb4KSettings::useKerberos())
  {
    return true;
  }
  
  // MAC address
  if (!d->mac.isEmpty())
  {
    return true;
  }
  
  // Send WOL packages before first scan
  if (d->wakeOnLanBeforeFirstScan)
  {
    return true;
  }
  
  // Send WOL packages before mount
  if (d->wakeOnLanBeforeMount)
  {
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
    d->mac = options->macAddress();
    d->wakeOnLanBeforeFirstScan = options->wolSendBeforeNetworkScan();
    d->wakeOnLanBeforeMount = options->wolSendBeforeMount();
}

