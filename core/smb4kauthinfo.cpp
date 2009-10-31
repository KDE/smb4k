/***************************************************************************
    smb4kauthinfo.cpp  -  This class provides a container for the
    authentication data.
                             -------------------
    begin                : Sa Feb 28 2004
    copyright            : (C) 2004-2009 by Alexander Reinholdt
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

// KDE includes
#include <kdebug.h>

// application specific includes
#include <smb4kauthinfo.h>


Smb4KAuthInfo::Smb4KAuthInfo( const Smb4KHost *host )
: m_url( QUrl() ), m_type( Host ), m_workgroup( host->workgroupName() ), m_homes_share( false ),
  m_homes_users( QStringList() )
{
  setUNC( host->unc( QUrl::None ) );
}


Smb4KAuthInfo::Smb4KAuthInfo( const Smb4KShare *share )
: m_url( QUrl() ), m_type( Share ), m_workgroup( share->workgroupName() ), m_homes_share( share->isHomesShare() ),
  m_homes_users( share->homesUsers() )
{
  setUNC( share->unc( QUrl::None ) );
}


Smb4KAuthInfo::Smb4KAuthInfo()
: m_url( QUrl() ), m_type( Unknown ), m_workgroup( QString() ), m_homes_share( false ),
  m_homes_users( QStringList() )
{
}


Smb4KAuthInfo::Smb4KAuthInfo( Smb4KAuthInfo &i )
: m_url( QUrl() ), m_type( i.type() ), m_workgroup( i.workgroupName() ), m_homes_share( i.isHomesShare() ),
  m_homes_users( i.homesUsers() )
{
  setUNC( i.unc( QUrl::None ) );
}


Smb4KAuthInfo::~Smb4KAuthInfo()
{
}


void Smb4KAuthInfo::setHost( Smb4KHost *host )
{
  Q_ASSERT( host );

  m_type        = Host;
  m_workgroup   = host->workgroupName();
  m_homes_share = false;
  m_homes_users = QStringList();

  setUNC( host->unc( QUrl::None ) );
}


void Smb4KAuthInfo::setShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  m_type        = Share;
  m_workgroup   = share->workgroupName();
  m_homes_share = share->isHomesShare();
  m_homes_users = share->homesUsers();

  setUNC( share->unc( QUrl::None ) );
}


void Smb4KAuthInfo::setWorkgroupName( const QString &workgroup )
{
  m_workgroup = workgroup;
}


void Smb4KAuthInfo::setUNC( const QString &unc )
{
  // Check that a valid UNC was passed to this function.
  if ( !unc.startsWith( "//" ) && !unc.startsWith( "smb:" ) &&
       !(unc.count( "/" ) == 2 || unc.count( "/" ) == 3) )
  {
    // The UNC is malformatted.
    return;
  }
  else
  {
    // Do nothing
  }

  // Set the type.
  if ( unc.count( "/" ) == 3 )
  {
    m_type = Share;
  }
  else
  {
    m_type = Host;
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

  // Determine whether this is a homes share.
  m_homes_share = (QString::compare( m_url.path().remove( 0, 1 ), "homes", Qt::CaseSensitive ) == 0);
}


QString Smb4KAuthInfo::unc( QUrl::FormattingOptions options ) const
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


QString Smb4KAuthInfo::hostUNC( QUrl::FormattingOptions options ) const
{
  return m_url.toString( options|QUrl::RemovePath ).replace( "//"+m_url.host(), "//"+hostName() );
}


QString Smb4KAuthInfo::shareName() const
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


void Smb4KAuthInfo::setLogin( const QString &login )
{
  m_url.setUserName( login );
}


void Smb4KAuthInfo::setPassword( const QString &passwd )
{
  m_url.setPassword( passwd );
}


void Smb4KAuthInfo::setHomesUser( const QString &user )
{
  if ( m_homes_share )
  {
    m_url.setPath( user );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KAuthInfo::setDefaultAuthInfo()
{
  m_type = Default;
}


void Smb4KAuthInfo::setHomesUsers( const QStringList &users )
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
