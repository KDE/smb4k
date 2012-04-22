/***************************************************************************
    smb4kshare  -  Smb4K's container class for information about a share.
                             -------------------
    begin                : Mo Jan 28 2008
    copyright            : (C) 2008-2012 by Alexander Reinholdt
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

// Qt include
#include <QHostAddress>
#include <QAbstractSocket>
#include <QDir>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

// system includes
#include <unistd.h>
#include <math.h>

// application specific includes
#include <smb4kshare.h>
#include <smb4kauthinfo.h>

Smb4KShare::Smb4KShare( const QString &host, const QString &name ) : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( "Disk" ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QString() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false )
{
  m_url.setHost( host );
  m_url.setPath( name );
  m_url.setScheme( "smb" );
  setShareIcon();
}


Smb4KShare::Smb4KShare( const QString &unc ) : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( "Disk" ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QString() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false )
{
  setURL( QUrl( unc ) );
  setShareIcon();
}


Smb4KShare::Smb4KShare( const Smb4KShare &s ) : Smb4KBasicNetworkItem( Share ),
  m_url( s.url() ), m_workgroup( s.workgroupName() ), m_type_string( s.typeString() ), m_comment( s.comment() ),
  m_host_ip( s.hostIP() ), m_path( s.path() ), m_inaccessible( s.isInaccessible() ),
  m_foreign( s.isForeign() ), m_filesystem( s.fileSystem() ), m_user( s.uid() ), m_group( s.gid() ),
  m_total( s.totalDiskSpace() ), m_free( s.freeDiskSpace() ), m_used( s.usedDiskSpace() ),
  m_is_mounted( s.isMounted() )
{
  if ( icon().isNull() )
  {
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
}



Smb4KShare::Smb4KShare() : Smb4KBasicNetworkItem( Share ),
  m_url( QUrl() ), m_workgroup( QString() ), m_type_string( "Disk" ), m_comment( QString() ),
  m_host_ip( QString() ), m_path( QString() ), m_inaccessible( false ), m_foreign( false ),
  m_filesystem( Unknown ), m_user( getuid() ), m_group( getgid() ), m_total( 0 ), m_free( 0 ), m_used( 0 ),
  m_is_mounted( false )
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
}


QString Smb4KShare::shareName() const
{
  // Since users might come up with very weird share names,
  // we are careful and do not use QString::remove( "/" ), but
  // only remove preceding and trailing slashes.
  QString share_name = m_url.path();
  
  if ( share_name.startsWith( "/" ) )
  {
    share_name = share_name.remove( 0, 1 );
  }
  else
  {
    // Do nothing
  }
  
  if ( share_name.endsWith( "/" ) )
  {
    share_name = share_name.remove( share_name.size() - 1, 1 );
  }
  else
  {
    // Do nothing
  }
  
  return share_name;
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


QString Smb4KShare::unc( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || m_url.userName().isEmpty() )
  {
    unc = m_url.toString( options|QUrl::StripTrailingSlash ).replace( "//"+m_url.host(), "//"+hostName() );
  }
  else
  {
    unc = m_url.toString( options|QUrl::StripTrailingSlash ).replace( "@"+m_url.host(), "@"+hostName() );
  }
  
  return unc;
}


QString Smb4KShare::homeUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( isHomesShare() && !m_url.userName().isEmpty() )
  {
    if ( (options & QUrl::RemoveUserInfo) )
    {
      unc = m_url.toString( options|QUrl::StripTrailingSlash );
      
      if ( m_url.path().startsWith( "/" ) )
      {
        unc = unc.replace( "//"+m_url.host(), "//"+hostName() ).replace( m_url.path(), "/"+m_url.userName() );
      }
      else
      {
        unc = unc.replace( "//"+m_url.host(), "//"+hostName() ).replace( m_url.path(), m_url.userName() );
      }
    }
    else
    {
      unc = m_url.toString( options|QUrl::StripTrailingSlash );
      
      if ( m_url.path().startsWith( "/" ) )
      {
        unc = unc.replace( "@"+m_url.host(), "@"+hostName() ).replace( m_url.path(), "/"+m_url.userName() );
      }
      else
      {
        unc = unc.replace( "@"+m_url.host(), "@"+hostName() ).replace( m_url.path(), m_url.userName() );
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  return unc;
}


void Smb4KShare::setURL( const QUrl &url )
{
  // Check validity.
  if ( !url.isValid() )
  {
    qDebug() << "Invalid URL provided";
    return;
  }
  else
  {
    // Do nothing
  }

  // Check scheme
  if ( !url.scheme().isEmpty() && QString::compare( "smb", url.scheme() ) != 0 )
  {
    qDebug() << "URL has wrong scheme";
    return;
  }
  else
  {
    // Do nothing
  }

  // Check that the share name is present
  if ( (url.path().endsWith( "/" ) && url.path().count( "/" ) > 2) ||
       (!url.path().endsWith( "/" ) && url.path().count( "/" ) > 1) )
  {
    qDebug() << "Not a URL of a share.";
    return;
  }
  else
  {
    // Do nothing
  }

  // Set the URL
  m_url = url;

  // Do some adjustments
  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QUrl Smb4KShare::homeURL() const
{
  QUrl url;
  
  if ( isHomesShare() && !m_url.userName().isEmpty() )
  {
    url = m_url;
    url.setPath( m_url.userName() );
  }
  else
  {
    // Do nothing
  }
  
  return url;
}


QString Smb4KShare::hostUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || m_url.userName().isEmpty() )
  {
    unc = m_url.toString( options|QUrl::RemovePath|QUrl::StripTrailingSlash ).replace( "//"+m_url.host(), "//"+hostName() );
  }
  else
  {
    unc = m_url.toString( options|QUrl::RemovePath|QUrl::StripTrailingSlash ).replace( "@"+m_url.host(), "@"+hostName() );
  }
  
  return unc;
}


void Smb4KShare::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KShare::setTypeString( const QString &typeString )
{
  m_type_string = typeString;
  
  setShareIcon();
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
  if ( isInaccessible() || m_type_string.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
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
  m_path = mountpoint;
}


const QString Smb4KShare::canonicalPath() const
{
  return (m_inaccessible ? m_path : QDir( m_path ).canonicalPath());
}


void Smb4KShare::setInaccessible( bool in )
{
  m_inaccessible = in;
  setShareIcon();
}


void Smb4KShare::setForeign( bool foreign )
{
  m_foreign = foreign;
  setShareIcon();
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


void Smb4KShare::setIsMounted( bool mounted )
{
  if ( !isPrinter() )
  {
    m_is_mounted = mounted;
   
    setShareIcon();
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
  QString total, total_dim;

  int exponent = 0;
  double tmp_factor = 0;
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
  QString free, free_dim;

  int exponent = 0;
  double tmp_factor = 0;
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
  QString used, used_dim;

  int exponent = 0;
  double tmp_factor = 0;
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


bool Smb4KShare::equals( Smb4KShare *share, CheckFlags flag ) const
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
    case NetworkOnly:
    {
      if ( QString::compare( unc( QUrl::RemovePassword ), share->unc( QUrl::RemovePassword ) ) == 0 &&
           QString::compare( m_workgroup, share->workgroupName() ) == 0 &&
           QString::compare( m_type_string, share->typeString() ) == 0 &&
           QString::compare( m_comment, share->comment() ) == 0 &&
           QString::compare( m_host_ip, share->hostIP() ) == 0 )
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
    m_type_string = share->typeString();
    
    setShareIcon();
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
  m_path.clear();
  m_inaccessible = false;
  m_foreign = false;
  m_filesystem = Unknown;
  m_user = KUser( getuid() );
  m_group = KUserGroup( getgid() );
  m_total = -1;
  m_free = -1;
  m_is_mounted = false;
  m_type_string = "Disk";
  
  setShareIcon();
}


bool Smb4KShare::isHomesShare() const
{
  return m_url.path().endsWith( "homes" );
}


void Smb4KShare::setPort( int port )
{
  m_url.setPort( port );
}


void Smb4KShare::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  // Avoid that the login is overwritten with an empty 
  // string if we have a homes share.
  if ( !isHomesShare() || !authInfo->login().isEmpty() )
  {
    m_url.setUserName( authInfo->login() );
    m_url.setPassword( authInfo->password() );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setLogin( const QString &login )
{
  // Avoid that the login is overwritten with an empty 
  // string if we have a homes share.
  if ( !isHomesShare() || !login.isEmpty() )
  {
    m_url.setUserName( login );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setPassword( const QString &passwd )
{
  // Avoid that the password is overwritten with an empty 
  // string if we have a homes share.
  if ( !isHomesShare() || !passwd.isEmpty() )
  {
    m_url.setPassword( passwd );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KShare::setShareIcon()
{
  // We have three base icons: The remote folder, the locked folder
  // and the printer icon.
  if ( !isPrinter() )
  {
    // Overlays
    QStringList overlays;
    
    if ( isMounted() )
    {
      overlays << "emblem-mounted";
    }
    else
    {
      overlays << "";
    }

    if ( isForeign() )
    {
      overlays << "";
      overlays << "flag-red";
    }
    else
    {
      // Do nothing
    }
    
    // Icon name
    QString icon_name;
    
    if ( !isInaccessible() )
    {
      icon_name = "folder-remote";
    }
    else
    {
      icon_name = "folder-locked";
    }
    
    setIcon( KIcon( icon_name, KIconLoader::global(), overlays ) );
  }
  else
  {
    setIcon( KIcon( "printer" ) );
  }
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
