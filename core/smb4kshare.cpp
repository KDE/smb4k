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

// application specific includes
#include "smb4kshare.h"
#include "smb4kauthinfo.h"

// Qt include
#include <QHostAddress>
#include <QDir>

// KDE includes
#include <klocale.h>
#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

// system includes
#include <unistd.h>
#include <math.h>

class Smb4KSharePrivate
{
  public:
    QUrl url;
    QString workgroup;
    QString typeString;
    QString comment;
    QHostAddress ip;
    QString path;
    bool inaccessible;
    bool foreign;
    int filesystem;
    KUser user;
    KUserGroup group;
    qulonglong totalSpace;
    qulonglong freeSpace;
    qulonglong usedSpace;
    bool mounted;
};


Smb4KShare::Smb4KShare( const QString &host, const QString &name )
: Smb4KBasicNetworkItem( Share ), d( new Smb4KSharePrivate )
{
  d->typeString   = "Disk";
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = Unknown;
  d->user         = KUser( getuid() );
  d->group        = KUserGroup( getgid() );
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->url.setHost( host );
  d->url.setPath( name );
  d->url.setScheme( "smb" );
  setShareIcon();
}


Smb4KShare::Smb4KShare( const QString &unc )
: Smb4KBasicNetworkItem( Share ), d( new Smb4KSharePrivate )
{
  d->typeString   = "Disk";
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = Unknown;
  d->user         = KUser( getuid() );
  d->group        = KUserGroup( getgid() );
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->url.setUrl( unc );
  d->url.setScheme( "smb" );
  setShareIcon();
}


Smb4KShare::Smb4KShare( const Smb4KShare &s )
: Smb4KBasicNetworkItem( Share ), d( new Smb4KSharePrivate )
{
  *d = *s.d;

  if ( icon().isNull() )
  {
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
}



Smb4KShare::Smb4KShare()
: Smb4KBasicNetworkItem( Share ), d( new Smb4KSharePrivate )
{
  d->typeString   = "Disk";
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = Unknown;
  d->user         = KUser( getuid() );
  d->group        = KUserGroup( getgid() );
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
}


Smb4KShare::~Smb4KShare()
{
}


void Smb4KShare::setShareName( const QString &name )
{
  d->url.setPath( name.trimmed() );

  if ( d->url.scheme().isEmpty() )
  {
    d->url.setScheme( "smb" );
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
  QString share_name = d->url.path();
  
  if ( share_name.startsWith( '/' ) )
  {
    share_name = share_name.remove( 0, 1 );
  }
  else
  {
    // Do nothing
  }
  
  if ( share_name.endsWith( '/' ) )
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
  d->url.setHost( hostName.trimmed() );

  if ( d->url.scheme().isEmpty() )
  {
    d->url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::hostName() const
{
  return d->url.host().toUpper();
}


QString Smb4KShare::unc( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || d->url.userName().isEmpty() )
  {
    unc = d->url.toString( options|QUrl::StripTrailingSlash ).replace( "//"+d->url.host(), "//"+hostName() );
  }
  else
  {
    unc = d->url.toString( options|QUrl::StripTrailingSlash ).replace( '@'+d->url.host(), '@'+hostName() );
  }
  
  return unc;
}


QString Smb4KShare::homeUNC( QUrl::FormattingOptions options ) const
{
  QString unc;

  if ( isHomesShare() && !d->url.userName().isEmpty() )
  {
    if ( (options & QUrl::RemoveUserInfo) )
    {
      unc = d->url.toString( options|QUrl::StripTrailingSlash );

      if ( d->url.path().startsWith( '/' ) )
      {
        unc = unc.replace( "//"+d->url.host(), "//"+hostName() ).replace( d->url.path(), '/'+d->url.userName() );
      }
      else
      {
        unc = unc.replace( "//"+d->url.host(), "//"+hostName() ).replace( d->url.path(), '/'+d->url.userName() );
      }
    }
    else
    {
      unc = d->url.toString( options|QUrl::StripTrailingSlash );

      if ( d->url.path().startsWith( '/' ) )
      {
        unc = unc.replace( '@'+d->url.host(), '@'+hostName() ).replace( d->url.path(), '/'+d->url.userName() );
      }
      else
      {
        unc = unc.replace( '@'+d->url.host(), '@'+hostName() ).replace( d->url.path(), '/'+d->url.userName() );
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
    return;
  }
  else
  {
    // Do nothing
  }

  // Check scheme
  if ( !url.scheme().isEmpty() && QString::compare( "smb", url.scheme() ) != 0 )
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Check that the share name is present
  if ( (url.path().endsWith( '/' ) && url.path().count( "/" ) > 2) ||
       (!url.path().endsWith( '/' ) && url.path().count( "/" ) > 1) )
  {
    return;
  }
  else
  {
    // Do nothing
  }

  // Set the URL
  d->url = url;

  // Do some adjustments
  if ( d->url.scheme().isEmpty() )
  {
    d->url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QUrl Smb4KShare::url() const
{
  return d->url;
}


QUrl Smb4KShare::homeURL() const
{
  QUrl url;
  
  if ( isHomesShare() && !d->url.userName().isEmpty() )
  {
    url = d->url;
    url.setPath( d->url.userName() );
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

  if ( (options & QUrl::RemoveUserInfo) || d->url.userName().isEmpty() )
  {
    unc = d->url.toString( options|QUrl::RemovePath|QUrl::StripTrailingSlash ).replace( "//"+d->url.host(), "//"+hostName() );
  }
  else
  {
    unc = d->url.toString( options|QUrl::RemovePath|QUrl::StripTrailingSlash ).replace( '@'+d->url.host(), '@'+hostName() );
  }
  
  return unc;
}


void Smb4KShare::setWorkgroupName( const QString &workgroup )
{
  d->workgroup = workgroup;
}


QString Smb4KShare::workgroupName() const
{
  return d->workgroup;
}


void Smb4KShare::setTypeString( const QString &typeString )
{
  d->typeString = typeString;
  setShareIcon();
}


QString Smb4KShare::typeString() const
{
  return d->typeString;
}


QString Smb4KShare::translatedTypeString() const
{
  if ( QString::compare( d->typeString, "Disk" ) == 0 )
  {
    return i18n( "Disk" );
  }
  else if ( QString::compare( d->typeString, "Print" ) == 0 ||
            QString::compare( d->typeString, "Printer" ) == 0 )
  {
    return i18n( "Printer" );
  }
  else
  {
    // Do nothing
  }

  return d->typeString;
}


void Smb4KShare::setComment( const QString &comment )
{
  d->comment = comment;
}


QString Smb4KShare::comment() const
{
  return d->comment;
}


void Smb4KShare::setHostIP( const QString &ip )
{
  d->ip.setAddress( ip );
}


QString Smb4KShare::hostIP() const
{
  return d->ip.toString();
}


bool Smb4KShare::isHidden() const
{
  return d->url.path().endsWith( '$' );
}


bool Smb4KShare::isPrinter() const
{
  if ( d->inaccessible || d->typeString.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return (QString::compare( d->typeString, "Print" ) == 0 ||
          QString::compare( d->typeString, "Printer" ) == 0);
}


bool Smb4KShare::isIPC() const
{
  return (QString::compare( d->url.path(), "IPC$" ) == 0);
}


bool Smb4KShare::isADMIN() const
{
  return (QString::compare( d->url.path(), "ADMIN$" ) == 0);
}


void Smb4KShare::setPath( const QString &mountpoint )
{
  d->path = mountpoint;
}


QString Smb4KShare::path() const
{
  return d->path;
}


QString Smb4KShare::canonicalPath() const
{
  return (d->inaccessible ? d->path : QDir( d->path ).canonicalPath());
}


void Smb4KShare::setInaccessible( bool in )
{
  d->inaccessible = in;
  setShareIcon();
}


bool Smb4KShare::isInaccessible() const
{
  return d->inaccessible;
}


void Smb4KShare::setForeign( bool foreign )
{
  d->foreign = foreign;
  setShareIcon();
}


bool Smb4KShare::isForeign() const
{
  return d->foreign;
}


void Smb4KShare::setFileSystem( FileSystem filesystem )
{
  d->filesystem = filesystem;
}


Smb4KShare::FileSystem Smb4KShare::fileSystem() const
{
  return static_cast<FileSystem>( d->filesystem );
}


QString Smb4KShare::fileSystemString() const
{
  switch ( d->filesystem )
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


void Smb4KShare::setUID( K_UID uid )
{
  d->user = KUser( uid );
}


uid_t Smb4KShare::uid() const
{
  return d->user.uid();
}


QString Smb4KShare::owner() const
{
  return d->user.loginName();
}


void Smb4KShare::setGID( K_GID gid )
{
  d->group = KUserGroup( gid );
}


K_GID Smb4KShare::gid() const
{
  return d->group.gid();
}


QString Smb4KShare::group() const
{
  return d->group.name();
}


void Smb4KShare::setIsMounted( bool mounted )
{
  if ( !isPrinter() )
  {
    d->mounted = mounted;
    setShareIcon();
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KShare::isMounted() const
{
  return d->mounted;
}


void Smb4KShare::setTotalDiskSpace( qulonglong size )
{
  d->totalSpace = size;
}


qulonglong Smb4KShare::totalDiskSpace() const
{
  return d->totalSpace;
}


QString Smb4KShare::totalDiskSpaceString() const
{
  QString total, total_dim;

  int exponent = 0;
  double tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( d->totalSpace * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_total = d->totalSpace / pow( 1024, factor );
  total = QString( "%1" ).arg( tmp_total, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      total_dim = 'B';
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

  return total+' '+total_dim;
}


void Smb4KShare::setFreeDiskSpace( qulonglong size )
{
  d->freeSpace = size;
}


qulonglong Smb4KShare::freeDiskSpace() const
{
  return d->freeSpace;
}


QString Smb4KShare::freeDiskSpaceString() const
{
  QString free, free_dim;

  int exponent = 0;
  double tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( d->freeSpace * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_free = d->freeSpace / pow( 1024, factor );
  free = QString( "%1" ).arg( tmp_free, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      free_dim = 'B';
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

  return free+' '+free_dim;
}


void Smb4KShare::setUsedDiskSpace( qulonglong size )
{
  d->usedSpace = size;
}


qulonglong Smb4KShare::usedDiskSpace() const
{
  return d->usedSpace;
}


QString Smb4KShare::usedDiskSpaceString() const
{
  QString used, used_dim;

  int exponent = 0;
  double tmp_factor = 0;
  qulonglong factor = 0;

  (void) frexp( d->usedSpace * 1024, &exponent );
  (void) modf( (exponent - 10) / 10, &tmp_factor );
  factor = tmp_factor;
  qreal tmp_used = d->usedSpace / pow( 1024, factor );
  used = QString( "%1" ).arg( tmp_used, 0, 'f', 1 );

  switch ( factor )
  {
    case 0:
    {
      used_dim = 'B';
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

  return used+' '+used_dim;
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
           QString::compare( workgroupName(), share->workgroupName() ) == 0 &&
           QString::compare( typeString(), share->typeString() ) == 0 &&
           QString::compare( comment(), share->comment() ) == 0 &&
           QString::compare( hostIP(), share->hostIP() ) == 0 &&
           QString::compare( path(), share->path() ) == 0 &&
           isInaccessible() == share->isInaccessible() &&
           isForeign() == share->isForeign() &&
           fileSystem() == share->fileSystem() &&
           uid() == share->uid() &&
           gid() == share->gid() &&
           totalDiskSpace() == share->totalDiskSpace() &&
           freeDiskSpace() == share->freeDiskSpace() )
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
           QString::compare( workgroupName(), share->workgroupName() ) == 0 &&
           QString::compare( typeString(), share->typeString() ) == 0 &&
           QString::compare( comment(), share->comment() ) == 0 &&
           QString::compare( hostIP(), share->hostIP() ) == 0 )
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
      if ( QString::compare( path(), share->path() ) == 0 &&
           QString::compare( login(), share->login() ) == 0 &&
           isInaccessible() == share->isInaccessible() &&
           isForeign() == share->isForeign() &&
           fileSystem() == share->fileSystem() &&
           uid() == share->uid() &&
           gid() == share->gid() &&
           totalDiskSpace() == share->totalDiskSpace() &&
           freeDiskSpace() == share->freeDiskSpace() )
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
      if ( !d->url.isEmpty() )
      {
        return false;
      }

      if ( !d->workgroup.isEmpty() )
      {
        return false;
      }

      if ( !d->typeString.isEmpty() )
      {
        return false;
      }

      if ( !d->comment.isEmpty() )
      {
        return false;
      }

      if ( !d->ip.isNull() )
      {
        return false;
      }

      if ( !d->path.isEmpty() )
      {
        return false;
      }

      if ( d->filesystem != Unknown )
      {
        return false;
      }

      if ( d->totalSpace > 0 )
      {
        return false;
      }

      if ( d->freeSpace > 0 )
      {
        return false;
      }
      
      if ( d->usedSpace > 0 )
      {
        return false;
      }

      break;
    }
    case NetworkOnly:
    {
      if ( !d->url.isEmpty() )
      {
        return false;
      }

      if ( !d->workgroup.isEmpty() )
      {
        return false;
      }

      if ( !d->typeString.isEmpty() )
      {
        return false;
      }

      if ( !d->comment.isEmpty() )
      {
        return false;
      }

      if ( !d->ip.isNull() )
      {
        return false;
      }

      break;
    }
    case LocalOnly:
    {
      if ( !d->path.isEmpty() )
      {
        return false;
      }

      if ( d->filesystem != Unknown )
      {
        return false;
      }

      if ( d->totalSpace > 0 )
      {
        return false;
      }

      if ( d->freeSpace > 0 )
      {
        return false;
      }
      
      if ( d->usedSpace > 0 )
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
  Q_ASSERT( share );

  if ( equals( share, NetworkOnly ) )
  {
    d->path         = share->path();
    d->inaccessible = share->isInaccessible();
    d->foreign      = share->isForeign();
    d->filesystem   = share->fileSystem();
    d->user         = KUser( share->uid() );
    d->group        = KUserGroup( share->gid() );
    d->totalSpace   = share->totalDiskSpace();
    d->freeSpace    = share->freeDiskSpace();
    d->usedSpace    = share->usedDiskSpace();
    d->mounted      = share->isMounted();
    d->typeString   = share->typeString();
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
  d->path.clear();
  d->inaccessible = false;
  d->foreign      = false;
  d->filesystem   = Unknown;
  d->user         = KUser( getuid() );
  d->group        = KUserGroup( getgid() );
  d->totalSpace   = -1;
  d->freeSpace    = -1;
  d->usedSpace    = -1;
  d->mounted      = false;
  d->typeString   = "Disk";
  setShareIcon();
}


bool Smb4KShare::isHomesShare() const
{
  return d->url.path().endsWith( "homes" );
}


void Smb4KShare::setPort( int port )
{
  d->url.setPort( port );
}


int Smb4KShare::port() const
{
  return d->url.port();
}


void Smb4KShare::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  // Avoid that the login is overwritten with an empty 
  // string if we have a homes share.
  if ( !isHomesShare() || !authInfo->login().isEmpty() )
  {
    d->url.setUserName( authInfo->login() );
    d->url.setPassword( authInfo->password() );
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
    d->url.setUserName( login );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::login() const
{
  return d->url.userName();
}


void Smb4KShare::setPassword( const QString &passwd )
{
  // Avoid that the password is overwritten with an empty 
  // string if we have a homes share.
  if ( !isHomesShare() || !passwd.isEmpty() )
  {
    d->url.setPassword( passwd );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KShare::password() const
{
  return d->url.password();
}


bool Smb4KShare::hasHostIP() const
{
  return !d->ip.isNull();
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

