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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

// KDE specific includes
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kconfigdialog.h>
#include <kicon.h>
#include <kstandardaction.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kactioncollection.h>
#include <kmenu.h>

// application specific includes
#include <smb4ksystemtray.h>
#include <smb4kbookmarkmenu.h>
#include <smb4ksharesmenu.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4kshare.h>
#include <core/smb4kglobal.h>
#include <core/smb4kmounter.h>
#include <core/smb4kscanner.h>

using namespace Smb4KGlobal;


Smb4KSystemTray::Smb4KSystemTray( QWidget *parent )
: KStatusNotifierItem( "smb4k_systemtray", parent )
{
  setIconByName( "smb4k" );
  setToolTip( KIconLoader::global()->loadIcon( "smb4k", KIconLoader::NoGroup ), i18n( "Smb4K" ), i18n( "Advanced Network Neighborhood Browser" ) );
  
  // Show the icon to the user. It will become passive, if the scanner
  // could not find something and no shares were mounted.
  setStatus( KStatusNotifierItem::Active );

  // Set up the context menu (skeleton):
  KAction *manual_mount = new KAction( KIcon( "view-form", KIconLoader::global(), QStringList( "emblem-mounted" ) ),
                          i18n( "&Open Mount Dialog" ), actionCollection() );
  KAction *configure    = KStandardAction::preferences( this, SLOT( slotConfigDialog() ),
                          actionCollection() );
  
  Smb4KSharesMenu *shares_menu = new Smb4KSharesMenu( associatedWidget(), this );
  Smb4KBookmarkMenu *bookmark_menu = new Smb4KBookmarkMenu( Smb4KBookmarkMenu::SystemTray, associatedWidget(), this );

  contextMenu()->addAction( shares_menu );
  contextMenu()->addAction( bookmark_menu );
  contextMenu()->addSeparator();
  contextMenu()->addAction( manual_mount );
  contextMenu()->addAction( configure );

  // Connections:
  connect( manual_mount,                 SIGNAL( triggered( bool ) ),
           this,                         SLOT( slotMountDialog( bool ) ) );

  connect( Smb4KMounter::self(),         SIGNAL( mounted( Smb4KShare * ) ),
           this,                         SLOT( slotSetStatus() ) );

  connect( Smb4KMounter::self(),         SIGNAL( unmounted( Smb4KShare * ) ),
           this,                         SLOT( slotSetStatus() ) );

  connect( Smb4KScanner::self(),         SIGNAL( workgroups( const QList<Smb4KWorkgroup *> & ) ),
           this,                         SLOT( slotSetStatus() ) );
}


Smb4KSystemTray::~Smb4KSystemTray()
{
}


void Smb4KSystemTray::loadSettings()
{
  // Adjust the bookmarks menu.
  Smb4KBookmarkMenu *bookmark_menu = findChild<Smb4KBookmarkMenu *>();

  if ( bookmark_menu )
  {
    bookmark_menu->refreshMenu();
  }
  else
  {
    // Do nothing
  }

  // Adjust the shares menu.
  Smb4KSharesMenu *shares_menu = findChild<Smb4KSharesMenu *>();

  if ( shares_menu )
  {
    shares_menu->refreshMenu();
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSystemTray::slotMountDialog( bool /* checked */ )
{
  Smb4KMounter::self()->openMountDialog( associatedWidget() );
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


void Smb4KSystemTray::slotSetStatus()
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

