/***************************************************************************
    smb4kdeclarative - This class provides the interface for Plasma and
    QtQuick
                             -------------------
    begin                : Mo 02 Sep 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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
#include "smb4kscanner.h"
#include "smb4kglobal.h"
#include "smb4kworkgroup.h"
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kbasicnetworkitem.h"
#include "smb4kbookmarkhandler.h"
#include "smb4kbookmark.h"
#include "smb4kprint.h"
#include "smb4kbookmarkobject.h"
#include "smb4kcustomoptionsmanager.h"

using namespace Smb4KGlobal;


Smb4KDeclarative::Smb4KDeclarative(QObject* parent)
: QObject(parent), d( new Smb4KDeclarativePrivate )
{
  connect( Smb4KScanner::self(), SIGNAL(workgroups(QList<Smb4KWorkgroup*>)), this, SLOT(slotWorkgroupsListChanged()) );
  connect( Smb4KScanner::self(), SIGNAL(hosts(Smb4KWorkgroup*,QList<Smb4KHost*>)), this, SLOT(slotHostsListChanged()) );
  connect( Smb4KScanner::self(), SIGNAL(shares(Smb4KHost*,QList<Smb4KShare*>)), this, SLOT(slotSharesListChanged()) );
  connect( Smb4KScanner::self(), SIGNAL(aboutToStart(Smb4KBasicNetworkItem*,int)), this, SIGNAL(busy()) );
  connect( Smb4KScanner::self(), SIGNAL(finished(Smb4KBasicNetworkItem*,int)), this, SIGNAL(idle()) );
  
  connect( Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)), this, SLOT(slotMountedSharesListChanged()) );
  connect( Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)), this, SLOT(slotMountedSharesListChanged()) );
  connect( Smb4KMounter::self(), SIGNAL(aboutToStart(Smb4KShare*,int)), this, SIGNAL(busy()) );
  connect( Smb4KMounter::self(), SIGNAL(finished(Smb4KShare*,int)), this, SIGNAL(idle()) );
  
  connect( Smb4KPrint::self(), SIGNAL(aboutToStart(Smb4KShare*)), this, SIGNAL(busy()) );
  connect( Smb4KPrint::self(), SIGNAL(finished(Smb4KShare*)), this, SIGNAL(idle()) );
  
  connect( Smb4KBookmarkHandler::self(), SIGNAL(updated()), this, SLOT(slotBookmarksListChanged()) );
  
  // Do the initial loading of items.
  slotBookmarksListChanged();
}


Smb4KDeclarative::~Smb4KDeclarative()
{
  while ( !d->workgroupObjects.isEmpty() )
  {
    delete d->workgroupObjects.takeFirst();
  }

  while ( !d->hostObjects.isEmpty() )
  {
    delete d->hostObjects.takeFirst();
  }

  while ( !d->shareObjects.isEmpty() )
  {
    delete d->shareObjects.takeFirst();
  }
  
  while ( !d->mountedObjects.isEmpty() )
  {
    delete d->mountedObjects.takeFirst();
  }
  
  while ( !d->bookmarkObjects.isEmpty() )
  {
    delete d->bookmarkObjects.takeFirst();
  }
  
  while ( !d->bookmarkGroupObjects.isEmpty() )
  {
    delete d->bookmarkGroupObjects.takeFirst();
  }
}


QDeclarativeListProperty<Smb4KNetworkObject> Smb4KDeclarative::workgroups()
{
  return QDeclarativeListProperty<Smb4KNetworkObject>( this, d->workgroupObjects );
}


QDeclarativeListProperty<Smb4KNetworkObject> Smb4KDeclarative::hosts()
{
  return QDeclarativeListProperty<Smb4KNetworkObject>( this, d->hostObjects );
}


QDeclarativeListProperty<Smb4KNetworkObject> Smb4KDeclarative::shares()
{
  return QDeclarativeListProperty<Smb4KNetworkObject>( this, d->shareObjects );
}


QDeclarativeListProperty<Smb4KNetworkObject> Smb4KDeclarative::mountedShares()
{
  return QDeclarativeListProperty<Smb4KNetworkObject>( this, d->mountedObjects );
}


QDeclarativeListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarks()
{
  return QDeclarativeListProperty<Smb4KBookmarkObject>( this, d->bookmarkObjects );
}


QDeclarativeListProperty<Smb4KBookmarkObject> Smb4KDeclarative::bookmarkGroups()
{
  return QDeclarativeListProperty<Smb4KBookmarkObject>( this, d->bookmarkGroupObjects );
}



void Smb4KDeclarative::lookup( Smb4KNetworkObject *object )
{
  if ( object )
  {
    switch ( object->type() )
    {
      case Network:
      {
        Smb4KScanner::self()->lookupDomains();
        break;
      }
      case Workgroup:
      {
        // Check if the workgroup is known.
        Smb4KWorkgroup *workgroup = findWorkgroup( object->url().host().toUpper() );
        
        if ( workgroup )
        {
          Smb4KScanner::self()->lookupDomainMembers( workgroup );
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
        Smb4KHost *host = findHost( object->url().host().toUpper() );
        
        if ( host )
        {
          Smb4KScanner::self()->lookupShares( host );
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
    // If the object is NULL, scan the whole network.
    Smb4KScanner::self()->lookupDomains();
  }
}


Smb4KNetworkObject *Smb4KDeclarative::findNetworkItem( const QUrl &url, int type )
{
  Smb4KNetworkObject *object = NULL;
  
  if ( url.isValid() )
  {  
    switch ( type )
    {
      case Workgroup:
      {
        for ( int i = 0; i < d->workgroupObjects.size(); ++i )
        {
          if ( url == d->workgroupObjects.at( i )->url() )
          {
            object = d->workgroupObjects[i];
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
        for ( int i = 0; i < d->hostObjects.size(); ++i )
        {
          if ( url == d->hostObjects.at( i )->url() )
          {
            object = d->hostObjects[i];
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
        for ( int i = 0; i < d->shareObjects.size(); ++i )
        {
          if ( url == d->shareObjects.at( i )->url() )
          {
            object = d->shareObjects[i];
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


void Smb4KDeclarative::mount(const QUrl& url)
{
  //
  // FIXME - Smb4K 1.2: Use Smb4KNetworkObject as argument
  // 
  if ( url.isValid() && !url.path().isEmpty() )
  {
    QString share_name = url.path();
      
    if ( share_name.startsWith( '/' ) )
    {
      share_name = share_name.mid( 1, -1 );
    }
    else
    {
      // Do nothing
    }
      
    Smb4KShare *share = findShare( share_name, url.host() );
    
    if ( share )
    {
      Smb4KMounter::self()->mountShare( share );
    }
    else
    {
      // If the share is not in the global list of shares,
      // try the list of bookmarks.
      QString unc( "//"+url.host()+"/"+share_name );
      Smb4KBookmark *bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC( unc );
      share = new Smb4KShare();
      share->setURL( url );
      share->setWorkgroupName( bookmark->workgroupName() );
      share->setHostIP( bookmark->hostIP() );
      Smb4KMounter::self()->mountShare( share );
      delete share;
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KDeclarative::unmount(const QUrl& mountpoint)
{
  //
  // FIXME - Smb4K 1.2: Use Smb4KNetworkObject as argument
  // 
  if ( mountpoint.isValid() )
  {
    Smb4KShare *share = findShareByPath( mountpoint.path() );
    
    if ( share )
    {
      Smb4KMounter::self()->unmountShare( share );
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


void Smb4KDeclarative::unmountAll()
{
  Smb4KMounter::self()->unmountAllShares();
}


Smb4KNetworkObject* Smb4KDeclarative::findMountedShare(const QUrl& url, bool exactMatch)
{
  Smb4KNetworkObject *object = NULL;
  
  if ( url.isValid() )
  {
    KUrl u1 = url;
    u1.setUserInfo( QString() );
    u1.setPort( -1 );
    
    for ( int i = 0; i < d->mountedObjects.size(); ++i )
    {
      KUrl u2 =  d->mountedObjects.at( i )->url();
      u2.setUserInfo( QString() );
      u2.setPort( -1 );
      
      if ( url == d->mountedObjects.at( i )->url() )
      {
        object = d->mountedObjects[i];
        break;
      }
      else if ( u1 == u2 && !exactMatch )
      {
        object = d->mountedObjects[i];
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
  if ( object )
  {
    QString host = object->url().host();
    QString name = object->url().path();
    
    if ( name.startsWith( '/' ) )
    {
      name = name.mid( 1 );
    }
    else
    {
      // Do nothing
    }
    
    Smb4KShare *printer = findShare( name, host );
    
    if ( printer )
    {
      Smb4KPrint::self()->print( printer, 0 );
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
  if ( object )
  {
    QList<Smb4KShare *> shares; 
    
    // First, search the list of shares gathered by 
    // the scanner.
    for ( int i = 0; i < sharesList().size(); ++i )
    {
      if ( sharesList().at( i )->url() == object->url() )
      {
        shares << sharesList().at( i );
        break;
      }
      else
      {
        continue;
      }
    }
    
    // Second, if the list is still empty, try the list 
    // of mounted shares.
    if ( shares.isEmpty() )
    {
      for ( int i = 0; i < mountedSharesList().size(); ++i )
      {
        if ( mountedSharesList().at( i )->url() == object->url() )
        {
          shares << mountedSharesList().at( i );
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
    if ( !shares.isEmpty() )
    {
      Smb4KBookmarkHandler::self()->addBookmarks( shares, 0 );
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
  if ( object )
  {
    // Find the bookmark in the list and remove it.
    QString path = (object->url().path().startsWith( '/' ) ? object->url().path().remove( 0, 1 ) : object->url().path());
    QString unc = QString( "//%1/%2" ).arg( object->url().host().toUpper() ).arg( path );
    Smb4KBookmark *bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC( unc );
    
    if ( bookmark )
    {
      Smb4KBookmarkHandler::self()->removeBookmark( bookmark );
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


void Smb4KDeclarative::openCustomOptionsDialog(Smb4KNetworkObject *object)
{
  if ( object )
  {
    switch ( object->type() )
    {
      case Host:
      {
        for ( int i = 0; i < hostsList().size(); ++i )
        {
          if ( hostsList().at( i )->url() == object->url() )
          {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog( hostsList().at( i ) );
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
        for ( int i = 0; i < sharesList().size(); ++i )
        {
          if ( sharesList().at( i )->url() == object->url() )
          {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog( sharesList().at( i ) );
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


void Smb4KDeclarative::slotWorkgroupsListChanged()
{
  // (Re)fill the list of workgroup objects.
  while ( !d->workgroupObjects.isEmpty() )
  {
    delete d->workgroupObjects.takeFirst();
  }

  for ( int i = 0; i < workgroupsList().size(); ++i )
  {
    d->workgroupObjects << new Smb4KNetworkObject( workgroupsList().at( i ) );
  }
  
  emit workgroupsListChanged();
}


void Smb4KDeclarative::slotHostsListChanged()
{
  // (Re)fill the list of host object.
  while ( !d->hostObjects.isEmpty() )
  {
    delete d->hostObjects.takeFirst();
  }

  for ( int i = 0; i < hostsList().size(); ++i )
  {
    d->hostObjects << new Smb4KNetworkObject( hostsList().at( i ) );
  } 
  
  emit hostsListChanged();
}


void Smb4KDeclarative::slotSharesListChanged()
{
  // (Re)fill the list of share object.
  while ( !d->shareObjects.isEmpty() )
  {
    delete d->shareObjects.takeFirst();
  }

  for ( int i = 0; i < sharesList().size(); ++i )
  {
    d->shareObjects << new Smb4KNetworkObject( sharesList().at( i ) );
  }
  
  emit sharesListChanged();
}


void Smb4KDeclarative::slotMountedSharesListChanged()
{
  // (Re)fill the list of share objects.
  while ( !d->mountedObjects.isEmpty() )
  {
    delete d->mountedObjects.takeFirst();
  }

  for ( int i = 0; i < mountedSharesList().size(); ++i )
  {
    d->mountedObjects << new Smb4KNetworkObject( mountedSharesList().at( i ) );
  }
  
  emit mountedSharesListChanged();
}


void Smb4KDeclarative::slotBookmarksListChanged()
{
  // (Re)fill the list of bookmark and group objects.
  while ( !d->bookmarkObjects.isEmpty() )
  {
    delete d->bookmarkObjects.takeFirst();
  }
  
  while ( !d->bookmarkGroupObjects.isEmpty() )
  {
    delete d->bookmarkGroupObjects.takeFirst();
  }
  
  for ( int i = 0; i < Smb4KBookmarkHandler::self()->bookmarksList().size(); ++i )
  {
    d->bookmarkObjects << new Smb4KBookmarkObject( Smb4KBookmarkHandler::self()->bookmarksList().at( i ) );
  }
  
  for ( int i = 0; i < Smb4KBookmarkHandler::self()->groupsList().size(); ++i )
  {
    d->bookmarkGroupObjects << new Smb4KBookmarkObject( Smb4KBookmarkHandler::self()->groupsList().at( i ) );
  }
  
  emit bookmarksListChanged();
}


#include "smb4kdeclarative.moc"
