/***************************************************************************
    smb4kauthinfo.cpp  -  This class provides a container for the
    authentication data.
                             -------------------
    begin                : Sa Feb 28 2004
    copyright            : (C) 2004-2012 by Alexander Reinholdt
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
#include "smb4kauthinfo.h"

// Qt includes
#include <QHostAddress>

// KDE includes
#include <kdebug.h>


class Smb4KAuthInfoPrivate
{
  public:
    QUrl url;
    QString workgroup;
    int type;
    bool homesShare;
    QHostAddress ip;
};


Smb4KAuthInfo::Smb4KAuthInfo( const Smb4KHost *host )
: d( new Smb4KAuthInfoPrivate )
{
  d->url        = host->url();
  d->type       = Host;
  d->workgroup  = host->workgroupName();
  d->homesShare = false;
  d->ip.setAddress( host->ip() );
}


Smb4KAuthInfo::Smb4KAuthInfo( const Smb4KShare *share )
: d( new Smb4KAuthInfoPrivate )
{
  if ( !share->isHomesShare() )
  {
    d->url      = share->url();
  }
  else
  {
    d->url      = share->homeURL();
  }

  d->type       = Share;
  d->workgroup  = share->workgroupName();
  d->homesShare = share->isHomesShare();
  d->ip.setAddress( share->hostIP() );
}


Smb4KAuthInfo::Smb4KAuthInfo()
: d( new Smb4KAuthInfoPrivate )
{
  d->type       = Unknown;
  d->homesShare = false;
  d->url.clear();
  d->workgroup.clear();
  d->ip.clear();
}


Smb4KAuthInfo::Smb4KAuthInfo( const Smb4KAuthInfo &i )
: d( new Smb4KAuthInfoPrivate )
{
  *d = *i.d;
}


Smb4KAuthInfo::~Smb4KAuthInfo()
{
}


void Smb4KAuthInfo::setHost( Smb4KHost *host )
{
  Q_ASSERT( host );

  d->url        = host->url();
  d->type       = Host;
  d->workgroup  = host->workgroupName();
  d->homesShare = false;
  d->ip.setAddress( host->ip() );
}


void Smb4KAuthInfo::setShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  if ( !share->isHomesShare() )
  {
    d->url      = share->url();
  }
  else
  {
    d->url      = share->homeURL();
  }
  
  d->type       = Share;
  d->workgroup  = share->workgroupName();
  d->homesShare = share->isHomesShare();
  d->ip.setAddress( share->hostIP() );
}


void Smb4KAuthInfo::setWorkgroupName( const QString &workgroup )
{
  d->workgroup = workgroup;
}


QString Smb4KAuthInfo::workgroupName() const
{
  return d->workgroup;
}


QString Smb4KAuthInfo::unc( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  switch ( d->type )
  {
    case Host:
    {
      if ( (options & QUrl::RemoveUserInfo) || d->url.userName().isEmpty() )
      {
        unc = d->url.toString( options|QUrl::RemovePath ).replace( "//"+d->url.host(), "//"+hostName() );
      }
      else
      {
        unc = d->url.toString( options|QUrl::RemovePath ).replace( '@'+d->url.host(), '@'+hostName() );
      }
      break;
    }
    case Share:
    {
      if ( (options & QUrl::RemoveUserInfo) || d->url.userName().isEmpty() )
      {
        unc = d->url.toString( options ).replace( "//"+d->url.host(), "//"+hostName() );
      }
      else
      {
        unc = d->url.toString( options ).replace( '@'+d->url.host(), '@'+hostName() );
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


QString Smb4KAuthInfo::hostUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || d->url.userName().isEmpty() )
  {
    unc = d->url.toString( options|QUrl::RemovePath ).replace( "//"+d->url.host(), "//"+hostName() );
  }
  else
  {
    unc = d->url.toString( options|QUrl::RemovePath ).replace( '@'+d->url.host(), '@'+hostName() );
  }
  
  return unc;
}


void Smb4KAuthInfo::setURL( const QUrl &url )
{
  d->url = url;

  // Set the type.
  if ( d->url.path().contains( "/" ) == 1 )
  {
    d->type = Share;
  }
  else
  {
    d->type = Host;
  }
  
  // Set the scheme.
  if ( d->url.scheme().isEmpty() )
  {
    d->url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }

  // Determine whether this is a homes share.
  d->homesShare = (QString::compare( d->url.path().remove( 0, 1 ), "homes", Qt::CaseSensitive ) == 0);
}


QUrl Smb4KAuthInfo::url() const
{
  return d->url;
}


QString Smb4KAuthInfo::hostName() const
{
  return d->url.host().toUpper();
}



QString Smb4KAuthInfo::shareName() const
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


void Smb4KAuthInfo::setLogin( const QString &login )
{
  d->url.setUserName( login );

  if ( d->homesShare )
  {
    d->url.setPath( login );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KAuthInfo::login() const
{
  return d->url.userName();
}


void Smb4KAuthInfo::setPassword( const QString &passwd )
{
  d->url.setPassword( passwd );
}


QString Smb4KAuthInfo::password() const
{
  return d->url.password();
}


int Smb4KAuthInfo::type() const
{
  return d->type;
}


bool Smb4KAuthInfo::isHomesShare() const
{
  return d->homesShare;
}


void Smb4KAuthInfo::useDefaultAuthInfo()
{
  d->type = Default;
}


bool Smb4KAuthInfo::equals( Smb4KAuthInfo *info ) const
{
  // URL
  QUrl url( info->unc( QUrl::None ) );
  
  if ( d->url != url )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Type
  if ( d->type != info->type() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if ( QString::compare( d->workgroup, info->workgroupName(), Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Homes share?
  if ( d->homesShare != info->isHomesShare() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // IP address
  if ( QString::compare( d->ip.toString(), info->ip() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  return true;
}


void Smb4KAuthInfo::setIP( const QString &ip )
{
  d->ip.setAddress( ip );
}


QString Smb4KAuthInfo::ip() const
{
  return d->ip.toString();
}

