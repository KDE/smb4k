/***************************************************************************
    This class derives from QObject and encapsulates the network items. 
    It is for use with QtQuick.
                             -------------------
    begin                : Fr Mär 02 2012
    copyright            : (C) 2012-2019 by Alexander Reinholdt
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
#include "smb4knetworkobject.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDebug>

using namespace Smb4KGlobal;


class Smb4KNetworkObjectPrivate
{
  public:
    QString workgroup;
    QUrl url;
    int type;
    int parentType;
    QString comment;
    bool mounted;
    QUrl mountpoint;
    bool printer;
    bool isMaster;
    bool inaccessible;
};


Smb4KNetworkObject::Smb4KNetworkObject(Smb4KWorkgroup *workgroup, QObject *parent)
: QObject(parent), d(new Smb4KNetworkObjectPrivate)
{
  d->workgroup    = workgroup->workgroupName();
  d->url          = workgroup->url();
  d->mounted      = false;
  d->inaccessible = false;
  d->printer      = false;
  d->isMaster     = false;
  setType(Workgroup);
}


Smb4KNetworkObject::Smb4KNetworkObject(Smb4KHost *host, QObject *parent)
: QObject(parent), d(new Smb4KNetworkObjectPrivate)
{
  d->workgroup    = host->workgroupName();
  d->url          = host->url();
  d->comment      = host->comment();
  d->mounted      = false;
  d->inaccessible = false;
  d->printer      = false;
  d->isMaster     = host->isMasterBrowser();
  setType(Host);
}


Smb4KNetworkObject::Smb4KNetworkObject(Smb4KShare *share, QObject *parent)
: QObject(parent), d(new Smb4KNetworkObjectPrivate)
{
  d->workgroup    = share->workgroupName();
  d->url          = share->url();
  d->comment      = share->comment();
  d->mounted      = share->isMounted();
  d->inaccessible = share->isInaccessible();
  d->printer      = share->isPrinter();
  d->isMaster     = false;
  d->mountpoint   = QUrl::fromLocalFile(share->path());
  setType(Share);
}


Smb4KNetworkObject::Smb4KNetworkObject(QObject *parent)
: QObject(parent), d(new Smb4KNetworkObjectPrivate)
{
  d->url.setUrl("smb://", QUrl::TolerantMode);
  d->mounted      = false;
  d->inaccessible = false;
  d->printer      = false;
  d->isMaster     = false;
  setType(Network);
}


Smb4KNetworkObject::~Smb4KNetworkObject()
{
}


Smb4KNetworkObject::NetworkItem Smb4KNetworkObject::type() const
{
  return static_cast<Smb4KNetworkObject::NetworkItem>(d->type);
}


Smb4KNetworkObject::NetworkItem Smb4KNetworkObject::parentType() const
{
  return static_cast<Smb4KNetworkObject::NetworkItem>(d->parentType);
}


void Smb4KNetworkObject::setType(NetworkItem type)
{
  d->type = type;
  
  switch (type)
  {
    case Host:
    {
      d->parentType = Workgroup;
      break;
    }
    case Share:
    {
      d->parentType = Host;
      break;
    }
    default:
    {
      d->parentType = Network;
      break;
    }
  }
  emit changed();
}


QString Smb4KNetworkObject::workgroupName() const
{
  return d->workgroup;
}


void Smb4KNetworkObject::setWorkgroupName(const QString& name)
{
  d->workgroup = name;
  emit changed();
}


QString Smb4KNetworkObject::hostName() const
{
  return d->url.host().toUpper();
}


void Smb4KNetworkObject::setHostName(const QString& name)
{
  d->url.setHost(name);
  emit changed();
}


bool Smb4KNetworkObject::isMasterBrowser() const
{
  return d->isMaster;
}


void Smb4KNetworkObject::setMasterBrowser(bool master)
{
  if (type() == Host)
  {
    d->isMaster = master;
    emit changed();
  }
}


QString Smb4KNetworkObject::shareName() const
{
  // Since users might come up with very weird share names,
  // we are careful and do not use QString::remove("/"), but
  // only remove preceding and trailing slashes.
  QString share_name = d->url.path();

  if (share_name.startsWith('/'))
  {
    share_name.remove(0, 1);
  }

  if (share_name.endsWith('/'))
  {
    share_name.remove(share_name.size() - 1, 1);
  }

  return share_name;
}


void Smb4KNetworkObject::setShareName(const QString& name)
{
  d->url.setPath(name);
  emit changed();
}


QString Smb4KNetworkObject::name() const
{
  QString name;
  
  switch (d->type)
  {
    case Workgroup:
    {
      name = workgroupName();
      break;
    }
    case Host:
    {
      name = hostName();
      break;
    }
    case Share:
    {
      name = shareName();
      break;
    }
    default:
    {
      break;
    }
  }
  
  return name;
}


QString Smb4KNetworkObject::comment() const
{
  return d->comment;
}


void Smb4KNetworkObject::setComment(const QString& comment)
{
  d->comment = comment;
  emit changed();
}


QUrl Smb4KNetworkObject::url() const
{
  return d->url;
}


QUrl Smb4KNetworkObject::parentUrl() const
{
  // Do not use QUrl::upUrl() here, because it produces
  // an URL like this: smb://HOST/Share/../ and we do not
  // want that.
  QUrl parent_url;
  parent_url.setUrl("smb://");

  switch (d->type)
  {
    case Host:
    {
      parent_url.setHost(d->workgroup);
      break;
    }
    case Share:
    {
      parent_url.setHost(d->url.host());
      break;
    }
    default:
    {
      break;
    }
  }
  
  return parent_url;
}


void Smb4KNetworkObject::setUrl(const QUrl& url)
{
  d->url = url;
  emit changed();
}


bool Smb4KNetworkObject::isMounted() const
{
  return d->mounted;
}


void Smb4KNetworkObject::setMounted(bool mounted)
{
  d->mounted = mounted;
  emit changed();
}


void Smb4KNetworkObject::update(Smb4KBasicNetworkItem *networkItem)
{
  if (d->type == Workgroup && networkItem->type() == Smb4KGlobal::Workgroup)
  {
    Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(networkItem);
    
    if (workgroup)
    {
      // Check that we update with the correct item.
      if (QString::compare(workgroupName(), workgroup->workgroupName(), Qt::CaseInsensitive) == 0)
      {
        d->workgroup    = workgroup->workgroupName();
        d->url          = workgroup->url();
        d->type         = Workgroup;
        d->mounted      = false;
        d->inaccessible = false;
        d->printer      = false;
      }
    }
  }
  else if (d->type == Host && networkItem->type() == Smb4KGlobal::Host)
  {
    Smb4KHost *host = static_cast<Smb4KHost *>(networkItem);
    
    if (host)
    {
      // Check that we update with the correct item.
      if (QString::compare(workgroupName(), host->workgroupName(), Qt::CaseInsensitive) == 0 &&
          QString::compare(hostName(), host->hostName(), Qt::CaseInsensitive) == 0)
      {
        d->workgroup    = host->workgroupName();
        d->url          = host->url();
        d->comment      = host->comment();
        d->type         = Host;
        d->mounted      = false;
        d->inaccessible = false;
        d->printer      = false;
      }
    }
  }
  else if (d->type == Share && networkItem->type() == Smb4KGlobal::Share)
  {
    Smb4KShare *share = static_cast<Smb4KShare *>(networkItem);
    
    if (share)
    {
      // Check that we update with the correct item.
      if (QString::compare(workgroupName(), share->workgroupName(), Qt::CaseInsensitive) == 0 &&
          QString::compare(hostName(), share->hostName(), Qt::CaseInsensitive) == 0 &&
          QString::compare(shareName(), share->shareName(), Qt::CaseInsensitive) == 0)
      {
        d->workgroup    = share->workgroupName();
        d->url          = share->url();
        d->comment      = share->comment();
        d->type         = Share;
        d->mounted      = share->isMounted();
        d->inaccessible = share->isInaccessible();
        d->printer      = share->isPrinter();
        d->mountpoint.setUrl(share->path(), QUrl::TolerantMode);
        d->mountpoint.setScheme("file");
      }
    }
  }
  else
  {
    d->type = Network;
  }
  
  emit changed();
}


bool Smb4KNetworkObject::isPrinter() const
{
  return d->printer;
}


void Smb4KNetworkObject::setPrinter(bool printer)
{
  d->printer = printer;
  emit changed();
}


QUrl Smb4KNetworkObject::mountpoint() const
{
  return d->mountpoint;
}


void Smb4KNetworkObject::setMountpoint(const QUrl &mountpoint)
{
  d->mountpoint = mountpoint;
  emit changed();
}


bool Smb4KNetworkObject::isInaccessible() const
{
  return (d->mounted && d->inaccessible);
}


void Smb4KNetworkObject::setInaccessible(bool inaccessible)
{
  d->inaccessible = inaccessible;
}



