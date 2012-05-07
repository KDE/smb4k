/***************************************************************************
    smb4kbasicnetworkitem  -  This class provides the basic network item
    for the core library of Smb4K.
                             -------------------
    begin                : Do Apr 2 2009
    copyright            : (C) 2009-2012 by Alexander Reinholdt
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
#include "smb4kbasicnetworkitem.h"

// KDE includes
#include <kdebug.h>

// system specific includes
#include <stdlib.h>


class Smb4KBasicNetworkItemPrivate
{
  public:
    int type;
    QString key;
    QIcon icon;
};


Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( Smb4KBasicNetworkItem::Type type )
: d( new Smb4KBasicNetworkItemPrivate )
{
  d->type = type;
  d->key  = QString( rand() );
}

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( const Smb4KBasicNetworkItem &item )
: d( new Smb4KBasicNetworkItemPrivate )
{
  *d = *item.d;
}


Smb4KBasicNetworkItem::~Smb4KBasicNetworkItem()
{
}


Smb4KBasicNetworkItem::Type Smb4KBasicNetworkItem::type() const
{
  return static_cast<Type>( d->type );
}


void Smb4KBasicNetworkItem::setKey( const QString &key )
{
  d->key = key;
}


QString Smb4KBasicNetworkItem::key() const
{
  return d->key;
}


void Smb4KBasicNetworkItem::setIcon( const QIcon &icon )
{
  d->icon = icon;
}


QIcon Smb4KBasicNetworkItem::icon() const
{
  return d->icon;
}

