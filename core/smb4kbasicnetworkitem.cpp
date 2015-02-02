/***************************************************************************
    smb4kbasicnetworkitem  -  This class provides the basic network item
    for the core library of Smb4K.
                             -------------------
    begin                : Do Apr 2 2009
    copyright            : (C) 2009-2015 by Alexander Reinholdt
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
#include "smb4kbasicnetworkitem.h"

// Qt includes
#include <QtCore/QtGlobal>

// KDE includes
#include <kdebug.h>


class Smb4KBasicNetworkItemPrivate
{
  public:
    NetworkItem type;
    QString key;
    QIcon icon;
};


Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( NetworkItem type )
: d( new Smb4KBasicNetworkItemPrivate )
{
  d->type = type;
  d->key  = QString("%1").arg(qrand());
}

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( const Smb4KBasicNetworkItem &item )
: d( new Smb4KBasicNetworkItemPrivate )
{
  *d = *item.d;
}


Smb4KBasicNetworkItem::~Smb4KBasicNetworkItem()
{
}


Smb4KGlobal::NetworkItem Smb4KBasicNetworkItem::type() const
{
  return d->type;
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

