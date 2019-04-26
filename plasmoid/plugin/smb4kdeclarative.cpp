/***************************************************************************
    This class provides the interface for Plasma and QtQuick
                             -------------------
    begin                : Mo 02 Sep 2013
    copyright            : (C) 2013-2019 by Alexander Reinholdt
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
#include "core/smb4kmounter.h"
#include "core/smb4kglobal.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kbasicnetworkitem.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4kprofilemanager.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kclient.h"

// Qt includes
#include <QDebug>
#include <QTest>

// KDE includes
#include <KCoreAddons/KPluginLoader>
#include <KCoreAddons/KPluginFactory>
#include <KConfigWidgets/KConfigDialog>


Smb4KDeclarative::Smb4KDeclarative(QObject* parent)
: QObject(parent), d(new Smb4KDeclarativePrivate)
{
  // 
  // Initialize the core
  // 
  Smb4KGlobal::initCore(true, false);
  
  // 
  // Connections
  // 
  connect(Smb4KClient::self(), SIGNAL(workgroups()), this, SLOT(slotWorkgroupsListChanged()));
  connect(Smb4KClient::self(), SIGNAL(hosts(WorkgroupPtr)), this, SLOT(slotHostsListChanged()));
  connect(Smb4KClient::self(), SIGNAL(shares(HostPtr)), this, SLOT(slotSharesListChanged()));
  connect(Smb4KClient::self(), SIGNAL(aboutToStart(NetworkItemPtr,int)), this, SIGNAL(busy()));
  connect(Smb4KClient::self(), SIGNAL(finished(NetworkItemPtr,int)), this, SIGNAL(idle()));
  
  connect(Smb4KMounter::self(), SIGNAL(mountedSharesListChanged()), this, SLOT(slotMountedSharesListChanged()));
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)), this, SIGNAL(busy()));
  connect(Smb4KMounter::self(), SIGNAL(finished(int)), this, SIGNAL(idle()));
  
  connect(Smb4KBookmarkHandler::self(), SIGNAL(updated()), this, SLOT(slotBookmarksListChanged()));
  
  connect(Smb4KProfileManager::self(), SIGNAL(profilesListChanged(QStringList)), this, SLOT(slotProfilesListChanged(QStringList)));
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), this, SLOT(slotActiveProfileChanged(QString)));
  connect(Smb4KProfileManager::self(), SIGNAL(profileUsageChanged(bool)), this, SLOT(slotProfileUsageChanged(bool)));
  
  // 
  // Do the initial loading of items
  // 
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
      case Smb4KNetworkObject::Network:
      {
        Smb4KClient::self()->lookupDomains();
        break;
      }
      case Smb4KNetworkObject::Workgroup:
      {
        // Check if the workgroup is known.
        WorkgroupPtr workgroup = Smb4KGlobal::findWorkgroup(object->url().host().toUpper());
        
        if (workgroup)
        {
          Smb4KClient::self()->lookupDomainMembers(workgroup);
        }
        
        break;
      }
      case Smb4KNetworkObject::Host:
      {
        // Check if the host is known.
        HostPtr host = Smb4KGlobal::findHost(object->url().host().toUpper());
        
        if (host)
        {
          Smb4KClient::self()->lookupShares(host);
        }
        
        break;
      }
      case Smb4KNetworkObject::Share:
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
    Smb4KClient::self()->lookupDomains();
  }
}


Smb4KNetworkObject *Smb4KDeclarative::findNetworkItem(const QUrl &url, int type)
{
  Smb4KNetworkObject *object = 0;
  
  if (url.isValid())
  {  
    switch (type)
    {
      case Smb4KNetworkObject::Workgroup:
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
      case Smb4KNetworkObject::Host:
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
      case Smb4KNetworkObject::Share:
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
  
  return object;
}


void Smb4KDeclarative::openMountDialog()
{
  Smb4KMounter::self()->openMountDialog();
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
      
    SharePtr share = Smb4KGlobal::findShare(object->url(), object->workgroupName());
    
    if (share)
    {
      Smb4KMounter::self()->mountShare(share);
    }
    else
    {
      // If the share is not in the global list of shares,
      // try the list of bookmarks.
      BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUrl(object->url());
      
      share = SharePtr(new Smb4KShare());
      share->setUrl(object->url());
      share->setWorkgroupName(bookmark->workgroupName());
      share->setHostIpAddress(bookmark->hostIpAddress());
      Smb4KMounter::self()->mountShare(share);
      
      while (Smb4KMounter::self()->isRunning())
      {
        QTest::qWait(50);
      }
      
      share.clear();      
    }
  }
}


void Smb4KDeclarative::unmount(Smb4KNetworkObject *object)
{
  if (object && object->type())
  {
    if (object->mountpoint().isValid())
    {
      SharePtr share = Smb4KGlobal::findShareByPath(object->mountpoint().path());
      
      if (share)
      {
        Smb4KMounter::self()->unmountShare(share);
      }
    }
  }
}


void Smb4KDeclarative::unmountAll()
{
  Smb4KMounter::self()->unmountAllShares(false);
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
  
  return object;
}


void Smb4KDeclarative::print(Smb4KNetworkObject* object)
{
  if (object && object->type() == Smb4KNetworkObject::Share)
  {
    SharePtr printer = Smb4KGlobal::findShare(object->url(), object->workgroupName());
    
    if (printer)
    {
      Smb4KClient::self()->openPrintDialog(printer);
    }
  }
}


void Smb4KDeclarative::addBookmark(Smb4KNetworkObject* object)
{
  if (object)
  {
    QList<SharePtr> shares; 
    
    // First, search the list of shares gathered by the scanner.
    for (const SharePtr &share : Smb4KGlobal::sharesList())
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
      for (const SharePtr &mountedShare : Smb4KGlobal::mountedSharesList())
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
    
    // Now add the share.
    if (!shares.isEmpty())
    {
      for (const SharePtr &p : shares)
      {
        qDebug() << p->url();
      }
      
      Smb4KBookmarkHandler::self()->addBookmarks(shares);
    }
  }
}


void Smb4KDeclarative::removeBookmark(Smb4KBookmarkObject* object)
{
  if (object)
  {
    // 
    // Find the bookmark in the list and remove it.
    // 
    BookmarkPtr bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUrl(object->url());
    
    if (bookmark)
    {
      Smb4KBookmarkHandler::self()->removeBookmark(bookmark);
    }
  }
}


void Smb4KDeclarative::editBookmarks()
{
  Smb4KBookmarkHandler::self()->editBookmarks();
}


void Smb4KDeclarative::synchronize(Smb4KNetworkObject* object)
{
  if (object && object->type() == Smb4KNetworkObject::Share)
  {
    for (const SharePtr &share : Smb4KGlobal::mountedSharesList())
    {
      if (share->url() == object->url())
      {
        Smb4KSynchronizer::self()->synchronize(share);
      }
    }
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
        for (const HostPtr &host : Smb4KGlobal::hostsList())
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
        for (const SharePtr &share : Smb4KGlobal::sharesList())
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
}


void Smb4KDeclarative::startClient()
{
  Smb4KClient::self()->start();
}


void Smb4KDeclarative::abortClient()
{
  Smb4KClient::self()->abort();
}


void Smb4KDeclarative::startMounter()
{
  Smb4KMounter::self()->start();
}


void Smb4KDeclarative::abortMounter()
{
  Smb4KMounter::self()->abort();
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
    SharePtr share = Smb4KGlobal::findShare(object->url(), object->workgroupName());
    
    if (share)
    {
      Smb4KClient::self()->openPreviewDialog(share);
    }
  }
}


void Smb4KDeclarative::openConfigurationDialog()
{
  //
  // Check if the configuration dialog exists and try to show it.
  //
  if (KConfigDialog::exists("Smb4KConfigDialog"))
  {
    KConfigDialog::showDialog("Smb4KConfigDialog");
    return;
  }
  
  //
  // If the dialog does not exist, load and show it:
  //
  KPluginLoader loader("smb4kconfigdialog", this);
  KPluginFactory *configFactory = loader.factory();

  if (configFactory)
  {
    KConfigDialog *dlg = configFactory->create<KConfigDialog>();
    
    if (dlg)
    {
      dlg->setObjectName("Smb4KConfigDialog");
      dlg->show();
    }
  }
}


void Smb4KDeclarative::slotWorkgroupsListChanged()
{
  // (Re)fill the list of workgroup objects.
  while (!d->workgroupObjects.isEmpty())
  {
    delete d->workgroupObjects.takeFirst();
  }

  for (const WorkgroupPtr &workgroup : Smb4KGlobal::workgroupsList())
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
  
  for (const HostPtr &host : Smb4KGlobal::hostsList())
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

  for (const SharePtr &share : Smb4KGlobal::sharesList())
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
  
  for (const SharePtr &mountedShare : Smb4KGlobal::mountedSharesList())
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

