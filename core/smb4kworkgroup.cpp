/***************************************************************************
    smb4kworkgroup  -  Smb4K's container class for information about a
    workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4kworkgroup.h"

// Qt includes
#include <QtNetwork/QHostAddress>

// KDE includes
#include <kicon.h>

class Smb4KWorkgroupPrivate
{
  public:
    QUrl url;
    QUrl masterURL;
    QHostAddress masterIP;
    bool pseudoMaster;
};


Smb4KWorkgroup::Smb4KWorkgroup( const QString &name )
: Smb4KBasicNetworkItem( Workgroup ), d( new Smb4KWorkgroupPrivate )
{
  d->pseudoMaster = false;
  d->url.setHost( name );
  d->url.setScheme( "smb" );
  setIcon( KIcon( "network-workgroup" ) );
}


Smb4KWorkgroup::Smb4KWorkgroup( const Smb4KWorkgroup &w )
: Smb4KBasicNetworkItem( Workgroup ), d( new Smb4KWorkgroupPrivate )
{
  *d = *w.d;
  
  if ( icon().isNull() )
  {
    setIcon( KIcon( "network-workgroup" ) );
  }
  else
  {
    // Do nothing
  }
}


Smb4KWorkgroup::Smb4KWorkgroup()
: Smb4KBasicNetworkItem( Workgroup ), d( new Smb4KWorkgroupPrivate )
{
  d->pseudoMaster = false;
  setIcon( KIcon( "network-workgroup" ) );  
}


Smb4KWorkgroup::~Smb4KWorkgroup()
{
}


void Smb4KWorkgroup::setWorkgroupName( const QString &name )
{
  d->url.setHost( name );
  d->url.setScheme( "smb" );
}


QString Smb4KWorkgroup::workgroupName() const
{
  return d->url.host().toUpper();
}


void Smb4KWorkgroup::setMasterBrowserName( const QString &name )
{
  d->masterURL.setHost( name );

  if ( d->masterURL.scheme().isEmpty() )
  {
    d->masterURL.setScheme( "smb" );
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KWorkgroup::masterBrowserName() const
{
  return d->masterURL.host().toUpper();
}


void Smb4KWorkgroup::setMasterBrowserIP( const QString &ip )
{
  d->masterIP.setAddress( ip );
}


QString Smb4KWorkgroup::masterBrowserIP() const
{
  return d->masterIP.toString();
}


void Smb4KWorkgroup::setHasPseudoMasterBrowser( bool pseudo )
{
  d->pseudoMaster = pseudo;
}


bool Smb4KWorkgroup::hasPseudoMasterBrowser() const
{
  return d->pseudoMaster;
}


bool Smb4KWorkgroup::isEmpty() const
{
  // Ignore all booleans.

  if ( !d->url.host().isEmpty() )
  {
    return false;
  }

  if ( !d->masterURL.host().isEmpty() )
  {
    return false;
  }

  if ( !d->masterIP.isNull() )
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

  if ( QString::compare( masterBrowserName(), workgroup->masterBrowserName() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( QString::compare( masterBrowserIP(), workgroup->masterBrowserIP() ) != 0 )
  {
    return false;
  }
  else
  {
    // Do nothing
  }

  if ( hasPseudoMasterBrowser() != workgroup->hasPseudoMasterBrowser() )
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


bool Smb4KWorkgroup::hasMasterBrowserIP() const
{
  return !d->masterIP.isNull();
}


QUrl Smb4KWorkgroup::url() const
{
  return d->url;
}


