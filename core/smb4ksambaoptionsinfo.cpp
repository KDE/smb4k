/***************************************************************************
    smb4ksambaoptionsinfo  -  This is a container class that carries
    various information of extra options for a specific host.
                             -------------------
    begin                : Mi Okt 18 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
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

// Qt includes
#include <QHostAddress>
#include <QAbstractSocket>

// system includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4ksambaoptionsinfo.h>
#include <smb4khost.h>
#include <smb4kshare.h>


Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo( Smb4KHost *host )
: m_this_url( QUrl() ), m_host_url( QUrl() ), m_type( Host ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSD
  m_write_access( UndefinedWriteAccess ), m_fs_port( -1 ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_user( getuid() ), m_group( getgid() ), m_workgroup( host->workgroupName() ), 
  m_ip( host->ip() ), m_profile( QString() )
{
  setUNC( host->unc( QUrl::None ) );
}



Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo( Smb4KShare *share )
: m_this_url( QUrl() ), m_host_url( QUrl() ), m_type( Share ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSD
  m_write_access( UndefinedWriteAccess ), m_fs_port( share->port() ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_user( share->uid() ), m_group( share->gid() ), m_workgroup( share->workgroupName() ), 
  m_ip( share->hostIP() ), m_profile( QString() )
{
  setUNC( share->unc( QUrl::None ) );
}



Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo( const Smb4KSambaOptionsInfo &info )
: m_this_url( QUrl() ), m_host_url( QUrl() ), m_type( info.type() ), m_remount( info.remount() ),
#ifndef Q_OS_FREEBSD
  m_write_access( info.writeAccess() ), m_fs_port( info.fileSystemPort() ),
#endif
  m_protocol( info.protocol() ), m_kerberos( info.useKerberos() ),
  m_user( info.uid() ), m_group( info.gid() ), m_workgroup( info.workgroupName() ), 
  m_ip( info.ip() ), m_profile( info.profile() )
{
  setUNC( info.unc( QUrl::None ) );
}


Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo()
: m_this_url( QUrl() ), m_host_url( QUrl() ), m_type( Unknown ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSDs
  m_write_access( UndefinedWriteAccess ), m_fs_port( -1 ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_user( getuid() ), m_group( getgid() ), m_workgroup( QString() ), 
  m_ip( QString() ), m_profile( QString() )
{
}


Smb4KSambaOptionsInfo::~Smb4KSambaOptionsInfo()
{
}


void Smb4KSambaOptionsInfo::setRemount( Smb4KSambaOptionsInfo::Remount remount )
{
  m_remount = remount;
}


QString Smb4KSambaOptionsInfo::shareName() const
{
  if ( m_type == Share )
  {
    if ( m_this_url.path().startsWith( "/" ) )
    {
      return m_this_url.path().remove( 0, 1 );
    }
    else
    {
      // Do nothing
    }

    return m_this_url.path();
  }
  else
  {
    // Do nothing
  }
  
  return QString();
}


void Smb4KSambaOptionsInfo::setUNC( const QString &unc )
{
  m_this_url.setUrl( unc, QUrl::TolerantMode );
  
  if ( m_this_url.scheme().isEmpty() )
  {
    m_this_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }

  if ( m_this_url.path().contains( "/" ) == 1 )
  {
    m_type = Share;
    m_host_url = m_this_url;
    m_host_url.setPort( -1 );
    m_host_url.setPath( QString() );
  }
  else
  {
    m_type = Host;
    m_host_url = m_this_url;
  }
}


QString Smb4KSambaOptionsInfo::unc( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  switch ( m_type )
  {
    case Host:
    {
      if ( (options & QUrl::RemoveUserInfo) || m_this_url.userName().isEmpty() )
      {
        unc = m_this_url.toString( options|QUrl::RemovePath ).replace( "//"+m_this_url.host(), "//"+hostName() );
      }
      else
      {
        unc = m_this_url.toString( options|QUrl::RemovePath ).replace( "@"+m_this_url.host(), "@"+hostName() );
      }
      break;
    }
    case Share:
    {
      if ( (options & QUrl::RemoveUserInfo) || m_host_url.userName().isEmpty() )
      {
        unc = m_this_url.toString( options ).replace( "//"+m_this_url.host(), "//"+hostName() );
      }
      else
      {
        unc = m_this_url.toString( options ).replace( "@"+m_this_url.host(), "@"+hostName() );
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


QString Smb4KSambaOptionsInfo::hostUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || m_host_url.userName().isEmpty() )
  {
    unc = m_host_url.toString( options|QUrl::RemovePath ).replace( "//"+m_host_url.host(), "//"+hostName() );
  }
  else
  {
    unc = m_host_url.toString( options|QUrl::RemovePath ).replace( "@"+m_host_url.host(), "@"+hostName() );
  }
  
  return unc;
}


void Smb4KSambaOptionsInfo::setSMBPort( int port )
{
  m_host_url.setPort( port );
}


int Smb4KSambaOptionsInfo::smbPort() const
{
  // Under FreeBSD we could also return m_this_url.port() here,
  // because the two ports are equal.
  return m_host_url.port();
}


#ifndef Q_OS_FREEBSD
void Smb4KSambaOptionsInfo::setFileSystemPort( int port )
{
  switch ( m_type )
  {
    case Share:
    {
      m_this_url.setPort( port );
      break;
    }
    case Host:
    {
      m_fs_port = port;
      break;
    }
    default:
    {
      break;
    }
  }
}


int Smb4KSambaOptionsInfo::fileSystemPort() const
{
  return m_type == Share ? m_this_url.port() : m_fs_port;
}
#endif


void Smb4KSambaOptionsInfo::setProtocol( Smb4KSambaOptionsInfo::Protocol protocol )
{
  m_protocol = protocol;
}


void Smb4KSambaOptionsInfo::setUseKerberos( Smb4KSambaOptionsInfo::Kerberos kerberos )
{
  m_kerberos = kerberos;
}


void Smb4KSambaOptionsInfo::setUID( K_UID uid )
{
  m_user = KUser( uid );
}


void Smb4KSambaOptionsInfo::setGID( K_GID gid )
{
  m_group = KUserGroup( gid );
}

#ifndef Q_OS_FREEBSD

void Smb4KSambaOptionsInfo::setWriteAccess( Smb4KSambaOptionsInfo::WriteAccess write_access )
{
  m_write_access = write_access;
}

#endif


void Smb4KSambaOptionsInfo::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KSambaOptionsInfo::setIP( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    m_ip.clear();
  }
  else
  {
    // The IP address is OK.
    m_ip = ip;
  }
}


void Smb4KSambaOptionsInfo::update( Smb4KSambaOptionsInfo *info )
{
  // UNC, workgroup and IP address are not updated.
  switch ( info->type() )
  {
    case Host:
    {
      m_this_url.setPort( info->smbPort() );
      m_host_url.setPort( info->smbPort() );
#ifndef Q_OS_FREEBSD
      m_fs_port = info->fileSystemPort();
#endif
      break;
    }
    case Share:
    {
      m_this_url.setPort( info->fileSystemPort() );
      m_host_url.setPort( info->smbPort() );
      break;
    }
    default:
    {
      break;
    }
  }

  m_remount      = info->remount();
#ifndef Q_OS_FREEBSD
  m_write_access = info->writeAccess();
#endif
  m_protocol     = info->protocol();
  m_kerberos     = info->useKerberos();
  m_user         = KUser( info->uid() );
  m_group        = KUserGroup( info->gid() );
}


QMap<QString,QString> Smb4KSambaOptionsInfo::entries()
{
  QMap<QString,QString> entries;

  switch ( m_remount )
  {
    case DoRemount:
    {
      entries.insert( "remount", "true" );
      break;
    }
    case NoRemount:
    {
      entries.insert( "remount", "false" );
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

  entries.insert( "smb_port", smbPort() != -1 ? QString( "%1" ).arg( smbPort() ) : QString() );
#ifndef Q_OS_FREEBSD
  entries.insert( "filesystem_port", fileSystemPort() != -1 ? QString( "%1" ).arg( fileSystemPort() ) : QString() );
#endif

  switch ( m_protocol )
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
    case UndefinedProtocol:
    {
      entries.insert( "protocol", QString() );

      break;
    }
    default:
    {
      break;
    }
  }

#ifndef Q_OS_FREEBSD
  switch ( m_write_access )
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
#endif

  switch ( m_kerberos )
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
  
  entries.insert( "uid", QString( "%1" ).arg( m_user.uid() ) );
  entries.insert( "owner", m_user.loginName() );
  entries.insert( "gid", QString( "%1" ).arg( m_group.gid() ) );
  entries.insert( "group", m_group.name() );

  return entries;
}


void Smb4KSambaOptionsInfo::setProfile( const QString &name )
{
  m_profile = name;
}


bool Smb4KSambaOptionsInfo::equals( Smb4KSambaOptionsInfo* info ) const
{
  if ( QString::compare( unc( QUrl::None ), info->unc( QUrl::None ), Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  if ( fileSystemPort() != info->fileSystemPort() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
#endif
  
  if ( QString::compare( workgroupName(), info->workgroupName(), Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( QString::compare( ip(), info->ip() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( QString::compare( profile(), info->profile() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( QString::compare( owner(), info->owner() ) != 0 || uid() != info->uid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( QString::compare( group(), info->group() ) != 0 || gid() != info->gid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( type() != info->type() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( remount() != info->remount() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  if ( writeAccess() != info->writeAccess() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
#endif

  if ( protocol() != info->protocol() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  if ( useKerberos() != info->useKerberos() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


