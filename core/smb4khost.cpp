/***************************************************************************
    Smb4K's container class for information about a host.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "smb4khost.h"
#include "smb4kauthinfo.h"

// Qt includes
#include <QStringList>
#include <QDebug>
#include <QUrl>

// KDE includes
#include <KIconThemes/KIconLoader>


class Smb4KHostPrivate
{
  public:
    QString workgroup;
    QHostAddress ip;
    QString comment;
    bool isMaster;
};


Smb4KHost::Smb4KHost(const QString &name)
: Smb4KBasicNetworkItem(Host), d(new Smb4KHostPrivate)
{
  d->isMaster = false;
  *pIcon = KDE::icon("network-server");
  setHostName(name);
}


Smb4KHost::Smb4KHost(const Smb4KHost &h)
: Smb4KBasicNetworkItem(Host), d(new Smb4KHostPrivate)
{
  *d = *h.d;
  
  if (pIcon->isNull())
  {
    *pIcon = KDE::icon("network-server");
  }
  else
  {
    // Do nothing
  }
}


Smb4KHost::Smb4KHost()
: Smb4KBasicNetworkItem(Host), d(new Smb4KHostPrivate)
{
  d->isMaster = false;
  *pIcon = KDE::icon("network-server");
}


Smb4KHost::~Smb4KHost()
{
}


void Smb4KHost::setHostName(const QString &name)
{
  pUrl->setHost(name);
  pUrl->setScheme("smb");
}


QString Smb4KHost::hostName() const
{
  return pUrl->host().toUpper();
}


void Smb4KHost::setWorkgroupName(const QString &workgroup)
{
  d->workgroup = workgroup.toUpper();
}


QString Smb4KHost::workgroupName() const
{
  return d->workgroup;
}


void Smb4KHost::setIpAddress(const QString &ip)
{
  d->ip.setAddress(ip);
}


void Smb4KHost::setIpAddress(const QHostAddress& address)
{
  if (!address.isNull() && address.protocol() != QAbstractSocket::UnknownNetworkLayerProtocol)
  {
    d->ip = address;
  }
  else
  {
    // Do nothing
  }
}


QString Smb4KHost::ipAddress() const
{
  return d->ip.toString();
}


bool Smb4KHost::hasIpAddress() const
{
  return !d->ip.isNull();
}


void Smb4KHost::setComment(const QString &comment)
{
  d->comment = comment;
}


QString Smb4KHost::comment() const
{
  return d->comment;
}


void Smb4KHost::setIsMasterBrowser(bool master)
{
  d->isMaster = master;
}


bool Smb4KHost::isMasterBrowser() const
{
  return d->isMaster;
}


void Smb4KHost::setLogin(const QString &login)
{
  pUrl->setUserName(login);
}


QString Smb4KHost::login() const
{
  return pUrl->userName();
}


void Smb4KHost::setPassword(const QString &passwd)
{
  pUrl->setPassword(passwd);
}


QString Smb4KHost::password() const
{
  return pUrl->password();
}


void Smb4KHost::setPort(int port)
{
  pUrl->setPort(port);
}


int Smb4KHost::port() const
{
  return pUrl->port();
}


void Smb4KHost::setAuthInfo(Smb4KAuthInfo *authInfo)
{
  pUrl->setUserName(authInfo->userName());
  pUrl->setPassword(authInfo->password());
}


void Smb4KHost::update(Smb4KHost* host)
{
  if (QString::compare(workgroupName(), host->workgroupName()) == 0 &&
      QString::compare(hostName(), host->hostName()) == 0)
  {
    *pUrl = host->url();
    setComment(host->comment());
    setIsMasterBrowser(host->isMasterBrowser());
    
    // Do not kill the already discovered IP address
    if (!hasIpAddress() && host->hasIpAddress())
    {
      setIpAddress(host->ipAddress());
    }
    else
    {
      // Do nothing
    }    
  }
  else
  {
    // Do nothing
  }
}

