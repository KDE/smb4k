/***************************************************************************
    smb4kcustomoptions - This class carries custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2014 by Alexander Reinholdt
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kcustomoptions.h"

// Qt includes
#include <QtCore/QDebug>
#include <QtNetwork/QHostAddress>

// KDE includes
#include <kuser.h>

// system specific includes
#include <unistd.h>
#include <sys/types.h>


class Smb4KCustomOptionsPrivate
{
  public:
    QString workgroup;
    KUrl url;
    QHostAddress ip;
    NetworkItem type;
    Smb4KCustomOptions::Remount remount;
    QString profile;
    int smbPort;
#ifndef Q_OS_FREEBSD
    int fileSystemPort;
    Smb4KCustomOptions::WriteAccess writeAccess;
    Smb4KCustomOptions::SecurityMode securityMode;
#endif
    Smb4KCustomOptions::ProtocolHint protocolHint;
    Smb4KCustomOptions::Kerberos kerberos;
    KUser user;
    KUserGroup group;
    QString mac;
    bool wol_first_scan;
    bool wol_mount;
};


Smb4KCustomOptions::Smb4KCustomOptions( Smb4KHost *host )
: d( new Smb4KCustomOptionsPrivate )
{
  d->workgroup      = host->workgroupName();
  d->url            = host->url();
  d->type           = Host;
  d->remount        = UndefinedRemount;
  d->smbPort        = host->port() != -1 ? host->port() : 139;
#ifndef Q_OS_FREEBSD
  d->fileSystemPort = 445;
  d->writeAccess    = UndefinedWriteAccess;
  d->securityMode   = UndefinedSecurityMode;
#endif
  d->protocolHint   = UndefinedProtocolHint;
  d->kerberos       = UndefinedKerberos;
  d->user           = KUser( getuid() );
  d->group          = KUserGroup( getgid() );
  d->ip.setAddress( host->ip() );
  d->wol_first_scan = false;
  d->wol_mount      = false;
}

Smb4KCustomOptions::Smb4KCustomOptions( Smb4KShare *share )
: d( new Smb4KCustomOptionsPrivate )
{
  d->url            = share->url();
  d->workgroup      = share->workgroupName();
  d->type           = Share;
  d->remount        = UndefinedRemount;
  d->smbPort        = 139;
#ifndef Q_OS_FREEBSD
  d->fileSystemPort = share->port() != -1 ? share->port() : 445;
  d->writeAccess    = UndefinedWriteAccess;
  d->securityMode   = UndefinedSecurityMode;
#endif
  d->protocolHint   = UndefinedProtocolHint;
  d->kerberos       = UndefinedKerberos;
  d->user           = KUser( share->uid() );
  d->group          = KUserGroup( share->gid() );
  d->ip.setAddress( share->hostIP() );
  d->wol_first_scan = false;
  d->wol_mount      = false;
}


Smb4KCustomOptions::Smb4KCustomOptions( const Smb4KCustomOptions &o )
: d( new Smb4KCustomOptionsPrivate )
{
  *d = *o.d;
}


Smb4KCustomOptions::Smb4KCustomOptions()
: d( new Smb4KCustomOptionsPrivate )
{
  d->type           = Unknown;
  d->remount        = UndefinedRemount;
  d->smbPort        = 139;
#ifndef Q_OS_FREEBSD
  d->fileSystemPort = 445;
  d->writeAccess    = UndefinedWriteAccess;
  d->securityMode   = UndefinedSecurityMode;
#endif
  d->protocolHint   = UndefinedProtocolHint;
  d->kerberos       = UndefinedKerberos;
  d->user           = KUser( getuid() );
  d->group          = KUserGroup( getgid() );
  d->wol_first_scan = false;
  d->wol_mount      = false;
}


Smb4KCustomOptions::~Smb4KCustomOptions()
{
}


void Smb4KCustomOptions::setHost( Smb4KHost *host )
{
  Q_ASSERT( host );
  
  switch ( d->type )
  {
    case Unknown:
    {
      d->workgroup      = host->workgroupName();
      d->url            = host->url();
      d->type           = Host;
      d->smbPort        = host->port() != -1 ? host->port() : 139;
#ifndef Q_OS_FREEBSD
      d->fileSystemPort = 445;
#endif
      d->ip.setAddress( host->ip() );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptions::setShare( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  switch ( d->type )
  {
    case Unknown:
    {
      d->url            = share->url();
      d->workgroup      = share->workgroupName();
      d->type           = Share;
      d->smbPort        = 139;
#ifndef Q_OS_FREEBSD
      d->fileSystemPort = share->port() != -1 ? share->port() : 445;
#endif
      d->user           = KUser( share->uid() );
      d->group          = KUserGroup( share->gid() );
      d->ip.setAddress( share->hostIP() );
      break;
    }
    case Host:
    {
      if ( QString::compare( unc(), share->hostUNC(), Qt::CaseInsensitive ) == 0 )
      {
        d->url            = share->url();
        d->type           = Share;
#ifndef Q_OS_FREEBSD
        d->fileSystemPort = share->port() != -1 ? share->port() : d->fileSystemPort;
#endif
        d->user           = KUser( share->uid() );
        d->group          = KUserGroup( share->gid() );
        d->ip.setAddress( share->hostIP() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


Smb4KGlobal::NetworkItem Smb4KCustomOptions::type() const
{
  return d->type;
}


void Smb4KCustomOptions::setRemount( Smb4KCustomOptions::Remount remount )
{
  switch ( d->type )
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


Smb4KCustomOptions::Remount Smb4KCustomOptions::remount() const
{
  return d->remount;
}


void Smb4KCustomOptions::setProfile( const QString &profile )
{
  d->profile = profile;
}


QString Smb4KCustomOptions::profile() const
{
  return d->profile;
}


void Smb4KCustomOptions::setWorkgroupName( const QString &workgroup )
{
  d->workgroup = workgroup;
}

QString Smb4KCustomOptions::workgroupName() const
{
  return d->workgroup;
}


void Smb4KCustomOptions::setURL( const KUrl &url )
{
  d->url = url;
}


void Smb4KCustomOptions::setURL( const QString &url )
{
  d->url.setUrl( url, KUrl::TolerantMode );
  d->url.setProtocol( "smb" );
}


KUrl Smb4KCustomOptions::url() const
{
  return d->url;
}


QString Smb4KCustomOptions::unc() const
{
  QString unc;
  
  switch ( d->type )
  {
    case Host:
    {
      if ( !hostName().isEmpty() )
      {
        unc = QString( "//%1" ).arg( hostName() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Share:
    {
      if ( !hostName().isEmpty() && !shareName().isEmpty() )
      {
        unc = QString( "//%1/%2" ).arg( hostName() ).arg( shareName() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }

  return unc;
}


QString Smb4KCustomOptions::hostName() const
{
  return d->url.host().toUpper();
}


QString Smb4KCustomOptions::shareName() const
{
  if ( d->url.path().startsWith( '/' ) )
  {
    return d->url.path().remove( 0, 1 );
  }
  else
  {
    // Do nothing
  }

  return d->url.path();
}


void Smb4KCustomOptions::setIP( const QString &ip )
{
  d->ip.setAddress( ip );
}


QString Smb4KCustomOptions::ip() const
{
  return d->ip.toString();
}


void Smb4KCustomOptions::setSMBPort( int port )
{
  d->smbPort = port;

  switch ( d->type )
  {
    case Host:
    {
      d->url.setPort( port );
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


#ifndef Q_OS_FREEBSD
void Smb4KCustomOptions::setFileSystemPort( int port )
{
  d->fileSystemPort = port;
  
  switch ( d->type )
  {
    case Share:
    {
      d->url.setPort( port );
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


void Smb4KCustomOptions::setWriteAccess( Smb4KCustomOptions::WriteAccess access )
{
  d->writeAccess = access;
}


Smb4KCustomOptions::WriteAccess Smb4KCustomOptions::writeAccess() const
{
  return d->writeAccess;
}


void Smb4KCustomOptions::setSecurityMode(Smb4KCustomOptions::SecurityMode mode)
{
  d->securityMode = mode;
}


Smb4KCustomOptions::SecurityMode Smb4KCustomOptions::securityMode() const
{
  return d->securityMode;
}
#endif


void Smb4KCustomOptions::setProtocolHint( Smb4KCustomOptions::ProtocolHint protocol )
{
  d->protocolHint = protocol;
}


Smb4KCustomOptions::ProtocolHint Smb4KCustomOptions::protocolHint() const
{
  return d->protocolHint;
}


void Smb4KCustomOptions::setUseKerberos( Smb4KCustomOptions::Kerberos kerberos )
{
  d->kerberos = kerberos;
}


Smb4KCustomOptions::Kerberos Smb4KCustomOptions::useKerberos() const
{
  return d->kerberos;
}


void Smb4KCustomOptions::setUID( K_UID uid )
{
  d->user = KUser( uid );
}


K_UID Smb4KCustomOptions::uid() const
{
  return d->user.uid();
}


QString Smb4KCustomOptions::owner() const
{
  return d->user.loginName();
}


void Smb4KCustomOptions::setGID( K_GID gid )
{
  d->group = KUserGroup( gid );
}


K_GID Smb4KCustomOptions::gid() const
{
  return d->group.gid();
}


QString Smb4KCustomOptions::group() const
{
  return d->group.name();
}


void Smb4KCustomOptions::setMACAddress( const QString &macAddress )
{
  d->mac = macAddress;
}


QString Smb4KCustomOptions::macAddress() const
{
  return d->mac;
}


void Smb4KCustomOptions::setWOLSendBeforeNetworkScan( bool send )
{
  d->wol_first_scan = send;
}


bool Smb4KCustomOptions::wolSendBeforeNetworkScan() const
{
  return d->wol_first_scan;
}


void Smb4KCustomOptions::setWOLSendBeforeMount( bool send )
{
  d->wol_mount = send;
}


bool Smb4KCustomOptions::wolSendBeforeMount() const
{
  return d->wol_mount;
}


QMap<QString, QString> Smb4KCustomOptions::customOptions() const
{
  QMap<QString,QString> entries;

  switch ( d->remount )
  {
    case RemountOnce:
    {
      entries.insert( "remount", "once" );
      break;
    }
    case RemountAlways:
    {
      entries.insert( "remount", "always" );
      break;
    }
    case RemountNever:
    {
      entries.insert( "remount", "never" );
      break;      
    }
    case UndefinedRemount:
    {
      entries.insert( "remount", QString() );
      break;
    }
    default:
    {
      break;
    }
  }

  entries.insert( "smb_port", QString( "%1" ).arg( smbPort() ) );
#ifdef Q_OS_LINUX
  entries.insert( "filesystem_port", QString( "%1" ).arg( fileSystemPort() ) );
  
  switch ( d->writeAccess )
  {
    case ReadWrite:
    {
      entries.insert( "write_access", "true" );
      break;
    }
    case ReadOnly:
    {
      entries.insert( "write_access", "false" );
      break;
    }
    case UndefinedWriteAccess:
    {
      entries.insert( "write_access", QString() );
      break;
    }
    default:
    {
      break;
    }
  }
  
  switch ( d->securityMode )
  {
    case NoSecurityMode:
    {
      entries.insert( "security_mode", "none" );
      break;
    }
    case Krb5:
    {
      entries.insert( "security_mode", "krb5" );
      break;
    }
    case Krb5i:
    {
      entries.insert( "security_mode", "krb5i" );
      break;
    }
    case Ntlm:
    {
      entries.insert( "security_mode", "ntlm" );
      break;
    }
    case Ntlmi:
    {
      entries.insert( "security_mode", "ntlmi" );
      break;
    }
    case Ntlmv2:
    {
      entries.insert( "security_mode", "ntlmv2" );
      break;
    }
    case Ntlmv2i:
    {
      entries.insert( "security_mode", "ntlmv2i" );
      break;
    }
    case Ntlmssp:
    {
      entries.insert( "security_mode", "ntlmssp" );
      break;
    }
    case Ntlmsspi:
    {
      entries.insert( "security_mode", "ntlmsspi" );
      break;
    }
    case UndefinedSecurityMode:
    {
      entries.insert( "security_mode", QString() );
      break;
    }
    default:
    {
      break;
    }
  }
#endif

  switch ( d->protocolHint )
  {
    case Automatic:
    {
      entries.insert( "protocol", "auto" );
      break;
    }
    case RPC:
    {
      entries.insert( "protocol", "rpc" );
      break;
    }
    case RAP:
    {
      entries.insert( "protocol", "rap" );
      break;
    }
    case ADS:
    {
      entries.insert( "protocol", "ads" );
      break;
    }
    case UndefinedProtocolHint:
    {
      entries.insert( "protocol", QString() );
      break;
    }
    default:
    {
      break;
    }
  }

  switch ( d->kerberos )
  {
    case UseKerberos:
    {
      entries.insert( "kerberos", "true" );
      break;
    }
    case NoKerberos:
    {
      entries.insert( "kerberos", "false" );
      break;
    }
    case UndefinedKerberos:
    {
      entries.insert( "kerberos", QString() );
      break;
    }
    default:
    {
      break;
    }
  }
  
  entries.insert( "uid", QString( "%1" ).arg( d->user.uid() ) );
  entries.insert( "owner", d->user.loginName() );
  entries.insert( "gid", QString( "%1" ).arg( d->group.gid() ) );
  entries.insert( "group", d->group.name() );
  entries.insert( "mac_address", d->mac );
  entries.insert( "wol_send_before_first_scan", d->wol_first_scan ? "true" : "false" );
  entries.insert( "wol_send_before_mount", d->wol_mount ? "true" : "false" );
  
  return entries;
}


bool Smb4KCustomOptions::equals(Smb4KCustomOptions *options, bool fullCheck) const
{
  // Type
  if (d->type != options->type())
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Profile
  if (QString::compare(d->profile, options->profile()) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if (QString::compare(d->workgroup, options->workgroupName(), Qt::CaseInsensitive) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // UNC
  if (QString::compare(unc(), options->unc(), Qt::CaseInsensitive) != 0)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Full check
  if (fullCheck)
  {
    // IP address
    if (QString::compare(d->ip.toString(), options->ip()) != 0)
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // Remounting
    if (d->remount != options->remount())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // SMB port
    if (d->smbPort != options->smbPort())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
#ifdef Q_OS_LINUX
    // File system port (used for mounting)
    if (d->fileSystemPort != options->fileSystemPort())
    {
      return false;
    }
    else
    {
      // Do nothing
    }

    // Write access
    if (d->writeAccess != options->writeAccess())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // Security mode
    if (d->securityMode != options->securityMode())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
#endif

    // Protocol hint
    if (d->protocolHint != options->protocolHint())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // Kerberos
    if (d->kerberos != options->useKerberos())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // UID
    if (d->user.uid() != options->uid())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // GID
    if (d->group.gid() != options->gid())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // MAC address
    if (QString::compare(d->mac, options->macAddress()) != 0)
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // Send WOL packages before first scan
    if (d->wol_first_scan != options->wolSendBeforeNetworkScan())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
    
    // Send WOL packages before mount
    if (d->wol_mount != options->wolSendBeforeMount())
    {
      return false;
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


bool Smb4KCustomOptions::isEmpty()
{
  // Type
  if ( d->type != Unknown )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Profile
  if ( !d->profile.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if ( !d->workgroup.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // URL
  if ( !d->url.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // IP address
  if ( !d->ip.isNull() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Remounting
  if ( d->remount != Smb4KCustomOptions::UndefinedRemount )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // SMB port
  if ( d->smbPort != 139 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
#ifdef Q_OS_LINUX
  // File system port (used for mounting)
  if ( d->fileSystemPort != 445 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Write access
  if ( d->writeAccess != UndefinedWriteAccess )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Security mode
  if (d->securityMode != Smb4KCustomOptions::UndefinedSecurityMode)
  {
    return false;
  }
  else
  {
    // Do nothing
  }
#endif

  // Protocol hint
  if ( d->protocolHint != UndefinedProtocolHint )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Kerberos
  if ( d->kerberos != UndefinedKerberos )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if ( d->user.uid() != getuid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // GID
  if ( d->group.gid() != getgid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // MAC address
  if ( !d->mac.isNull() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Send WOL packages before first scan
  if ( d->wol_first_scan )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Send WOL packages before mount
  if ( d->wol_mount )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}

