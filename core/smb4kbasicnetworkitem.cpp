/***************************************************************************
    This class provides the basic network item for the core library of 
    Smb4K.
                             -------------------
    begin                : Do Apr 2 2009
    copyright            : (C) 2009-2019 by Alexander Reinholdt
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
#include <QtGlobal>
#include <QDebug>

using namespace Smb4KGlobal;

class Smb4KBasicNetworkItemPrivate
{
  public:
    NetworkItem type;
    QIcon icon;
    QUrl url;
};


Smb4KBasicNetworkItem::Smb4KBasicNetworkItem(NetworkItem type)
: d(new Smb4KBasicNetworkItemPrivate)
{
  //
  // Set the type
  // 
  d->type = type;
  
  //
  // Initialize the protected variables
  // 
  pUrl = &d->url;
  pIcon = &d->icon;
}

Smb4KBasicNetworkItem::Smb4KBasicNetworkItem(const Smb4KBasicNetworkItem &item)
: d(new Smb4KBasicNetworkItemPrivate)
{
  //
  // Copy the private variables
  // 
  *d = *item.d;
  
  //
  // Initialize the protected variables
  // 
  pUrl = &d->url;
  pIcon = &d->icon;
}


Smb4KBasicNetworkItem::~Smb4KBasicNetworkItem()
{
}


Smb4KGlobal::NetworkItem Smb4KBasicNetworkItem::type() const
{
  return d->type;
}


void Smb4KBasicNetworkItem::setIcon(const QIcon &icon)
{
  d->icon = icon;
}


QIcon Smb4KBasicNetworkItem::icon() const
{
  return d->icon;
}


void Smb4KBasicNetworkItem::setUrl(const QUrl& url)
{
  //
  // Check that the URL is valid
  // 
  if (!url.isValid())
  {
    return;
  }
  
  //
  // Do some checks depending on the type of the network item
  // 
  switch (d->type)
  {
    case Network:
    {
      break;
    }
    case Workgroup:
    case Host:
    {
      // 
      // Check that the host name is present and there is no path
      // 
      if (url.host().isEmpty() || !url.path().isEmpty())
      {
        return;
      }
      
      break;
    }
    case Share:
    {
      //
      // Check that the share name is present
      // 
      if (url.path().isEmpty() || (url.path().size() == 1 && url.path().endsWith('/')))
      {
        return;
      }
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  //
  // Set the URL
  //
  d->url = url;
  
  //
  // Force the scheme
  // 
  if (d->url.scheme() != "smb")
  {
    d->url.setScheme("smb");
  }
}


QUrl Smb4KBasicNetworkItem::url() const
{
  return d->url;
}


