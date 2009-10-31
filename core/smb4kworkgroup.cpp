/***************************************************************************
    smb4kworkgroup  -  Smb4K's container class for information about a
    workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

// application specific includes
#include <smb4kworkgroup.h>

Smb4KWorkgroup::Smb4KWorkgroup( const QString &name ) : Smb4KBasicNetworkItem( Workgroup ),
  m_name( name ), m_master_name( QString() ), m_master_ip( QString() ), m_pseudo_master( false )
{
}


Smb4KWorkgroup::Smb4KWorkgroup( const Smb4KWorkgroup &w ) : Smb4KBasicNetworkItem( Workgroup ),
  m_name( w.workgroupName() ), m_master_name( w.masterBrowserName() ), m_master_ip( w.masterBrowserIP() ),
  m_pseudo_master( w.hasPseudoMasterBrowser() )
{
}


Smb4KWorkgroup::Smb4KWorkgroup() : Smb4KBasicNetworkItem( Workgroup ),
  m_name( QString() ), m_master_name( QString() ), m_master_ip( QString() ), m_pseudo_master( false )
{
}


Smb4KWorkgroup::~Smb4KWorkgroup()
{
}


void Smb4KWorkgroup::setWorkgroupName( const QString &name )
{
  m_name = name;
}


void Smb4KWorkgroup::setMasterBrowser( const QString &name, const QString &ip, bool pseudo )
{
  m_master_name   = name;
  m_master_ip     = ipIsValid( ip );
  m_pseudo_master = pseudo;
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

  if ( !m_name.isEmpty() )
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

  return true;
}


bool Smb4KWorkgroup::equals( Smb4KWorkgroup *workgroup )
{
  Q_ASSERT( workgroup );

  if ( QString::compare( m_name, workgroup->workgroupName() ) != 0 )
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

