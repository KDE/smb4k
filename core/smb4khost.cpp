/***************************************************************************
    smb4khost  -  Smb4K's container class for information about a host.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2011 by Alexander Reinholdt
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
#include <QHostAddress>
#include <QAbstractSocket>
#include <QStringList>

// KDE includes
#include <kdebug.h>
#include <kicon.h>

// application specific includes
#include <smb4khost.h>
#include <smb4kauthinfo.h>


Smb4KHost::Smb4KHost( const QString &name ) : Smb4KBasicNetworkItem( Host ),
  m_url( QUrl( ) ), m_workgroup( QString() ), m_ip( QString() ), m_comment( QString() ),
  m_server_string( QString() ), m_os_string( QString() ), m_info_checked( false ),
  m_ip_checked( false ), m_is_master( false )
{
  setHostName( name );
  setIcon( KIcon( "network-server" ) );
}


Smb4KHost::Smb4KHost( const Smb4KHost &h ) : Smb4KBasicNetworkItem( Host ),
  m_url( h.url() ), m_workgroup( h.workgroupName() ), m_ip( h.ip() ), m_comment( h.comment() ),
  m_server_string( h.serverString() ), m_os_string( h.osString() ), m_info_checked( h.infoChecked() ),
  m_ip_checked( h.ipChecked() ), m_is_master( h.isMasterBrowser() )
{
  if ( icon().isNull() )
  {
    setIcon( KIcon( "network-server" ) );
  }
  else
  {
    // Do nothing
  }
}


Smb4KHost::Smb4KHost() : Smb4KBasicNetworkItem( Host ),
  m_url( QUrl() ), m_workgroup( QString() ), m_ip( QString() ), m_comment( QString() ),
  m_server_string( QString() ), m_os_string( QString() ), m_info_checked( false ),
  m_ip_checked( false ), m_is_master( false )
{
}


Smb4KHost::~Smb4KHost()
{
}


void Smb4KHost::setHostName( const QString &name )
{
  m_url.setHost( name );
  
  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KHost::unc( QUrl::FormattingOptions options ) const
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


void Smb4KHost::setURL( const QUrl &url )
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

  // Check that this is a host item
  if ( !url.path().isEmpty() )
  {
    qDebug() << "Not a host URL. No URL set.";
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


void Smb4KHost::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KHost::setIP( const QString &ip )
{
  m_ip         = ipIsValid( ip );
  m_ip_checked = true;
}


void Smb4KHost::setComment( const QString &comment )
{
  m_comment = comment;
}


void Smb4KHost::setInfo( const QString &serverString, const QString &osString )
{
  m_server_string = serverString;
  m_os_string     = osString;
  m_info_checked  = true;
}


void Smb4KHost::resetInfo()
{
  m_info_checked = false;
  m_server_string.clear();
  m_os_string.clear();
}


void Smb4KHost::setIsMasterBrowser( bool master )
{
  m_is_master = master;
}


bool Smb4KHost::isEmpty() const
{
  if ( !m_url.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( !m_workgroup.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( !m_ip.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( !m_comment.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( !m_server_string.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( !m_os_string.isEmpty() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Do not include icon here.

  return true;
}


void Smb4KHost::setLogin( const QString &login )
{
  m_url.setUserName( login );
}


void Smb4KHost::setPort( int port )
{
  m_url.setPort( port );
}


bool Smb4KHost::equals( Smb4KHost *host ) const
{
  Q_ASSERT( host );

  if ( QString::compare( m_url.toString( QUrl::RemovePassword ), host->unc( QUrl::RemovePassword ) ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_workgroup, host->workgroupName() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_ip, host->ip() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_comment, host->comment() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_server_string, host->serverString() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_os_string, host->osString() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Do not include icon here.

  return true;
}


void Smb4KHost::setAuthInfo( Smb4KAuthInfo *authInfo )
{
  m_url.setUserName( authInfo->login() );
  m_url.setPassword( authInfo->password() );
}


const QString &Smb4KHost::ipIsValid( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    static_cast<QString>( ip ).clear();
  }

  return ip;
}
