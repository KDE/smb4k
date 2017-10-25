/***************************************************************************
    This class provides the interface for Plasma and QtQuick
                             -------------------
    begin                : Mo 02 Sep 2013
    copyright            : (C) 2013-2017 by Alexander Reinholdt
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
#include "smb4kdeclarative.h"
#include "smb4kdeclarative_p.h"
#include "smb4kbookmarkobject.h"
#include "smb4knetworkobject.h"
#include "smb4kprofileobject.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4kglobal.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kbasicnetworkitem.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kprint.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kpreviewer.h"

// Qt includes
#include <QDebug>
#include <QTest>

using namespace Smb4KGlobal;


Smb4KDeclarative::Smb4KDeclarative(QObject* parent)
: QObject(parent), d(new Smb4KDeclarativePrivate)
{
  // Initialize the core.
  Smb4KGlobal::initCore(true, false);
  
  // Connections
  connect(Smb4KScanner::self(), SIGNAL(workgroups()), this, SLOT(slotWorkgroupsListChanged()));
  connect(Smb4KScanner::self(), SIGNAL(hosts(Smb4KWorkgroup*)), this, SLOT(slotHostsListChanged()));
  connect(Smb4KScanner::self(), SIGNAL(shares(Smb4KHost*)), this, SLOT(slotSharesListChanged()));
  connect(Smb4KScanner::self(), SIGNAL(aboutToStart(Smb4KBasicNetworkItem*,int)), this, SIGNAL(busy()));
  connect(Smb4KScanner::self(), SIGNAL(finished(Smb4KBasicNetworkItem*,int)), this, SIGNAL(idle()));
  
  connect(Smb4KMounter::self(), SIGNAL(mountedSharesListChanged()), this, SLOT(slotMountedSharesListChanged()));
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)), this, SIGNAL(busy()));
  connect(Smb4KMounter::self(), SIGNAL(finished(int)), this, SIGNAL(idle()));
  
  connect(Smb4KPrint::self(), SIGNAL(aboutToStart(Smb4KShare*)), this, SIGNAL(busy()));
  connect(Smb4KPrint::self(), SIGNAL(finished(Smb4KShare*)), this, SIGNAL(idle()));
  
  connect(Smb4KBookmarkHandler::self(), SIGNAL(updated()), this, SLOT(slotBookmarksListChanged()));
  
  connect(Smb4KProfileManager::self(), SIGNAL(profilesListChanged(QStringList)), this, SLOT(slotProfilesListChanged(QStringList)));
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), this, SLOT(slotActiveProfileChanged(QString)));
  connect(Smb4KProfileManager::self(), SIGNAL(profileUsageChanged(bool)), this, SLOT(slotProfileUsageChanged(bool)));
  
  // Do the initial loading of items.
  slotBookmarksListChanged();
  slotProfilesListChanged(Smb4KProfileManager::self()->profilesList());
  slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
  slotProfileUsageChanged(Smb4KProfileManager::self()->useProfiles());
}


Smb4KDeclarative::~Smb4KDeclarative()
{
  while (!d->workgroupObjects.isEmpty())
  {
    delete d->workgroupObjects.takeFirst();
  }

  while (!d->hostObjects.isEmpty())
  {
    delete d->hostObjects.takeFirst();
  }

  while (!d->shareObjects.isEmpty())
  {
    delete d->shareObjects.takeFirst();
  }
  
  while (!d->mountedObjects.isEmpty())
  {
    delete d->mountedObjects.takeFirst();
  }
  
  while (!d->bookmarkObjects.isEmpty())
  {
    delete d->bookmarkObjects.takeFirst();
  }
  
  while (!d->bookmarkGroupObjects.isEmpty())
  {
    delete d->bookmarkGroupObjects.takeFirst();
  }
  
  while (!d->profileObjects.isEmpty())
  {
    delete d->profileObjects.takeFirst();
  }
}


QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::workgroups()
{
  return QQmlListProperty<Smb4KNetworkObject>(this, d->workgroupObjects);
}


QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::hosts()
{
  return QQmlListProperty<Smb4KNetworkObject>(this, d->hostObjects);
}


QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::shares()
{
  return QQmlListProperty<Smb4KNetworkObject>(this, d->shareObjects);
}


QQmlListProperty<Smb4KNetworkObject> Smb4KDeclarative::mountedShares()
{
  return QQmlListProperty<Smb4KNetworkObject>(this, d->mountedObjects);
}


QQmlListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarks()
{
  return QQmlListProperty<Smb4KBookmarkObject>(this, d->bookmarkObjects);
}


QQmlListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarkGroups()
{
  return QQmlListProperty<Smb4KBookmarkObject>(this, d->bookmarkGroupObjects);
}


QQmlListProperty<Smb4KProfileObject> Smb4KDeclarative::profiles()
{
  return QQmlListProperty<Smb4KProfileObject>(this, d->profileObjects);
}


void Smb4KDeclarative::lookup(Smb4KNetworkObject *object)
{
  if (object)
  {
    switch (object->type())
    {
      case Network:
      {
        Smb4KScanner::self()->lookupDomains();
        break;
      }
      case Workgroup:
      {
        // Check if the workgroup is known.
        WorkgroupPtr workgroup = findWorkgroup(object->url().host().toUpper());
        
        if (workgroup)
        {
          Smb4KScanner::self()->lookupDomainMembers(workgroup);
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Host:
      {
        // Check if the host is known.
        HostPtr host = findHost(object->url().host().toUpper());
        
        if (host)
        {
          Smb4KScanner::self()->lookupShares(host);
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Share:
      {
        break;
      }
      default:
      {
        // Shares are ignored
        break;
      }
    }
  }
  else
  {
    // If the object is 0, scan the whole network.
    Smb4KScanner::self()->lookupDomains();
  }
}


Smb4KNetworkObject *Smb4KDeclarative::findNetworkItem(const QUrl &url, int type)
{
  Smb4KNetworkObject *object = 0;
  
  if (url.isValid())
  {  
    switch (type)
    {
      case Workgroup:
      {
        for (Smb4KNetworkObject *obj : d->workgroupObjects)
        {
          if (url == obj->url())
          {
            object = obj;
            break;
          }
          else
          {
            continue;
          }
        }
        break;
      }
      case Host:
      {
        for (Smb4KNetworkObject *obj : d->hostObjects)
        {
          if (url == obj->url())
          {
            object = obj;
            break;
          }
          else
          {
            continue;
          }
        }
        break;
      }
      case Share:
      {
        for (Smb4KNetworkObject *obj : d->shareObjects)
        {
          if (url == obj->url())
          {
            object = obj;
            break;
          }
          else
          {
            continue;
          }
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  return object;
}


void Smb4KDeclarative::openMountDialog(QWidget* parent)
{
  Smb4KMounter::self()->openMountDialog(parent);
}


void Smb4KDeclarative::mount(Smb4KNetworkObject *object)
{
  if (object && object->type() == Smb4KNetworkObject::Share)
  {
    QString shareName = object->url().path();
      
    if (shareName.startsWith('/'))
    {
      shareName = shareName.mid(1, -1);
    }
    else
    {
      // Do nothing
    }
      
    SharePtr share = findShare("//"+object->hostName()+'/'+object->shareName(), object->workgroupName());
    
    if (share)
    {
      Smb4KMounter::self()->mountShare(share);
    }
    else
    {
      // If the share is not in the global list of shares,
      // try the list of bookmarks.
      QString unc("//"+object->hostName()+"/"+object->shareName());
      BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC(unc);
      
      share = SharePtr(new Smb4KShare());
      share->setURL(object->url());
      share->setWorkgroupName(bookmark->workgroupName());
      share->setHostIP(bookmark->hostIP());
      Smb4KMounter::self()->mountShare(share);
      
      while (Smb4KMounter::self()->isRunning())
      {
        QTest::qWait(50);
      }
      
      share.clear();      
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KDeclarative::unmount(Smb4KNetworkObject *object)
{
  if (object && object->type())
  {
    if (object->mountpoint().isValid())
    {
      SharePtr share = findShareByPath(object->mountpoint().path());
      
      if (share)
      {
        Smb4KMounter::self()->unmountShare(share);
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KDeclarative::unmountAll()
{
  Smb4KMounter::self()->unmountAllShares(false, 0);
}


Smb4KNetworkObject* Smb4KDeclarative::findMountedShare(const QUrl& url, bool exactMatch)
{
  Smb4KNetworkObject *object = 0;
  
  if (url.isValid())
  {
    for (Smb4KNetworkObject *obj : d->mountedObjects)
    {
      if (url.matches(obj->url(), QUrl::None))
      {
        object = obj;
        break;
      }
      else if (!exactMatch && url.matches(obj->url(), QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash))
      {
        object = obj;
        continue;
      }
      else
      {
        continue;
      }
    }
  }
  else
  {
    // Do nothing
  }
  
  return object;
}


void Smb4KDeclarative::print(Smb4KNetworkObject* object)
{
  if (object && object->type() == Smb4KNetworkObject::Share)
  {
    SharePtr printer = findShare("//"+object->hostName()+"/"+object->shareName(), object->workgroupName());
    
    if (printer)
    {
      Smb4KPrint::self()->print(printer, 0);
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


void Smb4KDeclarative::addBookmark(Smb4KNetworkObject* object)
{
  if (object)
  {
    QList<SharePtr> shares; 
    
    // First, search the list of shares gathered by the scanner.
    for (const SharePtr &share : sharesList())
    {
      if (share->url() == object->url())
      {
        shares << share;
        break;
      }
      else
      {
        continue;
      }
    }
    
    // Second, if the list is still empty, try the list of mounted shares.
    if (shares.isEmpty())
    {
      for (const SharePtr &mountedShare : mountedSharesList())
      {
        if (mountedShare->url() == object->url())
        {
          shares << mountedShare;
          break;
        }
        else
        {
          continue;
        }
      }
    }
    else
    {
      // Do nothing
    }
    
    // Now add the share.
    if (!shares.isEmpty())
    {
      Smb4KBookmarkHandler::self()->addBookmarks(shares, 0);
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


void Smb4KDeclarative::removeBookmark(Smb4KBookmarkObject* object)
{
  if (object)
  {
    // Find the bookmark in the list and remove it.
    QString path = (object->url().path().startsWith('/') ? object->url().path().remove(0, 1) : object->url().path());
    QString unc = QString("//%1/%2").arg(object->url().host().toUpper()).arg(path);
    BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC(unc);
    
    if (bookmark)
    {
      Smb4KBookmarkHandler::self()->removeBookmark(bookmark);
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


void Smb4KDeclarative::removeBookmarkGroup(const QString& name)
{
  Smb4KBookmarkHandler::self()->removeGroup(name);
}


void Smb4KDeclarative::editBookmarks()
{
  Smb4KBookmarkHandler::self()->editBookmarks();
}


void Smb4KDeclarative::synchronize(Smb4KNetworkObject* object)
{
  if (object && object->type() == Smb4KNetworkObject::Share)
  {
    for (const SharePtr &share : mountedSharesList())
    {
      if (share->url() == object->url())
      {
        Smb4KSynchronizer::self()->synchronize(share);
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KDeclarative::openCustomOptionsDialog(Smb4KNetworkObject *object)
{
  if (object)
  {
    switch (object->type())
    {
      case Smb4KNetworkObject::Host:
      {
        for (const HostPtr &host : hostsList())
        {
          if (host->url() == object->url())
          {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(host);
            break;
          }
          else
          {
            continue;
          }
        }
        break;
      }
      case Smb4KNetworkObject::Share:
      {
        for (const SharePtr &share : sharesList())
        {
          if (share->url() == object->url())
          {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(share);
            break;
          }
          else
          {
            continue;
          }
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KDeclarative::startScanner()
{
  Smb4KScanner::self()->start();
}


void Smb4KDeclarative::abortScanner()
{
  Smb4KScanner::self()->abortAll();
}


void Smb4KDeclarative::startMounter()
{
  Smb4KMounter::self()->start();
}


void Smb4KDeclarative::abortMounter()
{
  Smb4KMounter::self()->abortAll();
}


void Smb4KDeclarative::startPrinter()
{
  Smb4KPrint::self()->start();
}


void Smb4KDeclarative::abortPrinter()
{
  Smb4KPrint::self()->abortAll();
}


QString Smb4KDeclarative::activeProfile() const
{
  QString activeProfile;
  
  for (Smb4KProfileObject *profile : d->profileObjects)
  {
    if (profile->isActiveProfile())
    {
      activeProfile = profile->profileName();
      break;
    }
    else
    {
      continue;
    }
  }
  
  return activeProfile;
}


void Smb4KDeclarative::setActiveProfile(const QString& profile)
{
  Smb4KProfileManager::self()->setActiveProfile(profile);
}


bool Smb4KDeclarative::profileUsage() const
{
  return Smb4KProfileManager::self()->useProfiles();
}


void Smb4KDeclarative::preview(Smb4KNetworkObject* object)
{
  if (object->type() == Smb4KNetworkObject::Share)
  {
    QString unc = object->url().toString(QUrl::RemoveScheme|QUrl::RemoveUserInfo|QUrl::RemovePort|QUrl::StripTrailingSlash);
    SharePtr share = findShare(unc, object->workgroupName());
    
    if (share)
    {
      Smb4KPreviewer::self()->preview(share);
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



void Smb4KDeclarative::slotWorkgroupsListChanged()
{
  // (Re)fill the list of workgroup objects.
  while (!d->workgroupObjects.isEmpty())
  {
    delete d->workgroupObjects.takeFirst();
  }

  for (const WorkgroupPtr &workgroup : workgroupsList())
  {
    d->workgroupObjects << new Smb4KNetworkObject(workgroup.data());
  }
  
  emit workgroupsListChanged();
}


void Smb4KDeclarative::slotHostsListChanged()
{
  // (Re)fill the list of host object.
  while (!d->hostObjects.isEmpty())
  {
    delete d->hostObjects.takeFirst();
  }
  
  for (const HostPtr &host : hostsList())
  {
    d->hostObjects << new Smb4KNetworkObject(host.data());
  }
  
  emit hostsListChanged();
}


void Smb4KDeclarative::slotSharesListChanged()
{
  // (Re)fill the list of share objects.
  while (!d->shareObjects.isEmpty())
  {
    delete d->shareObjects.takeFirst();
  }

  for (const SharePtr &share : sharesList())
  {
    d->shareObjects << new Smb4KNetworkObject(share.data());
  }
  
  emit sharesListChanged();
}


void Smb4KDeclarative::slotMountedSharesListChanged()
{
  // (Re)fill the list of share objects.
  while (!d->mountedObjects.isEmpty())
  {
    delete d->mountedObjects.takeFirst();
  }
  
  for (const SharePtr &mountedShare : mountedSharesList())
  {
    d->mountedObjects << new Smb4KNetworkObject(mountedShare.data());
  }
  
  emit mountedSharesListChanged();
}


void Smb4KDeclarative::slotBookmarksListChanged()
{
  // (Re)fill the list of bookmark and group objects.
  while (!d->bookmarkObjects.isEmpty())
  {
    delete d->bookmarkObjects.takeFirst();
  }
  
  while (!d->bookmarkGroupObjects.isEmpty())
  {
    delete d->bookmarkGroupObjects.takeFirst();
  }
  
  for (const BookmarkPtr &bookmark : Smb4KBookmarkHandler::self()->bookmarksList())
  {
    d->bookmarkObjects << new Smb4KBookmarkObject(bookmark.data());
  }
  
  for (const QString &group : Smb4KBookmarkHandler::self()->groupsList())
  {
    d->bookmarkGroupObjects << new Smb4KBookmarkObject(group);
  }
  
  emit bookmarksListChanged();
}


void Smb4KDeclarative::slotProfilesListChanged(const QStringList& profiles)
{
  while (!d->profileObjects.isEmpty())
  {
    delete d->profileObjects.takeFirst();
  }
  
  for (const QString &p : profiles)
  {
    Smb4KProfileObject *profile = new Smb4KProfileObject();
    profile->setProfileName(p);
    
    if (QString::compare(p, Smb4KProfileManager::self()->activeProfile()) == 0)
    {
      profile->setActiveProfile(true);
    }
    else
    {
      profile->setActiveProfile(false);
    }
    
    d->profileObjects << profile;
  }
  
  emit profilesListChanged();
}


void Smb4KDeclarative::slotActiveProfileChanged(const QString& activeProfile)
{
  for (Smb4KProfileObject *profile : d->profileObjects)
  {
    if (QString::compare(profile->profileName(), activeProfile) == 0)
    {
      profile->setActiveProfile(true);
    }
    else
    {
      profile->setActiveProfile(false);
    }
  }

  emit activeProfileChanged();
}


void Smb4KDeclarative::slotProfileUsageChanged(bool /*use*/)
{
  emit profileUsageChanged();
}

