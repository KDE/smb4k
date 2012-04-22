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

// KDE includes
#include <kdebug.h>

// system specific includes
#include <stdlib.h>

// application specific includes
#include <smb4kbasicnetworkitem.h>


Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( Smb4KBasicNetworkItem::Type type )
: m_type( type ), m_key( QString( rand() ) ), m_icon( QIcon() )
{
}

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem( const Smb4KBasicNetworkItem &item )
: m_type( item.type() ), m_key( item.key() ), m_icon( item.icon() )
{
}


Smb4KBasicNetworkItem::~Smb4KBasicNetworkItem()
{
}


void Smb4KBasicNetworkItem::setKey( const QString &key )
{
  m_key = key;
}


void Smb4KBasicNetworkItem::setIcon( const QIcon &icon )
{
  m_icon = icon;
}

