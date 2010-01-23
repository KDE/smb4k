/***************************************************************************
    smb4ksambaoptionsinfo  -  This is a container class that carries
    various information of extra options for a specific host.
                             -------------------
    begin                : Mi Okt 18 2006
    copyright            : (C) 2006-2009 by Alexander Reinholdt
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
: m_url( QUrl() ), m_type( Host ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSD
  m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_uid( getuid() ), m_gid( getgid() ), m_workgroup( host->workgroupName() ), 
  m_ip( host->ip() ), m_profile( QString() )
{
  setUNC( host->unc( QUrl::None ) );
}



Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo( Smb4KShare *share )
: m_url( QUrl() ), m_type( Share ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSD
  m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_uid( share->uid() ), m_gid( share->gid() ), m_workgroup( share->workgroupName() ), 
  m_ip( share->hostIP() ), m_profile( QString() )
{
  setUNC( share->unc( QUrl::None ) );
}



Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo( const Smb4KSambaOptionsInfo &info )
: m_url( QUrl() ), m_type( info.type() ), m_remount( info.remount() ),
#ifndef Q_OS_FREEBSD
  m_write_access( info.writeAccess() ),
#endif
  m_protocol( info.protocol() ), m_kerberos( info.useKerberos() ),
  m_uid( info.uid() ), m_gid( info.gid() ), m_workgroup( info.workgroupName() ), 
  m_ip( info.ip() ), m_profile( info.profile() )
{
  setUNC( info.unc( QUrl::None ) );
}


Smb4KSambaOptionsInfo::Smb4KSambaOptionsInfo()
: m_url( QUrl() ), m_type( Unknown ), m_remount( UndefinedRemount ),
#ifndef Q_OS_FREEBSDs
  m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocol ), m_kerberos( UndefinedKerberos ),
  m_uid( getuid() ), m_gid( getgid() ), m_workgroup( QString() ), 
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
  if ( m_url.path().startsWith( "/" ) )
  {
    return m_url.path().remove( 0, 1 );
  }
  else
  {
    // Do nothing
  }

  return m_url.path();
}


void Smb4KSambaOptionsInfo::setUNC( const QString &unc )
{
  // Check that a valid UNC was passed to this function.
  if ( !unc.startsWith( "//" ) && !unc.startsWith( "smb:" ) && !(unc.count( "/" ) == 2 || unc.count( "/" ) == 3) )
  {
    // The UNC is malformatted.
    return;
  }
  else
  {
    // Set the type.
    if ( unc.count( "/" ) == 3 )
    {
      m_type = Share;
    }
    else
    {
      m_type = Host;
    }
  }
  

  m_url.setUrl( unc );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KSambaOptionsInfo::unc( QUrl::FormattingOptions options ) const
{
  switch ( m_type )
  {
    case Host:
    {
      return m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
    }
    case Share:
    {
      return m_url.toString( options ).replace( "//"+m_url.host(), "//"+hostName() );
    }
    default:
    {
      break;
    }
  }

  return QString();
}


QString Smb4KSambaOptionsInfo::hostUNC( QUrl::FormattingOptions options ) const
{
  return m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
}


void Smb4KSambaOptionsInfo::setPort( int port )
{
  m_url.setPort( port );
}


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
  m_uid = uid;
}


void Smb4KSambaOptionsInfo::setGID( K_GID gid )
{
  m_gid = gid;
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

  m_url.setPort( info->port() );

  m_remount      = info->remount();
#ifndef Q_OS_FREEBSD
  m_write_access = info->writeAccess();
#endif
  m_protocol     = info->protocol();
  m_kerberos     = info->useKerberos();
  m_uid          = info->uid();
  m_gid          = info->gid();
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

  entries.insert( "port", m_url.port() != -1 ?
                          QString( "%1" ).arg( m_url.port() ) :
                          QString() );

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
  
  switch ( m_type )
  {
    case Host:
    {
      entries.insert( "uid", QString() );
      entries.insert( "gid", QString() );
      break;
    }
    case Share:
    {
      entries.insert( "uid", QString( "%1" ).arg( m_uid ) );
      entries.insert( "gid", QString( "%1" ).arg( m_gid ) );
      break;
    }
    default:
    {
      break;
    }
  }

  return entries;
}


void Smb4KSambaOptionsInfo::setProfile( const QString &name )
{
  m_profile = name;
}

