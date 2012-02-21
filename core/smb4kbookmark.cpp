/***************************************************************************
    smb4kbookmark  -  This is the bookmark container for Smb4K (next
    generation).
                             -------------------
    begin                : So Jun 8 2008
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QHostAddress>
#include <QAbstractSocket>

// KDE includes
#include <kdebug.h>
#include <kicon.h>

// application specific includes
#include <smb4kbookmark.h>
#include <smb4kshare.h>


Smb4KBookmark::Smb4KBookmark( Smb4KShare *share, const QString &label )
: m_url( QUrl() ), m_workgroup( share->workgroupName() ), m_ip( share->hostIP() ),
  m_type( share->typeString() ), m_label( label ), m_group( QString() ), m_profile( QString() ),
  m_icon( KIcon( "folder-remote" ) )
{
  if ( !share->isHomesShare() )
  {
    m_url = share->url();
  }
  else
  {
    m_url = share->homeURL();
  }
}


Smb4KBookmark::Smb4KBookmark( const Smb4KBookmark &b )
: m_url( b.url() ), m_workgroup( b.workgroupName() ), m_ip( b.hostIP() ), m_type( b.typeString() ),
  m_label( b.label() ), m_group( b.group() ), m_profile( b.profile() ), m_icon( b.icon() )
{
}


Smb4KBookmark::Smb4KBookmark()
: m_url( QUrl() ), m_workgroup( QString() ), m_ip( QString() ), m_type( "Disk" ),
  m_label( QString() ), m_group( QString() ), m_profile( QString() ), 
  m_icon( KIcon( "folder-remote" ) )
{
}


Smb4KBookmark::~Smb4KBookmark()
{
}


void Smb4KBookmark::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KBookmark::setHostName( const QString &host )
{
  m_url.setHost( host );
  
  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KBookmark::setShareName( const QString &share )
{
  m_url.setPath( share );
}


QString Smb4KBookmark::shareName() const
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


void Smb4KBookmark::setHostIP( const QString &ip )
{
  m_ip = ipIsValid( ip );
}


void Smb4KBookmark::setTypeString( const QString &type )
{
  m_type = type;
}


void Smb4KBookmark::setUNC( const QString &unc )
{
  // Set the UNC.
  m_url.setUrl( unc, QUrl::TolerantMode );

  if ( m_url.scheme().isEmpty() )
  {
    m_url.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KBookmark::unc( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || m_url.userName().isEmpty() )
  {
    unc = m_url.toString( options ).replace( "//"+m_url.host(), "//"+hostName() );
  }
  else
  {
    unc = m_url.toString( options ).replace( "@"+m_url.host(), "@"+hostName() );
  }
  
  return unc;
}


QString Smb4KBookmark::hostUNC( QUrl::FormattingOptions options ) const
{
  QString unc;
  
  if ( (options & QUrl::RemoveUserInfo) || m_url.userName().isEmpty() )
  {
    unc = m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
  }
  else
  {
    unc = m_url.toString( options|QUrl::RemovePath ).replace( "@"+m_url.host(), "@"+hostName() );
  }
  
  return unc;
}


void Smb4KBookmark::setLabel( const QString &label )
{
  m_label = label;
}


void Smb4KBookmark::setLogin( const QString &login )
{
  m_url.setUserName( login );
}


void Smb4KBookmark::setGroup( const QString &name )
{
  m_group = name;
}


void Smb4KBookmark::setProfile( const QString &profile )
{
  m_profile = profile;
}


bool Smb4KBookmark::equals( Smb4KBookmark *bookmark ) const
{
  // URL
  QUrl url( bookmark->unc( QUrl::None ) );
  
  if ( m_url != url )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Workgroup
  if ( QString::compare( m_workgroup, bookmark->workgroupName(), Qt::CaseInsensitive ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // IP address
  if ( QString::compare( m_ip, bookmark->hostIP() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Type string
  if ( QString::compare( m_type, bookmark->typeString() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Label
  if ( QString::compare( m_label, bookmark->label() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Group
  if ( QString::compare( m_group, bookmark->group() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Profile
  if ( QString::compare( m_profile, bookmark->profile() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  // The icon is not used here.

  return true;
}


void Smb4KBookmark::setIcon( const QIcon &icon )
{
  m_icon = icon;
}


const QString &Smb4KBookmark::ipIsValid( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    static_cast<QString>( ip ).clear();
  }

  return ip;
}
