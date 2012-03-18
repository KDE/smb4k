/***************************************************************************
    smb4kworkgroup  -  Smb4K's container class for information about a
    workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2010 by Alexander Reinholdt
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

// KDE includes
#include <kicon.h>

// application specific includesustpuppy@users.berlios.de
#include <smb4kworkgroup.h>

Smb4KWorkgroup::Smb4KWorkgroup( const QString &name ) : Smb4KBasicNetworkItem( Workgroup ),
  m_url( QUrl() ), m_master_name( QString() ), m_master_ip( QString() ), m_pseudo_master( false )
{
  m_url.setHost( name );
  m_url.setScheme( "smb" );
  setIcon( KIcon( "network-workgroup" ) );
}


Smb4KWorkgroup::Smb4KWorkgroup( const Smb4KWorkgroup &w ) : Smb4KBasicNetworkItem( Workgroup ),
  m_url( w.url() ), m_master_name( w.masterBrowserName() ), m_master_ip( w.masterBrowserIP() ),
  m_pseudo_master( w.hasPseudoMasterBrowser() )
{
  if ( icon().isNull() )
  {
    setIcon( KIcon( "network-workgroup" ) );
  }
  else
  {
    // Do nothing
  }
}


Smb4KWorkgroup::Smb4KWorkgroup() : Smb4KBasicNetworkItem( Workgroup ),
  m_url( QUrl() ), m_master_name( QString() ), m_master_ip( QString() ), m_pseudo_master( false )
{
}


Smb4KWorkgroup::~Smb4KWorkgroup()
{
}


void Smb4KWorkgroup::setWorkgroupName( const QString &name )
{
  m_url.setHost( name );
  m_url.setScheme( "smb" );
}


void Smb4KWorkgroup::setMasterBrowserName( const QString &name )
{
  m_master_name = name;
}


void Smb4KWorkgroup::setMasterBrowserIP( const QString &ip )
{
  m_master_ip = ipIsValid( ip );
}


void Smb4KWorkgroup::setHasPseudoMasterBrowser( bool pseudo )
{
  m_pseudo_master = pseudo;
}


bool Smb4KWorkgroup::isEmpty() const
{
  // Ignore all booleans.

  if ( !m_url.host().isEmpty() )
  {
    return false;
  }

  if ( !m_master_name.isEmpty() )
  {
    return false;
  }

  if ( !m_master_ip.isEmpty() )
  {
    return false;
  }
  
  // Do not include the icon here.

  return true;
}


bool Smb4KWorkgroup::equals( Smb4KWorkgroup *workgroup ) const
{
  Q_ASSERT( workgroup );

  if ( QString::compare( workgroupName(), workgroup->workgroupName() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_master_name, workgroup->masterBrowserName() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( m_master_ip, workgroup->masterBrowserIP() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( m_pseudo_master != workgroup->hasPseudoMasterBrowser() )
  {
    return false;
  }
  else
  {
    // Do nothing
  }
  
  // Do not include the icon here.

  return true;
}


const QString &Smb4KWorkgroup::ipIsValid( const QString &ip )
{
  QHostAddress ip_address( ip );

  if ( ip_address.protocol() == QAbstractSocket::UnknownNetworkLayerProtocol )
  {
    // The IP address is invalid.
    static_cast<QString>( ip ).clear();
  }

  return ip;
}

