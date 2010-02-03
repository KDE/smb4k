/***************************************************************************
    smb4kshare  -  Smb4K's container class for information about a share.
                             -------------------
    begin                : Mo Jan 28 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

// Qt include
#include <QHostAddress>
#include <QAbstractSocket>
#include <QDir>

// KDE includes
#include <klocale.h>
#include <kdebug.h>

// system includes
#include <unistd.h>
#include <math.h>

// application specific includes
#include <smb4kshare.h>
#include <smb4kauthinfo.h>

Smb4KShare::Smb4KShare( const QString &host, const QString &name ) : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( QString() ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QByteArray() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false ), m_homes_share( false ), m_homes_users( QStringList() )
{
  m_url.setHost( host );
  m_url.setPath( name );
  m_url.setScheme( "smb" );

  m_homes_share = (QString::compare( name, "homes", Qt::CaseSensitive ) == 0);
}


Smb4KShare::Smb4KShare( const QString &unc ) : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( QString() ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QByteArray() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false ), m_homes_share( false ), m_homes_users( QStringList() )
{
  setUNC( unc );
}


Smb4KShare::Smb4KShare( const Smb4KShare &s ) : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( s.workgroupName() ), m_type_string( s.typeString() ), m_comment( s.comment() ),
  m_host_ip( s.hostIP() ), m_path( s.path() ), m_inaccessible( s.isInaccessible() ),
  m_foreign( s.isForeign() ), m_filesystem( s.fileSystem() ), m_user( s.uid() ), m_group( s.gid() ),
  m_total( s.totalDiskSpace() ), m_free( s.freeDiskSpace() ), m_used( s.usedDiskSpace() ),
  m_is_mounted( s.isMounted() ), m_homes_share( s.isHomesShare() ), m_homes_users( s.homesUsers() )
{
  setUNC( s.unc( QUrl::None ) );
}



Smb4KShare::Smb4KShare() : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( QString() ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QByteArray() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false ), m_homes_share( false ), m_homes_users( QStringList() )
{
}


Smb4KShare::~Smb4KShare()
{
}


void Smb4KShare::setShareName( const QString &name )
{
  m_url.setPath( name.trimmed() );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }

  m_homes_share = (QString::compare( name, "homes", Qt::CaseSensitive ) == 0);
}


QString Smb4KShare::shareName() const
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


void Smb4KShare::setHostName( const QString &hostName )
{
  m_url.setHost( hostName.trimmed() );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setUNC( const QString &unc )
{
  // Check that a valid UNC was passed to this function.
  if ( !unc.startsWith( "//" ) && !unc.startsWith( "smb:" ) && unc.count( "/" ) != 3 )
  {
    // The UNC is malformatted.
    return;
  }
  else
  {
    // Do nothing
  }

  m_url.setUrl( unc.trimmed() );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }

  // Determine whether this is a homes share.
  m_homes_share = (QString::compare( m_url.path().remove( 0, 1 ), "homes", Qt::CaseSensitive ) == 0);
}


QString Smb4KShare::unc( QUrl::FormattingOptions options ) const
{
  return m_url.toString( options ).replace( "//"+m_url.host(), "//"+hostName() );
}


QString Smb4KShare::homeUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( isHomesShare() )
  {
    unc = m_url.toString( options ).replace( "//"+m_url.host(), "//"+hostName() ).replace( m_url.path(), "/"+m_url.userName() );
  }
  else
  {
    // Do nothing
  }
  
  return unc;
}


QString Smb4KShare::hostUNC( QUrl::FormattingOptions options ) const
{
  return m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
}


void Smb4KShare::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KShare::setTypeString( const QString &typeString )
{
  m_type_string = typeString;
}


const QString Smb4KShare::translatedTypeString() const
{
  if ( QString::compare( m_type_string, "Disk" ) == 0 )
  {
    return i18n( "Disk" );
  }
  else if ( QString::compare( m_type_string, "Print" ) == 0 ||
            QString::compare( m_type_string, "Printer" ) == 0 )
  {
    return i18n( "Printer" );
  }
  else
  {
    // Do nothing
  }

  return m_type_string;
}


void Smb4KShare::setComment( const QString &comment )
{
  m_comment = comment;
}


void Smb4KShare::setHostIP( const QString &ip )
{
  m_host_ip = ipIsValid( ip );
}


bool Smb4KShare::isHidden() const
{
  return m_url.path().endsWith( "$" );
}


bool Smb4KShare::isPrinter() const
{
  return (QString::compare( m_type_string, "Print" ) == 0 ||
          QString::compare( m_type_string, "Printer" ) == 0);
}


bool Smb4KShare::isIPC() const
{
  return (QString::compare( m_url.path(), "IPC$" ) == 0);
}


bool Smb4KShare::isADMIN() const
{
  return (QString::compare( m_url.path(), "ADMIN$" ) == 0);
}


void Smb4KShare::setPath( const QString &mountpoint )
{
  m_path = mountpoint.toUtf8();
}


const QByteArray Smb4KShare::canonicalPath() const
{
  return (m_inaccessible ? m_path : QDir( m_path ).canonicalPath().toUtf8());
}


void Smb4KShare::setInaccessible( bool in )
{
  m_inaccessible = in;
}


void Smb4KShare::setForeign( bool foreign )
{
  m_foreign = foreign;
}


void Smb4KShare::setFileSystem( FileSystem filesystem )
{
  m_filesystem = filesystem;
}


const QString Smb4KShare::fileSystemString() const
{
  switch ( m_filesystem )
  {
    case CIFS:
    {
      return "cifs";
    }
    case SMBFS:
    {
      return "smbfs";
    }
    default:
    {
      break;
    }
  }

  return QString();
}


void Smb4KShare::setUID( uid_t uid )
{
  m_user = KUser( uid );
}


void Smb4KShare::setGID( gid_t gid )
{
  m_group = KUserGroup( gid );
}


void Smb4KShare::setLogin( const QString &login )
{
  m_url.setUserName( login );
}


void Smb4KShare::setIsMounted( bool mounted )
{
  if ( !isPrinter() )
  {
    m_is_mounted = mounted;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setTotalDiskSpace( qulonglong size )
{
  m_total = size;
}


QString Smb4KShare::totalDiskSpaceString() const
{
  QString total, total_dim = QString();

  int exponent = 0;
  qreal tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( m_total * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_total = m_total / pow( 1024, factor );
  total = QString( "%1" ).arg( tmp_total, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      total_dim = "B";
      break;
    }
    case 1:
    {
      total_dim = "KiB";
      break;
    }
    case 2:
    {
      total_dim = "MiB";
      break;
    }
    case 3:
    {
      total_dim = "GiB";
      break;
    }
    case 4:
    {
      total_dim = "TiB";
      break;
    }
    default:
    {
      break;
    }
  }

  return total+" "+total_dim;
}


void Smb4KShare::setFreeDiskSpace( qulonglong size )
{
  m_free = size;
}


QString Smb4KShare::freeDiskSpaceString() const
{
  QString free, free_dim = QString();

  int exponent = 0;
  qreal tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( m_free * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_free = m_free / pow( 1024, factor );
  free = QString( "%1" ).arg( tmp_free, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      free_dim = "B";
      break;
    }
    case 1:
    {
      free_dim = "KiB";
      break;
    }
    case 2:
    {
      free_dim = "MiB";
      break;
    }
    case 3:
    {
      free_dim = "GiB";
      break;
    }
    case 4:
    {
      free_dim = "TiB";
      break;
    }
    default:
    {
      break;
    }
  }

  return free+" "+free_dim;
}


void Smb4KShare::setUsedDiskSpace( qulonglong size )
{
  m_used = size;
}


QString Smb4KShare::usedDiskSpaceString() const
{
  QString used, used_dim = QString();

  int exponent = 0;
  qreal tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( m_used * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_used = m_used / pow( 1024, factor );
  used = QString( "%1" ).arg( tmp_used, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      used_dim = "B";
      break;
    }
    case 1:
    {
      used_dim = "KiB";
      break;
    }
    case 2:
    {
      used_dim = "MiB";
      break;
    }
    case 3:
    {
      used_dim = "GiB";
      break;
    }
    case 4:
    {
      used_dim = "TiB";
      break;
    }
    default:
    {
      break;
    }
  }

  return used+" "+used_dim;
}


qreal Smb4KShare::diskUsage() const
{
  qreal used( usedDiskSpace() );
  qreal total( totalDiskSpace() );

  if ( total > 0 )
  {
    return used * 100 / total;
  }
  
  return 0;
}


QString Smb4KShare::diskUsageString() const
{
  return QString( "%1 %" ).arg( diskUsage(), 0, 'f', 1 );
}


bool Smb4KShare::equals( Smb4KShare *share, CheckFlags flag )
{
  Q_ASSERT( share );

  switch ( flag )
  {
    case Full:
    {
      if ( QString::compare( unc( QUrl::RemovePassword ), share->unc( QUrl::RemovePassword ) ) == 0 &&
           QString::compare( m_workgroup, share->workgroupName() ) == 0 &&
           QString::compare( m_type_string, share->typeString() ) == 0 &&
           QString::compare( m_comment, share->comment() ) == 0 &&
           QString::compare( m_host_ip, share->hostIP() ) == 0 &&
           QString::compare( m_path, share->path() ) == 0 &&
           m_inaccessible == share->isInaccessible() &&
           m_foreign == share->isForeign() &&
           m_filesystem == share->fileSystem() &&
           m_user.uid() == share->uid() &&
           m_group.gid() == share->gid() &&
           m_total == share->totalDiskSpace() &&
           m_free == share->freeDiskSpace() &&
           m_homes_users == share->homesUsers() )
      {
        return true;
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case NetworkOnly:
    {
      if ( QString::compare( unc( QUrl::RemovePassword ), share->unc( QUrl::RemovePassword ) ) == 0 &&
           QString::compare( m_workgroup, share->workgroupName() ) == 0 &&
           QString::compare( m_type_string, share->typeString() ) == 0 &&
           QString::compare( m_comment, share->comment() ) == 0 &&
           QString::compare( m_host_ip, share->hostIP() ) == 0 &&
           m_homes_users == share->homesUsers() )
      {
        return true;
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case LocalOnly:
    {
      if ( QString::compare( m_path, share->path() ) == 0 &&
           QString::compare( m_url.userName(), share->login() ) == 0 &&
           m_inaccessible == share->isInaccessible() &&
           m_foreign == share->isForeign() &&
           m_filesystem == share->fileSystem() &&
           m_user.uid() == share->uid() &&
           m_group.gid() == share->gid() &&
           m_total == share->totalDiskSpace() &&
           m_free == share->freeDiskSpace() )
      {
        return true;
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

  return false;
}


bool Smb4KShare::isEmpty( CheckFlags flag ) const
{
  switch ( flag )
  {
    case Full:
    {
      if ( !m_url.isEmpty() )
      {
        return false;
      }

      if ( !m_workgroup.isEmpty() )
      {
        return false;
      }

      if ( !m_type_string.isEmpty() )
      {
        return false;
      }

      if ( !m_comment.isEmpty() )
      {
        return false;
      }

      if ( !m_host_ip.isEmpty() )
      {
        return false;
      }

      if ( !m_path.isEmpty() )
      {
        return false;
      }

      if ( m_filesystem != Unknown )
      {
        return false;
      }

      if ( m_total > 0 )
      {
        return false;
      }

      if ( m_free > 0 )
      {
        return false;
      }
      
      if ( m_used > 0 )
      {
        return false;
      }

      break;
    }
    case NetworkOnly:
    {
      if ( !m_url.isEmpty() )
      {
        return false;
      }

      if ( !m_workgroup.isEmpty() )
      {
        return false;
      }

      if ( !m_type_string.isEmpty() )
      {
        return false;
      }

      if ( !m_comment.isEmpty() )
      {
        return false;
      }

      if ( !m_host_ip.isEmpty() )
      {
        return false;
      }

      break;
    }
    case LocalOnly:
    {
      if ( !m_path.isEmpty() )
      {
        return false;
      }

      if ( m_filesystem != Unknown )
      {
        return false;
      }

      if ( m_total > 0 )
      {
        return false;
      }

      if ( m_free > 0 )
      {
        return false;
      }
      
      if ( m_used > 0 )
      {
        return false;
      }

      break;
    }
    default:
    {
      break;
    }
  }

  return true;
}


void Smb4KShare::setMountData( Smb4KShare *share )
{
  if ( share )
  {
    m_path = share->path();
    m_inaccessible = share->isInaccessible();
    m_foreign = share->isForeign();
    m_filesystem = share->fileSystem();
    m_user = KUser( share->uid() );
    m_group = KUserGroup( share->gid() );
    m_total = share->totalDiskSpace();
    m_free = share->freeDiskSpace();
    m_is_mounted = share->isMounted();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::resetMountData()
{
  // The login name is not reset here, because it is also
  // needed for things that are not mount-releated.
  m_path = QByteArray();
  m_inaccessible = false;
  m_foreign = false;
  m_filesystem = Unknown;
  m_user = KUser( getuid() );
  m_group = KUserGroup( getgid() );
  m_total = -1;
  m_free = -1;
  m_is_mounted = false;
}


void Smb4KShare::setHomesUsers( const QStringList &users )
{
  if ( m_homes_share )
  {
    m_homes_users = users;
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setPort( int port )
{
  m_url.setPort( port );
}


void Smb4KShare::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  m_url.setUserName( authInfo->login() );
  m_url.setPassword( authInfo->password() );
}


const QString &Smb4KShare::ipIsValid( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    static_cast<QString>( ip ).clear();
  }

  return ip;
}
