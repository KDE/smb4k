/***************************************************************************
    smb4kbookmarkmenu  -  Bookmark menu
                             -------------------
    begin                : Sat Apr 02 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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
#include "smb4kbookmarkmenu.h"
#include "core/smb4kbookmark.h"
#include "core/smb4kshare.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDebug>

// KDE includes
#include <klocale.h>
#include <kaction.h>
#include <kicon.h>
#include <kmenu.h>

using namespace Smb4KGlobal;


Smb4KBookmarkMenu::Smb4KBookmarkMenu( int type, QWidget *parentWidget, QObject *parent )
: KActionMenu( KIcon( "folder-favorites" ), i18n( "Bookmarks" ), parent ), m_type( type ),
  m_parent_widget( parentWidget )
{
  // Set up action collection
  m_action_collection = new KActionCollection( this );

  // Set up action group for the bookmark groups
  m_groups = new QActionGroup( m_action_collection );

  // Set up action group for the bookmarks
  m_bookmarks = new QActionGroup( m_action_collection );

  // Set up the menu
  setupMenu();

  // Connections
  connect( m_action_collection, SIGNAL(actionTriggered(QAction*)), SLOT(slotActionTriggered(QAction*)) );
  connect( Smb4KBookmarkHandler::self(), SIGNAL(updated()), SLOT(slotBookmarksUpdated()) );
  connect( Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)), SLOT(slotDisableBookmark(Smb4KShare*)) );
  connect( Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)), SLOT(slotEnableBookmark(Smb4KShare*)) );
}


Smb4KBookmarkMenu::~Smb4KBookmarkMenu()
{
}


QAction *Smb4KBookmarkMenu::addBookmarkAction()
{
  return m_action_collection->action( "add_action" );
}


void Smb4KBookmarkMenu::refreshMenu()
{
  // Delete all entries
  while(!m_action_collection->actions().isEmpty())
  {
    QAction *action = m_action_collection->actions().first();
    m_action_collection->takeAction(action);
    removeAction(action);
    delete action;
  }
  
  // Set up the menu
  setupMenu();
}


void Smb4KBookmarkMenu::setupMenu()
{
  // Set up the actions for managing the bookmarks.
  switch ( m_type )
  {
    case MainWindow:
    {
      KAction *edit_action =  m_action_collection->addAction( "edit_action",
                              new KAction( KIcon( "bookmarks-organize" ), i18n( "&Edit Bookmarks" ), m_action_collection ) );
      KAction *add_action = m_action_collection->addAction( "add_action",
                            new KAction( KIcon( "bookmark-new" ), i18n( "Add &Bookmark" ), m_action_collection ) );
      add_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_B ) );
      addAction( edit_action );
      addAction( add_action );
      break;
    }
    case SystemTray:
    {
      KAction *edit_action =  m_action_collection->addAction( "edit_action",
                              new KAction( KIcon( "bookmarks-organize" ), i18n( "&Edit Bookmarks" ), m_action_collection ) );
      addAction( edit_action );
      break;
    }
    default:
    {
      break;
    }
  }

  // Get the groups
  QStringList groups = Smb4KBookmarkHandler::self()->groupsList();

  // Add a mount action if there were no groups defined and
  // there are bookmarks present (i.e. there is one empty group).
  if ( groups.size() == 0 || (groups.size() == 1 && groups.first().isEmpty()) )
  {
    KAction *mount_toplevel = new KAction( KIcon( "emblem-mounted" ), i18n( "Mount All Bookmarks" ), m_action_collection );
    m_action_collection->addAction( "mount_toplevel", mount_toplevel );
    addAction( mount_toplevel );
    
    QList<Smb4KBookmark *> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
    int number = 0;
    
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QList<Smb4KShare *> mounted = findShareByUNC( bookmarks.at( i )->unc() );
      
      if ( !mounted.isEmpty() )
      {
        for ( int j = 0; j < mounted.size(); ++j )
        {
          if ( !mounted.at( j )->isForeign() )
          {
            number++;
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
    }
    
    mount_toplevel->setEnabled( !(number == bookmarks.size()) );
  }
  else
  {
    // Do nothing
  }

  QAction *separator = insertSeparator( 0 );
  m_action_collection->addAction( "separator", separator );

  // Now add the groups and their bookmarks
  for ( int i = 0; i < groups.size(); ++i )
  {
    if ( !groups.at( i ).isEmpty() )
    {
      KAction *group = new KAction( KIcon( "folder-favorites" ), groups.at( i ), m_groups );
      m_action_collection->addAction( groups.at( i ), group );
      addAction( group );

      KActionMenu *group_menu = new KActionMenu( group );
      group->setMenu( group_menu->menu() );

      KAction *group_mount = new KAction( KIcon( "emblem-mounted" ), i18n( "Mount All Bookmarks" ), m_action_collection );
      m_action_collection->addAction( QString( "mount_%1" ).arg( groups.at( i ) ), group_mount );
      group_menu->addAction( group_mount );
      
      QList<Smb4KBookmark *> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList( groups.at( i ) );
      int number = 0;
    
      for ( int i = 0; i < bookmarks.size(); ++i )
      {
        QList<Smb4KShare *> mounted = findShareByUNC( bookmarks.at( i )->unc() );
      
        if ( !mounted.isEmpty() )
        {
          for ( int j = 0; j < mounted.size(); ++j )
          {
            if ( !mounted.at( j )->isForeign() )
            {
              number++;
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
      }
    
      group_mount->setEnabled( !(number == bookmarks.size()) );
      
      group_menu->addSeparator();

      QStringList sorted_bookmarks;
      
      for ( int j = 0; j < bookmarks.size(); ++j )
      {
        KAction *bookmark = NULL;
        QString item;

        if ( !bookmarks.at( j )->label().isEmpty() && Smb4KSettings::showCustomBookmarkLabel() )
        {
          bookmark = new KAction( KIcon( bookmarks.at( j )->icon() ), bookmarks.at( j )->label(), m_bookmarks );
          item = bookmarks.at( j )->label();
        }
        else
        {
          bookmark = new KAction( KIcon( bookmarks.at( j )->icon() ), bookmarks.at( j )->unc(), m_bookmarks );
          item = bookmarks.at( j )->unc();
        }

        QList<Smb4KShare *> mounted = findShareByUNC( bookmarks.at( j )->unc() );

        if ( !mounted.isEmpty() )
        {
          for ( int k = 0; k < mounted.size(); ++k )
          {
            if ( !mounted.at( k )->isForeign() )
            {
              bookmark->setEnabled( false );
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
          bookmark->setEnabled( true );
        }
        
        bookmark->setData( bookmarks.at( j )->url() );
        m_action_collection->addAction( QString( "[%1]_%2" ).arg( bookmarks.at( j )->groupName() ).arg( bookmarks.at( j )->unc() ), bookmark );
        sorted_bookmarks << item;
      }

      sorted_bookmarks.sort();

      for ( int j = 0; j < sorted_bookmarks.size(); ++j )
      {
        Smb4KBookmark *bookmark = Smb4KBookmarkHandler::self()->findBookmarkByLabel( sorted_bookmarks.at( j ) );

        if ( bookmark )
        {
          group_menu->addAction( m_action_collection->action( QString( "[%1]_%2" ).arg( bookmark->groupName() ).arg( bookmark->unc() ) ) );
        }
        else
        {
          bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC( sorted_bookmarks.at( j ) );

          if ( bookmark )
          {
            group_menu->addAction( m_action_collection->action( QString( "[%1]_%2" ).arg( bookmark->groupName() ).arg( bookmark->unc() ) ) );
          }
          else
          {
            // Do nothing
          }
        }
      }
    }
    else
    {
      // Do nothing
    }
  }

  // Now add all bookmarks that have no group.
  QList<Smb4KBookmark *> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList( "" );
  QStringList sorted_bookmarks;

  for ( int j = 0; j < bookmarks.size(); ++j )
  {
    KAction *bookmark = NULL;
    QString item;

    if ( !bookmarks.at( j )->label().isEmpty() && Smb4KSettings::showCustomBookmarkLabel() )
    {
      bookmark = new KAction( KIcon( bookmarks.at( j )->icon() ), bookmarks.at( j )->label(), m_bookmarks );
      item = bookmarks.at( j )->label();
    }
    else
    {
      bookmark = new KAction( KIcon( bookmarks.at( j )->icon() ), bookmarks.at( j )->unc(), m_bookmarks );
      item = bookmarks.at( j )->unc();
    }

    QList<Smb4KShare *> mounted = findShareByUNC( bookmarks.at( j )->unc() );

    if ( !mounted.isEmpty() )
    {
      for ( int k = 0; k < mounted.size(); ++k )
      {
        if ( !mounted.at( k )->isForeign() )
        {
          bookmark->setEnabled( false );
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
      bookmark->setEnabled( true );
    }

    bookmark->setData( bookmarks.at( j )->url() );
    m_action_collection->addAction( QString( "[%1]_%2" ).arg( bookmarks.at( j )->groupName() ).arg( bookmarks.at( j )->unc() ), bookmark );
    sorted_bookmarks << item;
  }

  sorted_bookmarks.sort();

  for ( int j = 0; j < sorted_bookmarks.size(); ++j )
  {
    Smb4KBookmark *bookmark = Smb4KBookmarkHandler::self()->findBookmarkByLabel( sorted_bookmarks.at( j ) );

    if ( bookmark )
    {
      addAction( m_action_collection->action( QString( "[%1]_%2" ).arg( bookmark->groupName() ).arg( bookmark->unc() ) ) );
    }
    else
    {
      bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC( sorted_bookmarks.at( j ) );

      if ( bookmark )
      {
        addAction( m_action_collection->action( QString( "[%1]_%2" ).arg( bookmark->groupName() ).arg( bookmark->unc() ) ) );
      }
      else
      {
        // Do nothing
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KBookmarkMenu::slotActionTriggered( QAction *action )
{
  if ( QString::compare( "edit_action", action->objectName() ) == 0 )
  {
    // Edit the bookmarks
    Smb4KBookmarkHandler::self()->editBookmarks( m_parent_widget );
  }
  else if ( QString::compare( "add_action", action->objectName() ) == 0 )
  {
    // Use addBookmarkAction() and connect the triggered() signal
    // in your application.
  }
  else if ( QString::compare( "mount_toplevel", action->objectName() ) == 0 )
  {
    // Mount all shares belonging to the top level bookmarks. This will only be 
    // called if there are not groups defined.
    QList<Smb4KBookmark *> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList();
    QList<Smb4KShare *> mounts;

    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      // FIXME: Check if the bookmarked share has already been mounted.
      Smb4KShare *share = new Smb4KShare( bookmarks.at( i )->hostName(), bookmarks.at( i )->shareName() );
      share->setWorkgroupName( bookmarks.at( i )->workgroupName() );
      share->setHostIP( bookmarks.at( i )->hostIP() );
      share->setLogin( bookmarks.at( i )->login() );
      mounts << share;
    }

    Smb4KMounter::self()->mountShares( mounts );

    while ( !mounts.isEmpty() )
    {
      delete mounts.takeFirst();
    }
  }
  else if ( action->objectName().startsWith( QLatin1String( "mount_" ) ) && QString::compare( "mount_toplevel", action->objectName() ) != 0 )
  {
    // Mount all bookmarked share that belong to this group.
    QString group_name = action->objectName().section( '_', 1, -1 ).trimmed();
    QList<Smb4KBookmark *> bookmarks = Smb4KBookmarkHandler::self()->bookmarksList( group_name );
    QList<Smb4KShare *> mounts;

    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      Smb4KShare *share = new Smb4KShare( bookmarks.at( i )->hostName(), bookmarks.at( i )->shareName() );
      share->setWorkgroupName( bookmarks.at( i )->workgroupName() );
      share->setHostIP( bookmarks.at( i )->hostIP() );
      share->setLogin( bookmarks.at( i )->login() );
      mounts << share;
    }

    Smb4KMounter::self()->mountShares( mounts );

    while ( !mounts.isEmpty() )
    {
      delete mounts.takeFirst();
    }
  }
  else if ( action->objectName().startsWith( '[' ) && action->objectName().contains( "]_//" ) )
  {
    // Mount a single bookmarked share.
    QString group_name = action->objectName().section( '[', 1, 1 ).section( "]_", 0, 0 ).trimmed();
    QString unc = action->objectName().section( "]_", 1, -1 ).trimmed();

    Smb4KBookmark *bookmark = Smb4KBookmarkHandler::self()->findBookmarkByUNC( unc );

    if ( bookmark && QString::compare( group_name, bookmark->groupName() ) == 0 )
    {
      Smb4KShare *share = new Smb4KShare( bookmark->hostName(), bookmark->shareName() );
      share->setWorkgroupName( bookmark->workgroupName() );
      share->setHostIP( bookmark->hostIP() );
      share->setLogin( bookmark->login() );
      Smb4KMounter::self()->mountShare( share );
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


void Smb4KBookmarkMenu::slotBookmarksUpdated()
{
  refreshMenu();
}


void Smb4KBookmarkMenu::slotDisableBookmark( Smb4KShare *share )
{
  if ( !share->isForeign() && !m_bookmarks->actions().isEmpty() )
  {
    QList<QAction *> bookmarks = m_bookmarks->actions();
    QString group_name;
    
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QAction *bookmark = bookmarks.at( i );
      
      if ( bookmark->isEnabled() && bookmark->objectName().startsWith( '[' ) && bookmark->objectName().contains( "]_//" ) )
      {
        QString unc = bookmark->objectName().section( "]_", 1, -1 ).trimmed();
        
        if ( QString::compare( unc, share->unc(), Qt::CaseInsensitive ) == 0 )
        {
          bookmark->setEnabled( !share->isMounted() );
          group_name = bookmark->objectName().section( '[', 1, -1 ).section( "]_", 0, 0 ).trimmed();
          break;
        }
        else
        {
          continue;
        }
      }
      else
      {
        continue;
      }
    }
    
    bool all_mounted = true;
    
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QAction *bookmark = bookmarks.at( i );
      
      if ( bookmark->objectName().startsWith( QString( "[%1]" ).arg( group_name ) ) && bookmark->isEnabled() )
      {
        all_mounted = false;
        break;
      }
      else
      {
        continue;
      }
    }
    
    if ( all_mounted )
    {
      if ( group_name.isEmpty() )
      {
        QAction *action = m_action_collection->action( "mount_toplevel" );

        if ( action )
        {
          action->setEnabled( false );
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        QAction *action = m_action_collection->action( QString( "mount_%1" ).arg( group_name ) );

        if ( action )
        {
          action->setEnabled( false );
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
  else
  {
    // Do nothing
  }
}


void Smb4KBookmarkMenu::slotEnableBookmark( Smb4KShare *share )
{
  if ( !share->isForeign() && !m_bookmarks->actions().isEmpty() )
  {
    QList<QAction *> bookmarks = m_bookmarks->actions();
    QString group_name;
    
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QAction *bookmark = bookmarks.at( i );
      
      if ( !bookmark->isEnabled() && bookmark->objectName().startsWith( '[' ) && bookmark->objectName().contains( "]_//" ) )
      {
        QString unc = bookmark->objectName().section( "]_", 1, -1 ).trimmed();
        
        if ( QString::compare( unc, share->unc(), Qt::CaseInsensitive ) == 0 )
        {
          bookmark->setEnabled( !share->isMounted() );
          group_name = bookmark->objectName().section( '[', 1, -1 ).section( "]_", 0, 0 ).trimmed();
          break;
        }
        else
        {
          continue;
        }
      }
      else
      {
        continue;
      }
    }
    
    bool all_mounted = true;
    
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QAction *bookmark = bookmarks.at( i );
      
      if ( bookmark->objectName().startsWith( QString( "[%1]" ).arg( group_name ) ) && bookmark->isEnabled() )
      {
        all_mounted = false;
        break;
      }
      else
      {
        continue;
      }
    }
    
    if ( !all_mounted )
    {
      if ( group_name.isEmpty() )
      {
        QAction *action = m_action_collection->action( "mount_toplevel" );

        if ( action )
        {
          action->setEnabled( true );
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        QAction *action = m_action_collection->action( QString( "mount_%1" ).arg( group_name ) );

        if ( action )
        {
          action->setEnabled( true );
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
  else
  {
    // Do nothing
  }
}


#include "smb4kbookmarkmenu.moc"

