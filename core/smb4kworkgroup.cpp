/***************************************************************************
    Smb4K's container class for information about a workgroup.
                             -------------------
    begin                : Sa Jan 26 2008
    copyright            : (C) 2008-2017 by Alexander Reinholdt
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
#include "smb4kworkgroup.h"
#include "smb4kglobal.h"

// Qt includes
#include <QAbstractSocket>
#include <QUrl>

// KDE includes
#include <KIconThemes/KIconLoader>

using namespace Smb4KGlobal;

class Smb4KWorkgroupPrivate
{
  public:
    QUrl masterURL;
    QHostAddress masterIP;
};


Smb4KWorkgroup::Smb4KWorkgroup(const QString &name)
: Smb4KBasicNetworkItem(Workgroup), d(new Smb4KWorkgroupPrivate)
{
  // 
  // Set the URL of the workgroup
  // 
  pUrl->setScheme("smb");
  pUrl->setHost(name);
  
  //
  // Set the icon
  // 
  *pIcon = KDE::icon("network-workgroup");
}


Smb4KWorkgroup::Smb4KWorkgroup(const Smb4KWorkgroup &w)
: Smb4KBasicNetworkItem(Workgroup), d(new Smb4KWorkgroupPrivate)
{
  // Copy the private variables
  // 
  *d = *w.d;
  
  //
  // Set the icon if necessary
  // 
  if (pIcon->isNull())
  {
    *pIcon = KDE::icon("network-workgroup");
  }
}


Smb4KWorkgroup::Smb4KWorkgroup()
: Smb4KBasicNetworkItem(Workgroup), d(new Smb4KWorkgroupPrivate)
{
  //
  // Set the URL
  //
  pUrl->setScheme("smb");
  
  //
  // Set the icon
  // 
  *pIcon = KDE::icon("network-workgroup");  
}


Smb4KWorkgroup::~Smb4KWorkgroup()
{
}


void Smb4KWorkgroup::setWorkgroupName(const QString &name)
{
  pUrl->setHost(name);
  pUrl->setScheme("smb");
}


QString Smb4KWorkgroup::workgroupName() const
{
  return pUrl->host().toUpper();
}


void Smb4KWorkgroup::setMasterBrowserName(const QString &name)
{
  d->masterURL.setHost(name);
  d->masterURL.setScheme("smb");
}


QString Smb4KWorkgroup::masterBrowserName() const
{
  return d->masterURL.host().toUpper();
}


bool Smb4KWorkgroup::hasMasterBrowser() const
{
  return !d->masterURL.host().isEmpty();
}


void Smb4KWorkgroup::setMasterBrowserIpAddress(const QString &ip)
{
  d->masterIP.setAddress(ip);
}


void Smb4KWorkgroup::setMasterBrowserIpAddress(const QHostAddress& address)
{
  if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    d->masterIP = address;
  }
}


QString Smb4KWorkgroup::masterBrowserIpAddress() const
{
  return d->masterIP.toString();
}


bool Smb4KWorkgroup::hasMasterBrowserIpAddress() const
{
  return !d->masterIP.isNull();
}


void Smb4KWorkgroup::update(Smb4KWorkgroup* workgroup)
{
  if (QString::compare(workgroupName(), workgroup->workgroupName()) == 0)
  {
    setMasterBrowserName(workgroup->masterBrowserName());
    setMasterBrowserIpAddress(workgroup->masterBrowserIpAddress());
  }

  
  
  
}


