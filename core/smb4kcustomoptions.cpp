/***************************************************************************
    smb4kcustomoptions - This class carries custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QDebug>

// system specific includes
#include <unistd.h>
#include <sys/types.h>

// application specific includes
#include <smb4kcustomoptions.h>


Smb4KCustomOptions::Smb4KCustomOptions( Smb4KHost *host )
: m_host( *host ), m_share( Smb4KShare() ), m_type( Host ), m_remount( UndefinedRemount ),
  m_profile( QString() ), m_smb_port( (host->port() != -1 ? host->port() : 139) ),
#ifndef Q_OS_FREEBSD  
  m_fs_port( 445 ), m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocolHint ), m_kerberos( UndefinedKerberos ), m_user( getuid() ),
  m_group( getgid() )
{
}

Smb4KCustomOptions::Smb4KCustomOptions( Smb4KShare *share )
: m_host( Smb4KHost() ), m_share( *share ), m_type( Share ), m_remount( UndefinedRemount ),
  m_profile( QString() ), m_smb_port( 139 ),
#ifndef Q_OS_FREEBSD
  m_fs_port( (share->port() != -1 ? share->port() : 445) ), m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocolHint ), m_kerberos( UndefinedKerberos ), m_user( share->uid() ),
  m_group( share->gid() )
{
}


Smb4KCustomOptions::Smb4KCustomOptions( const Smb4KCustomOptions &o )
: m_host( *o.host() ), m_share( *o.share() ), m_type( o.type() ), m_remount( o.remount() ),
  m_profile( o.profile() ), m_smb_port( o.smbPort() ), 
#ifndef Q_OS_FREEBSD
  m_fs_port( o.fileSystemPort() ), m_write_access( o.writeAccess() ),
#endif
  m_protocol( o.protocolHint() ), m_kerberos( o.useKerberos() ), m_user( o.uid() ),
  m_group( o.gid() )
{
}


Smb4KCustomOptions::Smb4KCustomOptions()
: m_host( Smb4KHost() ), m_share( Smb4KShare() ), m_type( Unknown ), m_remount( UndefinedRemount ),
  m_profile( QString() ), m_smb_port( 139 ),
#ifndef Q_OS_FREEBSD
  m_fs_port( 445 ), m_write_access( UndefinedWriteAccess ),
#endif
  m_protocol( UndefinedProtocolHint ), m_kerberos( UndefinedKerberos ), m_user( getuid() ),
  m_group( getgid() )
{
}


Smb4KCustomOptions::~Smb4KCustomOptions()
{
}


void Smb4KCustomOptions::setHost( Smb4KHost *host )
{
  Q_ASSERT( host );
  
  switch ( m_type )
  {
    case Unknown:
    {
      m_type = Host;
      m_host = *host;
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
  
  switch ( m_type )
  {
    case Unknown:
    {
      m_type = Share;
      m_share = *share;
      break;
    }
    case Host:
    {
      if ( QString::compare( m_host.hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( m_host.workgroupName(), share->workgroupName(), Qt::CaseInsensitive ) == 0 )
      {
        m_type = Share;
        m_host = Smb4KHost();
        m_share = *share;
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


void Smb4KCustomOptions::setRemount( Smb4KCustomOptions::Remount remount )
{
  switch ( m_type )
  {
    case Share:
    {
      m_remount = remount;
      break;
    }
    default:
    {
      m_remount = UndefinedRemount;
      break;
    }
  }
}


void Smb4KCustomOptions::setProfile( const QString &profile )
{
  m_profile = profile;
}


void Smb4KCustomOptions::setWorkgroupName( const QString &workgroup )
{
  switch ( m_type )
  {
    case Host:
    {
      m_host.setWorkgroupName( workgroup );
      break;
    }
    case Share:
    {
      m_share.setWorkgroupName( workgroup );
      break;
    }
    default:
    {
      break;
    }
  }
}

const QString Smb4KCustomOptions::workgroupName() const
{
  switch ( m_type )
  {
    case Host:
    {
      return m_host.workgroupName();
    }
    case Share:
    {
      return m_share.workgroupName();
    }
    default:
    {
      break;
    }
  }
  return QString();
}


void Smb4KCustomOptions::setURL( const QUrl &url )
{
  switch ( m_type )
  {
    case Host:
    {
      m_host.setURL( url );
      break;
    }
    case Share:
    {
      m_share.setURL( url );
      break;
    }
    default:
    {
      break;
    }
  }
}


const QUrl Smb4KCustomOptions::url() const
{
  QUrl url;
  
  switch ( m_type )
  {
    case Host:
    {
      url = m_host.url();
      break;
    }
    case Share:
    {
      url = m_share.url();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return url;
}


void Smb4KCustomOptions::setIP( const QString &ip )
{
  switch ( m_type )
  {
    case Host:
    {
      m_host.setIP( ip );
      break;
    }
    case Share:
    {
      m_share.setHostIP( ip );
      break;
    }
    default:
    {
      break;
    }
  }
}


const QString Smb4KCustomOptions::ip() const
{
  switch ( m_type )
  {
    case Host:
    {
      return m_host.ip();
    }
    case Share:
    {
      return m_share.hostIP();
    }
    default:
    {
      break;
    }
  }
  return QString();
}


void Smb4KCustomOptions::setSMBPort( int port )
{
  m_smb_port = port;

  switch ( m_type )
  {
    case Host:
    {
      m_host.setPort( m_smb_port );
      break;
    }
    default:
    {
      break;
    }
  }
}


#ifndef Q_OS_FREEBSD
void Smb4KCustomOptions::setFileSystemPort( int port )
{
  m_fs_port = port;
  
  switch ( m_type )
  {
    case Share:
    {
      m_share.setPort( m_fs_port );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KCustomOptions::setWriteAccess( Smb4KCustomOptions::WriteAccess access )
{
  m_write_access = access;
}
#endif


void Smb4KCustomOptions::setProtocolHint( Smb4KCustomOptions::ProtocolHint protocol )
{
  m_protocol = protocol;
}


void Smb4KCustomOptions::setUseKerberos( Smb4KCustomOptions::Kerberos kerberos )
{
  m_kerberos = kerberos;
}


void Smb4KCustomOptions::setUID( K_UID uid )
{
  m_user = KUser( uid );
}


void Smb4KCustomOptions::setGID( K_GID gid )
{
  m_group = KUserGroup( gid );
}


QMap<QString, QString> Smb4KCustomOptions::customOptions()
{
  QMap<QString, QString> options;
  
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

  entries.insert( "smb_port", QString( "%1" ).arg( smbPort() ) );
#ifndef Q_OS_FREEBSD
  entries.insert( "filesystem_port", QString( "%1" ).arg( fileSystemPort() ) );
  
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
  
  return options;
}


bool Smb4KCustomOptions::equals( Smb4KCustomOptions *options ) const
{
  // Type
  if ( m_type != options->type() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Profile
  if ( QString::compare( profile(), options->profile() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if ( QString::compare( workgroupName(), options->workgroupName(), Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // URL - Instead of checking if the whole network item equals
  //  the one defined here, it is sufficient to check the URL.
  if ( url() != options->url() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // IP address
  if ( QString::compare( ip(), options->ip() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // SMB port
  if ( smbPort() != options->smbPort() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
#ifndef Q_OS_FREEBSD
  
  // File system port (used for mounting)
  if ( fileSystemPort() != options->fileSystemPort() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // Write access
  if ( writeAccess() != options->writeAccess() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
#endif

  // Protocol hint
  if ( protocolHint() != options->protocolHint() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Kerberos
  if ( useKerberos() != options->useKerberos() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // UID
  if ( uid() != options->uid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // GID
  if ( gid() != options->gid() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  return true;
}




