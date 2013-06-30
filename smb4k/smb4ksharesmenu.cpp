/***************************************************************************
    smb4ksharesmenu  -  Shares menu
                             -------------------
    begin                : Mon Sep 05 2011
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
#include "smb4ksharesmenu.h"
#include "core/smb4kshare.h"
#include "core/smb4kmounter.h"
#include "core/smb4kglobal.h"
#include "core/smb4ksettings.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kbookmarkhandler.h"

// Qt includes
#include <QMap>
#include <QVariant>
#include <QString>
#include <QStringList>

// KDE includes
#include <klocale.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kstandarddirs.h>

using namespace Smb4KGlobal;


Smb4KSharesMenu::Smb4KSharesMenu( QWidget *parentWidget, QObject *parent )
: KActionMenu( KIcon( "folder-remote", KIconLoader::global(), QStringList( "emblem-mounted" ) ), i18n( "Mounted Shares" ), parent ),
  m_parent_widget( parentWidget )
{
  // Set up action collection
  m_action_collection = new KActionCollection( this );

  // Set up action group for the shares menus
  m_menus = new QActionGroup( m_action_collection );

  // Set up action group for the shares actions
  m_actions = new QActionGroup( m_action_collection );

  // Setup the menu
  setupMenu();

  connect( m_actions, SIGNAL(triggered(QAction*)), SLOT(slotShareAction(QAction*)) );
  connect( Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)), SLOT(slotShareMounted(Smb4KShare*)) );
  connect( Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)), SLOT(slotShareUnmounted(Smb4KShare*)) );
}


Smb4KSharesMenu::~Smb4KSharesMenu()
{
}


void Smb4KSharesMenu::refreshMenu()
{
  m_action_collection->action( "unmount_all" )->setEnabled(
    ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()) );

  if ( !Smb4KSettings::showMountPoint() )
  {
    for ( int i = 0; i < m_menus->actions().size(); ++i )
    {
      QString text = m_menus->actions()[i]->data().toMap().value( "unc" ).toString();
      m_menus->actions()[i]->setText( text );
    }
  }
  else
  {
    for ( int i = 0; i < m_menus->actions().size(); ++i )
    {
      QString text = m_menus->actions()[i]->data().toMap().value( "mountpoint" ).toString();
      m_menus->actions()[i]->setText( text );
    }
  }
}


void Smb4KSharesMenu::setupMenu()
{
  // Unmount All action
  KAction *unmount_all = new KAction( KIcon( "system-run" ), i18n( "U&nmount All" ), m_action_collection );
  unmount_all->setEnabled( false );
  m_action_collection->addAction( "unmount_all", unmount_all );

  connect( unmount_all, SIGNAL(triggered(bool)), SLOT(slotUnmountAllShares()) );

  addAction( unmount_all );

  // Separator
  addSeparator();

  // Shares
  for ( int i = 0; i < mountedSharesList().size(); ++i )
  {
    slotShareMounted( mountedSharesList().at( i ) );
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesMenu::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Create the share menu.
  KActionMenu *share_menu = new KActionMenu(
    (Smb4KSettings::showMountPoint() ? share->canonicalPath() : share->unc()),
    m_menus );
  share_menu->setIcon( share->icon() );
  share_menu->setObjectName( share->canonicalPath() );

  QMap<QString,QVariant> data;
  data["unc"] = share->unc();
  data["mountpoint"] = share->path();
  data["foreign"] = share->isForeign();
  share_menu->setData( data );

  // Put the share at the right position in the menu. For
  // this we get the object names of the share menus and
  // sort them.

  // Get the list of share menus.
  QList<QAction *> share_menus = m_menus->actions();

  // Insert the new action menu
  if ( share_menus.size() != 1 )
  {
    // NOTE: We do not need to add the new share to the QActionGroup
    // here, because this was already done by the creation of the
    // menu above.
    QStringList names;

    for ( int i = 0; i < share_menus.size(); ++i )
    {
      QMap<QString,QVariant> share_data = share_menus.at( i )->data().toMap();

      if ( !Smb4KSettings::showMountPoint() )
      {
        names << share_data.value( "unc" ).toString();
      }
      else
      {
        names << share_data.value( "mountpoint" ).toString();
      }
    }

    names.sort();

    QString name;
    int index = names.indexOf( (!Smb4KSettings::showMountPoint() ? share->unc() : share->path()) );

    if ( index < names.size() - 1 )
    {
      index++;
      name = names.at( index );
    }
    else
    {
      // Do nothing
    }

    if ( !name.isEmpty() )
    {
      for ( int i = 0; i < share_menus.size(); ++i )
      {
        if ( QString::localeAwareCompare( name, share_menus.at( i )->data().toMap().value( "unc" ).toString() ) == 0 ||
             QString::localeAwareCompare( name, share_menus.at( i )->data().toMap().value( "mountpoint" ).toString() ) == 0 )
        {
          insertAction( share_menus.at( i ), share_menu );
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
      insertAction( 0, share_menu );
    }
  }
  else
  {
    insertAction( 0, share_menu );
  }

  m_action_collection->addAction( share_menu->objectName(), share_menu );

  // Now add the actions for this share.
  KAction *unmount = new KAction( KIcon( "enblem-unmounted" ), i18n( "Unmount" ), m_actions );
  unmount->setObjectName( QString( "[unmount]_%1" ).arg( share->canonicalPath() ) );
  unmount->setEnabled( !share->isForeign() || Smb4KSettings::unmountForeignShares() );
  share_menu->addAction( unmount );
  m_action_collection->addAction( unmount->objectName(), unmount );

  share_menu->addSeparator();

  KAction *add_bookmark = new KAction( KIcon( "bookmark-new" ), i18n( "Add Bookmark" ), m_actions );
  add_bookmark->setObjectName( QString( "[bookmark]_%1" ).arg( share->canonicalPath() ) );
  share_menu->addAction( add_bookmark );
  m_action_collection->addAction( add_bookmark->objectName(), add_bookmark );

  KAction *synchronize = new KAction( KIcon( "folder-sync" ), i18n( "Synchronize" ), m_actions );
  synchronize->setObjectName( QString( "[synchronize]_%1" ).arg( share->canonicalPath() ) );
  synchronize->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() && !share->isInaccessible() );
  share_menu->addAction( synchronize );
  m_action_collection->addAction( synchronize->objectName(), synchronize );

  share_menu->addSeparator();

  KAction *konsole = new KAction( KIcon( "utilities-terminal" ), i18n( "Open with Konsole" ), m_actions );
  konsole->setObjectName( QString( "[konsole]_%1" ).arg( share->canonicalPath() ) );
  konsole->setEnabled( !KStandardDirs::findExe( "konsole" ).isEmpty() && !share->isInaccessible() );
  share_menu->addAction( konsole );
  m_action_collection->addAction( konsole->objectName(), konsole );

  KAction *filemanager = new KAction( KIcon( "system-file-manager" ), i18n( "Open with File Manager" ), m_actions );
  filemanager->setObjectName( QString( "[filemanager]_%1" ).arg( share->canonicalPath() ) );
  filemanager->setEnabled( !share->isInaccessible() );
  share_menu->addAction( filemanager );
  m_action_collection->addAction( filemanager->objectName(), filemanager );

  m_action_collection->action( "unmount_all" )->setEnabled(
    ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()) );
}


void Smb4KSharesMenu::slotShareUnmounted( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Get the shares action menu
  KActionMenu *share_menu = static_cast<KActionMenu *>( m_action_collection->action( share->canonicalPath() ) );

  if ( share_menu )
  {
    // Remove all actions belonging to this share
    QList<QAction *> actions = m_actions->actions();
    QMutableListIterator<QAction *> a( actions );

    while ( a.hasNext() )
    {
      QAction *action = a.next();

      if ( action->objectName().endsWith( QString( "]_%1" ).arg( share->canonicalPath() ) ) )
      {
        share_menu->removeAction( action );
        m_actions->removeAction( action );
        a.remove();

        if ( action )
        {
          delete action;
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

    // Remove the share's action menu
    removeAction( share_menu );
    m_menus->removeAction( share_menu );

    if ( share_menu )
    {
      delete share_menu;
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

  m_action_collection->action( "unmount_all" )->setEnabled(
    ((!onlyForeignMountedShares() || Smb4KSettings::unmountForeignShares()) && !m_menus->actions().isEmpty()) );
}


void Smb4KSharesMenu::slotUnmountAllShares()
{
  Smb4KMounter::self()->unmountAllShares( m_parent_widget );
}


void Smb4KSharesMenu::slotShareAction( QAction *action )
{
  Smb4KShare *share = NULL;

  if ( action->objectName().contains( "]_" ) )
  {
    QString path = action->objectName().section( "]_", 1, -1 ).trimmed();
    share = findShareByPath( path );
  }
  else
  {
    // Do nothing
  }

  if ( share )
  {
    if ( action->objectName().startsWith( QLatin1String( "[unmount]" ) ) )
    {
      Smb4KMounter::self()->unmountShare( share, false, m_parent_widget );
    }
    else if ( action->objectName().startsWith( QLatin1String( "[bookmark]" ) ) )
    {
      Smb4KBookmarkHandler::self()->addBookmark( share, m_parent_widget );
    }
    else if ( action->objectName().startsWith( QLatin1String( "[synchronize]" ) ) )
    {
      Smb4KSynchronizer::self()->synchronize( share, m_parent_widget );
    }
    else if ( action->objectName().startsWith( QLatin1String( "[konsole]" ) ) )
    {
      open( share, Smb4KGlobal::Konsole );
    }
    else if ( action->objectName().startsWith( QLatin1String( "[filemanager]" ) ) )
    {
      open( share, Smb4KGlobal::FileManager );
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

#include "smb4ksharesmenu.moc"
