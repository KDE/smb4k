/***************************************************************************
    smb4ksystemtray  -  This is the system tray window class of Smb4K.
                             -------------------
    begin                : Mi Jun 13 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

// Qt includes
#include <QMenu>

// KDE specific includes
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kconfigdialog.h>
#include <kiconeffect.h>
#include <kicon.h>
#include <kstandardaction.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>

// application specific includes
#include <smb4ksystemtray.h>
#include <smb4kbookmarkmenu.h>
#include <dialogs/smb4kmountdialog.h>
#include <core/smb4kcore.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kshare.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>
#include <core/smb4kbookmarkhandler.h>
#include <core/smb4kmounter.h>
#include <core/smb4kscanner.h>
#include <core/smb4ksynchronizer.h>

using namespace Smb4KGlobal;


Smb4KSystemTray::Smb4KSystemTray( QWidget *parent )
: KStatusNotifierItem( "smb4k_systemtray", parent )
{
  setIconByName( "smb4k" );
  setToolTip( KIconLoader::global()->loadIcon( "smb4k", KIconLoader::NoGroup ), i18n( "Smb4K" ), i18n( "Advanced Network Neighborhood Browser" ) );
  
  // Show the icon to the user. It will become passive, if the scanner
  // could not find something and no shares were mounted.
  setStatus( KStatusNotifierItem::Active );

  m_share_menus    = new QActionGroup( actionCollection() );
  m_shares_actions = new QActionGroup( actionCollection() );

  // Set up the context menu (skeleton):
  QStringList shares_overlay;
  shares_overlay.append( "emblem-mounted" );

  m_shares_menu         = new KActionMenu( KIcon( "folder-remote", KIconLoader::global(), shares_overlay ),
                          i18n( "Mounted Shares" ), actionCollection() );
  KAction *manual_mount = new KAction( KIcon( "list-add" ), i18n( "M&ount Manually" ),
                          actionCollection() );
  KAction *configure    = KStandardAction::preferences( this, SLOT( slotConfigDialog() ),
                          actionCollection() );
  Smb4KBookmarkMenu *bookmark_menu = new Smb4KBookmarkMenu( Smb4KBookmarkMenu::SystemTray, associatedWidget(), this );

  contextMenu()->addAction( m_shares_menu );
  contextMenu()->addAction( bookmark_menu );
  contextMenu()->addSeparator();
  contextMenu()->addAction( manual_mount );
  contextMenu()->addAction( configure );

  // Set up the menus:
  setupSharesMenu();

  // Connections:
  connect( manual_mount,                 SIGNAL( triggered( bool ) ),
           this,                         SLOT( slotMountDialog( bool ) ) );

  connect( m_shares_actions,             SIGNAL( triggered( QAction * ) ),
           this,                         SLOT( slotShareActionTriggered( QAction * ) ) );

  connect( Smb4KBookmarkHandler::self(), SIGNAL( updated() ),
           this,                         SLOT( slotSetupBookmarksMenu() ) );
           
  connect( Smb4KMounter::self(),         SIGNAL( mounted( Smb4KShare * ) ),
           this,                         SLOT( slotEnableBookmarks( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(),         SIGNAL( unmounted( Smb4KShare * ) ),
           this,                         SLOT( slotEnableBookmarks( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(),         SIGNAL( mounted( Smb4KShare * ) ),
           this,                         SLOT( slotMountEvent() ) );
           
  connect( Smb4KMounter::self(),         SIGNAL( unmounted( Smb4KShare * ) ),
           this,                         SLOT( slotMountEvent() ) );
           
  connect( Smb4KScanner::self(),         SIGNAL( workgroups( const QList<Smb4KWorkgroup *> & ) ),
           this,                         SLOT( slotNetworkEvent() ) );
}


Smb4KSystemTray::~Smb4KSystemTray()
{
}


void Smb4KSystemTray::loadSettings()
{
  // Adjust the bookmarks menu.
  Smb4KBookmarkMenu *menu = findChild<Smb4KBookmarkMenu *>();

  if ( menu )
  {
    menu->refreshMenu();
  }
  else
  {
    // Do nothing
  }
  
  // Adjust the shares menu.
  // slotSetupSharesMenu() is doing everything for us, so just call it.
  setupSharesMenu();
}


void Smb4KSystemTray::setupSharesMenu()
{
  // First check if we have to set up the menu completely:
  if ( !actionCollection()->action( "st_unmount_all_action" ) )
  {
    // OK, build the menu from ground up:
    KAction *unmount_all  = new KAction( KIcon( "system-run" ), i18n( "U&nmount All" ),
                            actionCollection() );
    actionCollection()->addAction( "st_unmount_all_action", unmount_all );

    connect( unmount_all, SIGNAL( triggered( bool ) ), this, SLOT( slotUnmountAllTriggered( bool ) ) );

    m_shares_menu->addAction( unmount_all );
    m_shares_menu->addSeparator();
  }

  // Since we are updating the list of shares very frequently, we should
  // not delete all entries in the menu, but look for changes.

  // Get the list of mounted shares:
  const QList<Smb4KShare *> &shares_list = mountedSharesList();

  if ( !shares_list.isEmpty() )
  {
    // Enable the "Unmount All" action.
    actionCollection()->action( "st_unmount_all_action" )->setEnabled( true );

    // Delete all obsolete actions.
    for ( int i = 0; i < m_share_menus->actions().size(); ++i )
    {
      // Find the associated share by its canonical path.
      QString canonical_path = m_share_menus->actions().at( i )->objectName();
      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share )
      {
        // To avoid sorting problems later, we remove *all* actions that
        // have data entries (displayed texts) that do not match the current
        // criterions.
        if ( (!Smb4KSettings::showMountPoint() &&
             QString::compare( m_share_menus->actions().at( i )->data().toString(),
             share->unc() ) == 0) ||
             (Smb4KSettings::showMountPoint() &&
             QString::compare( m_share_menus->actions().at( i )->data().toString(),
             share->canonicalPath() ) == 0) )
        {
#ifdef __linux__
          // Find the "Force Unmount" action and decide if it needs to be
          // enabled/disabled:
          QAction *force = actionCollection()->action( "st_force_"+canonical_path );
            
          if ( force )
          {
            force->setEnabled( true );
          }
          else
          {
            // Do nothing
          }
#endif
          continue;
        }
        else
        {
          // Remove all actions associated with this share.
          KActionMenu *menu = static_cast<KActionMenu *>( m_share_menus->actions().at( i ) );
          QAction *action = NULL;

          while ( !menu->menu()->actions().isEmpty() )
          {
            // Remove the action from the menu.
            action = menu->menu()->actions().takeFirst();
            // Remove the action from the action group.
            m_shares_actions->removeAction( action );
            // Delete it.
            delete action;
          }

          // Now remove the menu itself.
          m_shares_menu->removeAction( menu );
          m_share_menus->removeAction( menu );
          delete menu;
          continue;
        }
      }
      else
      {
        // First remove all actions associated with this share.
        KActionMenu *menu = static_cast<KActionMenu *>( m_share_menus->actions().at( i ) );
        QAction *action = NULL;

        while ( !menu->menu()->actions().isEmpty() )
        {
          // Remove the action from the menu.
          action = menu->menu()->actions().takeFirst();
          // Remove the action from the action group.
          m_shares_actions->removeAction( action );
          // Delete it.
          delete action;
        }

        // Now remove the menu itself.
        m_shares_menu->removeAction( menu );
        m_share_menus->removeAction( menu );
        delete menu;

        continue;
      }
    }

    // Now look if we have to add some shares or if we have to
    // alter their icon/text.
    // First, work around sorting problems. We cannot sort the
    // Smb4KShare items properly...
    QMap<QString, Smb4KShare> shares_map;

    for ( int i = 0; i < shares_list.size(); ++i )
    {
      // ATTENTION: If the user chose to see the mount points
      // rather than the share name, the mount point is the key,
      // otherwise it is the share name.
      if ( Smb4KSettings::showMountPoint() )
      {
        shares_map.insert( QString::fromUtf8( shares_list.at( i )->canonicalPath() ),
                           *shares_list.at( i ) );
        continue;
      }
      else
      {
        shares_map.insert( shares_list.at( i )->unc(), *shares_list.at( i ) );
        continue;
      }
    }

    // We are ready to insert the new shares into the menu.
    // First, we look in m_share_menus, if an respective menu
    // already exists or not. If not, we add a new menu:
    QMapIterator<QString, Smb4KShare> it( shares_map );

    while ( it.hasNext() )
    {
      it.next();

      KActionMenu *action_menu = NULL;
      QAction *menu = actionCollection()->action( QVariant( it.value().canonicalPath() ).toString() );

      if ( (action_menu = static_cast<KActionMenu *>( menu )) )
      {
        // The action already exists. Have a look whether we have
        // to change anything.
        if ( it.value().isInaccessible() )
        {
          // Change the icon:
          QStringList overlay;
          overlay.append( "emblem-mounted" );

          KIcon icon( "folder-locked", KIconLoader::global(), overlay );

          if ( it.value().isForeign() )
          {
            int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
            KIcon disabled_icon( icon.pixmap( icon_size, QIcon::Disabled ) );

            action_menu->setIcon( disabled_icon );
          }
          else
          {
            action_menu->setIcon( icon );
          }

          // Disable actions that should not be performed on an inaccessible
          // share:
          QAction *synchronize = actionCollection()->action( QVariant( "st_synchronize_"+it.value().canonicalPath() ).toString() );
          
          if ( synchronize )
          {
            synchronize->setEnabled( false );
          }
          else
          {
            // Do nothing
          }
          
          QAction *konsole = actionCollection()->action( QVariant( "st_konsole_"+it.value().canonicalPath() ).toString() );
          
          if ( konsole )
          {
            konsole->setEnabled( false );
          }
          else
          {
            // Do nothing
          }
          
          QAction *filemanager = actionCollection()->action( QVariant( "st_filemanager_"+it.value().canonicalPath() ).toString() );
          
          if ( filemanager )
          {
            filemanager->setEnabled( false );
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

        // Change the text if necessary:
        if ( !Smb4KSettings::showMountPoint() &&
             QString::compare( action_menu->data().toString(), it.value().unc() ) != 0 )
        {
          action_menu->setText( it.value().unc() );
          action_menu->setData( it.value().unc() );
        }
        else if ( Smb4KSettings::showMountPoint() &&
                  QString::compare( action_menu->data().toString(), it.value().path() ) != 0 )
        {
          action_menu->setText( it.value().canonicalPath() );
          action_menu->setData( it.value().canonicalPath() );
        }
        else
        {
          // Do nothing
        }

        // If we have a foreign share, check if we have to enable/disable the
        // unmount actions.
        QAction *unmount = actionCollection()->action( QVariant( "st_unmount_"+it.value().canonicalPath() ).toString() );
        
        if ( unmount )
        {
          unmount->setEnabled( !(it.value().isForeign() && !Smb4KSettings::unmountForeignShares()) );
        }
        else
        {
          // Do nothing
        }
        
#ifdef __linux__
        QAction *force = actionCollection()->action( QVariant( "st_force_"+it.value().canonicalPath() ).toString() );
        
        if ( force )
        {
          force->setEnabled( !(it.value().isForeign() && !Smb4KSettings::unmountForeignShares()) );
        }
        else
        {
          // Do nothing
        }
#endif

        continue;
      }
      else
      {
        // The menu does not exist. Create it with all its entries.
        // First set up the action menu.
        QStringList overlay;
        overlay.append( "emblem-mounted" );

        KIcon icon;

        if ( !it.value().isInaccessible() )
        {
          icon = KIcon( "folder-remote", KIconLoader::global(), overlay );
        }
        else
        {
          icon = KIcon( "folder-locked", KIconLoader::global(), overlay );
        }

        QString text = Smb4KSettings::showMountPoint() ?
                       it.value().canonicalPath() :
                       it.value().unc();

        if ( it.value().isForeign() )
        {
          int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
          KIcon disabled_icon( icon.pixmap( icon_size, QIcon::Disabled ) );

          action_menu = new KActionMenu( disabled_icon, text, m_share_menus );
        }
        else
        {
          action_menu = new KActionMenu( icon, text, m_share_menus );
        }

        action_menu->setObjectName( it.value().canonicalPath() );
        action_menu->setData( text );
        actionCollection()->addAction( it.value().canonicalPath(), action_menu );

        // Now add the actions. We do not need to connect them to any slots,
        // because m_shares_actions is.
        KAction *unmount     = new KAction( KIcon( "media-eject" ), i18n( "&Unmount" ),
                               m_shares_actions );
        unmount->setData( "st_unmount_"+it.value().canonicalPath() );
        unmount->setEnabled( !(it.value().isForeign() && !Smb4KSettings::unmountForeignShares()) );
        actionCollection()->addAction( unmount->data().toString(), unmount );

#ifdef __linux__
        KAction *force       = new KAction( KIcon( "media-eject" ), i18n( "&Force Unmounting" ),
                               m_shares_actions );
        force->setData( "st_force_"+it.value().canonicalPath() );
        force->setEnabled( !(it.value().isForeign() && !Smb4KSettings::unmountForeignShares()) );
        actionCollection()->addAction( force->data().toString(), force );
#endif
        KAction *synchronize = new KAction( KIcon( "go-bottom" ), i18n( "S&ynchronize" ),
                               m_shares_actions );
        synchronize->setData( "st_synchronize_"+it.value().canonicalPath() );
        synchronize->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() );
        actionCollection()->addAction( synchronize->data().toString(), synchronize );

        KAction *konsole     = new KAction( KIcon( "utilities-terminal" ), i18n( "Open with Konso&le" ),
                               m_shares_actions );
        konsole->setData( "st_konsole_"+it.value().canonicalPath() );
        konsole->setEnabled( !KGlobal::dirs()->findResource( "exe", "konsole" ).isEmpty() );
        actionCollection()->addAction( konsole->data().toString(), konsole );

        KAction *filemanager = new KAction( KIcon( "system-file-manager" ), i18n( "Open with F&ile Manager" ),
                               m_shares_actions );
        filemanager->setData( "st_filemanager_"+it.value().canonicalPath() );
        actionCollection()->addAction( filemanager->data().toString(), filemanager );

        action_menu->addAction( unmount );
#ifdef __linux__
        action_menu->addAction( force );
#endif
        action_menu->addSeparator();
        action_menu->addAction( synchronize );
        action_menu->addSeparator();
        action_menu->addAction( konsole );
        action_menu->addAction( filemanager );

        // Now put the menu into the shares menu:
        QAction *action = actionCollection()->action( it.peekPrevious().key() );

        m_shares_menu->insertAction( action, action_menu );
      }
    }
  }
  else
  {
    // Remove all share menus and all their children.
    for ( int i = 0; i < m_share_menus->actions().size(); ++i )
    {
      // First remove all actions associated with this share.
      KActionMenu *menu = static_cast<KActionMenu *>( m_share_menus->actions().at( i ) );
      QAction *action = NULL;

      while ( !menu->menu()->actions().isEmpty() )
      {
        // Remove the action from the menu.
        action = menu->menu()->actions().takeFirst();
        // Remove the action from the action group.
        m_shares_actions->removeAction( action );
        // Delete it.
        delete action;
      }

      // Now remove the menu itself.
      m_shares_menu->removeAction( menu );
      m_share_menus->removeAction( menu );
      delete menu;
    }

    // Disable the "Unmount All" action.
    actionCollection()->action( "st_unmount_all_action" )->setEnabled( false );
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSystemTray::slotMountDialog( bool /* checked */ )
{
  Smb4KMountDialog *dlg = NULL;

  // Do not open the mount dialog twice. So, look
  // if there is already one.
  if ( associatedWidget() )
  {
    dlg = associatedWidget()->findChild<Smb4KMountDialog *>();
  }
  else
  {
    dlg = contextMenu()->findChild<Smb4KMountDialog *>();
  }

  // If there is no dialog yet, create one.
  if ( !dlg )
  {
    if ( associatedWidget() )
    {
      dlg = new Smb4KMountDialog( associatedWidget() );
    }
    else
    {
      // This is a bit strange, but we need a QWidget object.
      // Since KSystemTrayIcon class inherits QObject, we do
      // it this way. 0 as parent would make the check above
      // fail.
      dlg = new Smb4KMountDialog( contextMenu() );
    }
  }
  else
  {
    // Do nothing
  }

  if ( !dlg->isVisible() )
  {
    dlg->show();
  }
  else
  {
    if ( dlg->isMinimized() )
    {
      dlg->showNormal();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KSystemTray::slotConfigDialog()
{
  // If the config dialog is already created and cached,
  // we do not create a new one but show the old instead:
  KConfigDialog *dlg = NULL;

  if ( (dlg = KConfigDialog::exists( "ConfigDialog" )) && KConfigDialog::showDialog( "ConfigDialog" ) )
  {
    // To make sure we do not connect the config dialog several times
    // to slotSettingsChanged(), we break the connection first and re-
    // establish it afterwards:
    disconnect( dlg,  SIGNAL( settingsChanged( const QString & ) ),
                this, SLOT( slotSettingsChanged( const QString & ) ) );

    connect( dlg,  SIGNAL( settingsChanged( const QString & ) ),
             this, SLOT( slotSettingsChanged( const QString & ) ) );

    return;
  }
  else
  {
    // Do nothing
  }

  // Load the configuration dialog:
  KPluginLoader loader( "smb4kconfigdialog" );
  KPluginFactory *config_factory = loader.factory();

  if ( config_factory )
  {
    if ( associatedWidget() )
    {
      dlg = config_factory->create<KConfigDialog>( associatedWidget() );
      dlg->setObjectName( "ConfigDialog" );
    }
    else
    {
      dlg = config_factory->create<KConfigDialog>( contextMenu() );
      dlg->setObjectName( "ConfigDialog" );
    }

    // ... and show it.
    if ( dlg )
    {
      connect( dlg,  SIGNAL( settingsChanged( const QString & ) ),
               this, SLOT( slotSettingsChanged( const QString & ) ) );

      connect( dlg,  SIGNAL( settingsChanged( const QString & ) ),
               this, SIGNAL( settingsChanged( const QString & ) ) );

      dlg->show();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    KMessageBox::error( 0, "<qt>"+loader.errorString()+"</qt>" );

    return;
  }
}

void Smb4KSystemTray::slotSettingsChanged( const QString & )
{
  // Execute loadSettings():
  loadSettings();
}


void Smb4KSystemTray::slotUnmountAllTriggered( bool /* checked */ )
{
  Smb4KMounter::self()->unmountAllShares();
}


void Smb4KSystemTray::slotShareActionTriggered( QAction *action )
{
  if ( action )
  {
    if ( action->data().toString().startsWith( "st_unmount_" ) )
    {
      QString canonical_path = QString( action->data().toString() ).section( "st_unmount_", 1, 1 );

      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share )
      {
        Smb4KMounter::self()->unmountShare( share, false, false );
      }
      else
      {
        // Do nothing
      }
    }
#ifdef __linux__
    else if ( action->data().toString().startsWith( "st_force_" ) )
    {
      QString canonical_path = QString( action->data().toString() ).section( "st_force_", 1, 1 );

      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share )
      {
        Smb4KMounter::self()->unmountShare( share, true, false );
      }
      else
      {
        // Do nothing
      }
    }
#endif
    else if ( action->data().toString().startsWith( "st_synchronize_" ) )
    {
      QString canonical_path = QString( action->data().toString() ).section( "st_synchronize_", 1, 1 );

      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share && !share->isInaccessible() )
      {
        if ( associatedWidget() && associatedWidget()->isVisible() )
        {
          Smb4KSynchronizer::self()->synchronize( share, associatedWidget() );
        }
        else
        {
          // This is a bit strange, but we need a QWidget object.
          // Since KSystemTrayIcon class inherits QObject, we do
          // it this way. 0 as parent witll make the check above
          // fail.
          Smb4KSynchronizer::self()->synchronize( share, contextMenu() );
        }
      }
      else
      {
        // Do nothing
      }
    }
    else if ( action->data().toString().startsWith( "st_konsole_" ) )
    {
      QString canonical_path = QString( action->data().toString() ).section( "st_konsole_", 1, 1 );

      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share && !share->isInaccessible() )
      {
        open( share, Konsole );
      }
      else
      {
        // Do nothing
      }
    }
    else if ( action->data().toString().startsWith( "st_filemanager" ) )
    {
      QString canonical_path = QString( action->data().toString() ).section( "st_filemanager_", 1, 1 );

      Smb4KShare *share = findShareByPath( canonical_path.toUtf8() );

      if ( share && !share->isInaccessible() )
      {
        open( share, FileManager );
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
  else
  {
    // Do nothing
  }
}


void Smb4KSystemTray::slotMountEvent()
{
  // Set the status of the system tray icon.
  if ( !mountedSharesList().isEmpty() || !workgroupsList().isEmpty() )
  {
    setStatus( KStatusNotifierItem::Active );
  }
  else
  {
    setStatus( KStatusNotifierItem::Passive );
  }
  
  // Set up the shares menu.
  setupSharesMenu();
}


void Smb4KSystemTray::slotNetworkEvent()
{
  // Set the status of the system tray icon.
  if ( !mountedSharesList().isEmpty() || !workgroupsList().isEmpty() )
  {
    setStatus( KStatusNotifierItem::Active );
  }
  else
  {
    setStatus( KStatusNotifierItem::Passive );
  }
}

#include "smb4ksystemtray.moc"

