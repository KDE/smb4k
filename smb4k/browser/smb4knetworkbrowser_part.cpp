/***************************************************************************
    smb4knetworkbrowser_part  -  This Part encapsulates the network
    browser of Smb4K.
                             -------------------
    begin                : Fr Jan 5 2007
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
#include <QKeySequence>
#include <QEvent>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QHeaderView>

// KDE includes
#include <kaboutdata.h>
#include <kaction.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kcomponentdata.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>

// application specific includes
#include <smb4knetworkbrowser_part.h>
#include <smb4knetworkbrowser.h>
#include <smb4knetworkbrowseritem.h>
#include <../tooltips/smb4ktooltip.h>
#include <core/smb4kcore.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksettings.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kwalletmanager.h>
#include <core/smb4kauthinfo.h>
#include <core/smb4kscanner.h>
#include <core/smb4kmounter.h>
#include <core/smb4kipaddressscanner.h>
#include <core/smb4kprint.h>
#include <core/smb4kpreviewer.h>
#include <core/smb4kbookmarkhandler.h>
#include <core/smb4kcustomoptionsmanager.h>
#include <core/smb4kcustomoptions.h>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY( Smb4KNetworkBrowserPartFactory, registerPlugin<Smb4KNetworkBrowserPart>(); )
K_EXPORT_PLUGIN( Smb4KNetworkBrowserPartFactory( "Smb4KNetworkBrowserPart" ) );


Smb4KNetworkBrowserPart::Smb4KNetworkBrowserPart( QWidget *parentWidget, QObject *parent, const QList<QVariant> &args )
: KParts::Part( parent ), m_bookmark_shortcut( true ), m_silent( false )
{
  // Parse arguments:
  for ( int i = 0; i < args.size(); ++i )
  {
    if ( args.at( i ).toString().startsWith( "bookmark_shortcut" ) )
    {
      if ( QString::compare( args.at( i ).toString().section( "=", 1, 1 ).trimmed(), "\"false\"" ) == 0 )
      {
        m_bookmark_shortcut = false;
      }
      else
      {
        // Do nothing
      }

      continue;
    }
    else if ( args.at( i ).toString().startsWith( "silent" ) )
    {
      if ( QString::compare( args.at( i ).toString().section( "=", 1, 1 ).trimmed(), "\"true\"" ) == 0 )
      {
        m_silent = true;
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      continue;
    }
  }

  // Set the XML file:
  setXMLFile( "smb4knetworkbrowser_part.rc" );

  // Set the widget of this part:
  m_widget = new Smb4KNetworkBrowser( parentWidget );
  
  int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
  m_widget->setIconSize( QSize( icon_size, icon_size ) );
  
  setWidget( m_widget );

  // Set up the actions.
  // Do not put this before setWidget() or the shortcuts of the
  // actions will not be shown.
  setupActions();

  // Load the settings
  loadSettings();

  // Add some connections:
  connect( m_widget,               SIGNAL( customContextMenuRequested( const QPoint & ) ),
           this,                   SLOT( slotContextMenuRequested( const QPoint & ) ) );

  connect( m_widget,               SIGNAL( itemSelectionChanged() ),
           this,                   SLOT( slotItemSelectionChanged() ) );

  connect( m_widget,               SIGNAL( itemPressed( QTreeWidgetItem *, int ) ),
           this,                   SLOT( slotItemPressed( QTreeWidgetItem *, int ) ) );

  connect( m_widget,               SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ),
           this,                   SLOT( slotItemExecuted( QTreeWidgetItem *, int ) ) );

  connect( m_widget->tooltip(),    SIGNAL( aboutToShow( Smb4KBasicNetworkItem * ) ),
           this,                   SLOT( slotAboutToShowToolTip( Smb4KBasicNetworkItem * ) ) );

  connect( m_widget->tooltip(),    SIGNAL( aboutToHide( Smb4KBasicNetworkItem * ) ),
           this,                   SLOT( slotAboutToHideToolTip( Smb4KBasicNetworkItem * ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( workgroups( const QList<Smb4KWorkgroup *> & ) ),
           this,                   SLOT( slotWorkgroups( const QList<Smb4KWorkgroup *> & ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( hosts( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ),
           this,                   SLOT( slotWorkgroupMembers( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( shares( Smb4KHost *, const QList<Smb4KShare *> & ) ),
           this,                   SLOT( slotShares( Smb4KHost *, const QList<Smb4KShare *> & ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( info( Smb4KHost * ) ),
           this,                   SLOT( slotAddInformation( Smb4KHost * ) ) );
  
  connect( Smb4KScanner::self(),   SIGNAL( authError( Smb4KHost *, int ) ),
           this,                   SLOT( slotAuthError( Smb4KHost *, int ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( aboutToStart( Smb4KBasicNetworkItem *, int ) ),
           this,                   SLOT( slotScannerAboutToStart( Smb4KBasicNetworkItem *, int ) ) );

  connect( Smb4KScanner::self(),   SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
           this,                   SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );

  connect( Smb4KIPAddressScanner::self(), SIGNAL( ipAddress( Smb4KHost * ) ),
           this,                   SLOT( slotAddIPAddress( Smb4KHost * ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                   SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( finished( Smb4KShare *, int ) ),
           this,                   SLOT( slotMounterFinished( Smb4KShare *, int ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( mounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareMounted( Smb4KShare * ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( unmounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareUnmounted( Smb4KShare * ) ) );

  connect( kapp,                   SIGNAL( aboutToQuit() ),
           this,                   SLOT( slotAboutToQuit() ) );
           
  connect( KGlobalSettings::self(), SIGNAL( iconChanged( int ) ),
           this,                    SLOT( slotIconSizeChanged( int ) ) );
}


Smb4KNetworkBrowserPart::~Smb4KNetworkBrowserPart()
{
}


void Smb4KNetworkBrowserPart::setupActions()
{
  KAction *rescan_action   = new KAction( KIcon( "view-refresh" ), i18n( "Scan Netwo&rk" ),
                             actionCollection() );
  QList<QKeySequence> rescan_shortcuts;
  rescan_shortcuts += QKeySequence::Refresh;
  rescan_shortcuts += QKeySequence( Qt::CTRL+Qt::Key_R );
  rescan_action->setShortcuts( rescan_shortcuts );
  connect( rescan_action, SIGNAL( triggered( bool ) ), this, SLOT( slotRescan( bool ) ) );

  KAction *abort_action    = new KAction( KIcon( "process-stop" ), i18n( "&Abort" ),
                             actionCollection() );
  abort_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_A ) );
  connect( abort_action, SIGNAL( triggered( bool ) ), this, SLOT( slotAbort( bool ) ) );

  KAction *manual_action   = new KAction( KIcon( "view-form", KIconLoader::global(), QStringList( "emblem-mounted" ) ),
                             i18n( "&Open Mount Dialog" ), actionCollection() );
  manual_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_O ) );
  connect( manual_action, SIGNAL( triggered( bool ) ), this, SLOT( slotMountManually( bool ) ) );

  KAction *auth_action     = new KAction( KIcon( "dialog-password" ), i18n( "Au&thentication" ),
                             actionCollection() );
  auth_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_T ) );
  connect( auth_action, SIGNAL( triggered( bool ) ), this, SLOT( slotAuthentication( bool ) ) );

  KAction *custom_action   = new KAction( KIcon( "preferences-system-network" ), i18n( "&Custom Options" ),
                             actionCollection() );
  custom_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_C ) );
  connect( custom_action, SIGNAL( triggered( bool ) ), this, SLOT( slotCustomOptions( bool ) ) );

  KAction *bookmark_action = new KAction( KIcon( "bookmark-new" ), i18n( "Add &Bookmark" ),
                             actionCollection() );
  if ( m_bookmark_shortcut )
  {
    bookmark_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_B ) );
  }
  else
  {
    // Do nothing
  }
  connect( bookmark_action, SIGNAL( triggered( bool ) ), this, SLOT( slotAddBookmark( bool ) ) );

  KAction *preview_action  = new KAction( KIcon( "view-list-icons" ), i18n( "Pre&view" ),
                             actionCollection() );
  preview_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_V ) );
  connect( preview_action, SIGNAL( triggered( bool ) ), this, SLOT( slotPreview( bool ) ) );

  KAction *print_action    = new KAction( KIcon( "printer" ), i18n( "&Print File" ),
                             actionCollection() );
  print_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_P ) );
  connect( print_action, SIGNAL( triggered( bool ) ), this, SLOT( slotPrint( bool ) ) );

  KAction *mount_action    = new KAction( KIcon( "emblem-mounted" ), i18n( "&Mount" ),
                             actionCollection() );
  mount_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_M ) );
  connect( mount_action, SIGNAL( triggered( bool ) ), this, SLOT( slotMount( bool ) ) );

  actionCollection()->addAction( "rescan_action", rescan_action );
  actionCollection()->addAction( "abort_action", abort_action );
  actionCollection()->addAction( "mount_manually_action", manual_action );
  actionCollection()->addAction( "authentication_action", auth_action );
  actionCollection()->addAction( "custom_action", custom_action );
  actionCollection()->addAction( "bookmark_action", bookmark_action );
  actionCollection()->addAction( "preview_action", preview_action );
  actionCollection()->addAction( "print_action", print_action );
  actionCollection()->addAction( "mount_action", mount_action );

  // Enable/disable the actions:
  rescan_action->setEnabled( true );
  abort_action->setEnabled( false );
  manual_action->setEnabled( true );
  auth_action->setEnabled( false );
  custom_action->setEnabled( false );
  bookmark_action->setEnabled( false );
  preview_action->setEnabled( false );
  print_action->setEnabled( false );
  mount_action->setEnabled( false );

  // Plug the actions into the action menu:
  m_menu = new KActionMenu( this );
  m_menu_title = m_menu->menu()->addTitle( KIcon( "network-workgroup" ), i18n( "Network" ) );
  m_menu->addAction( rescan_action );
  m_menu->addAction( abort_action );
  m_menu->addSeparator();
  m_menu->addAction( bookmark_action );
  m_menu->addAction( manual_action );
  m_menu->addSeparator();
  m_menu->addAction( auth_action );
  m_menu->addAction( custom_action );
  m_menu->addAction( preview_action );
  m_menu->addAction( print_action );
  m_menu->addAction( mount_action );
}


void Smb4KNetworkBrowserPart::loadSettings()
{
  // Show/hide columns:
  m_widget->setColumnHidden( Smb4KNetworkBrowser::IP, !Smb4KSettings::showIPAddress() );
  m_widget->setColumnHidden( Smb4KNetworkBrowser::Type, !Smb4KSettings::showType() );
  m_widget->setColumnHidden( Smb4KNetworkBrowser::Comment, !Smb4KSettings::showComment() );

  // Load and apply the positions of the columns.
  KConfigGroup configGroup( Smb4KSettings::self()->config(), "NetworkBrowserPart" );

  QMap<int, int> map;
  map.insert( configGroup.readEntry( "ColumnPositionNetwork", (int)Smb4KNetworkBrowser::Network ), Smb4KNetworkBrowser::Network );
  map.insert( configGroup.readEntry( "ColumnPositionType", (int)Smb4KNetworkBrowser::Type ), Smb4KNetworkBrowser::Type );
  map.insert( configGroup.readEntry( "ColumnPositionIP", (int)Smb4KNetworkBrowser::IP ), Smb4KNetworkBrowser::IP );
  map.insert( configGroup.readEntry( "ColumnPositionComment", (int)Smb4KNetworkBrowser::Comment ), Smb4KNetworkBrowser::Comment );

  QMap<int, int>::const_iterator it = map.constBegin();

  while ( it != map.constEnd() )
  {
    if ( it.key() != m_widget->header()->visualIndex( it.value() ) )
    {
      m_widget->header()->moveSection( m_widget->header()->visualIndex( it.value() ), it.key() );
    }
    else
    {
      // Do nothing
    }

    ++it;
  }

  // Does anything has to be changed with the marked shares?
  for ( int i = 0; i < mountedSharesList().size(); ++i )
  {
    // We do not need to use slotShareUnmounted() here, too,
    // because slotShareMounted() will take care of everything
    // we need here.
    slotShareMounted( mountedSharesList().at( i ) );
  }
}


void Smb4KNetworkBrowserPart::saveSettings()
{
  // Save the position of the columns.
  KConfigGroup configGroup( Smb4KSettings::self()->config(), "NetworkBrowserPart" );
  configGroup.writeEntry( "ColumnPositionNetwork", m_widget->header()->visualIndex( Smb4KNetworkBrowser::Network ) );
  configGroup.writeEntry( "ColumnPositionType", m_widget->header()->visualIndex( Smb4KNetworkBrowser::Type ) );
  configGroup.writeEntry( "ColumnPositionIP", m_widget->header()->visualIndex( Smb4KNetworkBrowser::IP ) );
  configGroup.writeEntry( "ColumnPositionComment", m_widget->header()->visualIndex( Smb4KNetworkBrowser::Comment ) );

  configGroup.sync();
}


KAboutData *Smb4KNetworkBrowserPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData( "smb4knetworkbrowserpart",
                          "smb4k",
                          ki18n( "Smb4KNetworkBrowserPart" ),
                          "3.0",
                          ki18n( "The network browser KPart of Smb4K" ),
                          KAboutData::License_GPL_V2,
                          ki18n( "\u00A9 2007-2011, Alexander Reinholdt" ),
                          KLocalizedString(),
                          "http://smb4k.berlios.de",
                          "smb4k-bugs@lists.berlios.de" );

  return aboutData;
}


void Smb4KNetworkBrowserPart::customEvent( QEvent *e )
{
  if ( e->type() == Smb4KEvent::LoadSettings )
  {
    loadSettings();
  }
  else if ( e->type() == Smb4KEvent::SetFocus )
  {
    if ( m_widget->topLevelItemCount() != 0 )
    {
      kDebug() << "Do we need to port the selection stuff?" << endl;
    }

    m_widget->setFocus( Qt::OtherFocusReason );
  }
  else if ( e->type() == Smb4KEvent::ScanNetwork )
  {
    slotRescan( false ); // boolean is ignored
  }
  else if ( e->type() == Smb4KEvent::AddBookmark )
  {
    slotAddBookmark( false );
  }
  else
  {
    // Do nothing
  }

  KParts::Part::customEvent( e );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS (Smb4KNetworkBrowserPart)
/////////////////////////////////////////////////////////////////////////////

void Smb4KNetworkBrowserPart::slotContextMenuRequested( const QPoint &pos )
{
  QTreeWidgetItem *item = m_widget->itemAt( pos );

  m_menu->removeAction( m_menu_title );
  delete m_menu_title;

  if ( item )
  {
    m_menu_title = m_menu->menu()->addTitle( item->icon( Smb4KNetworkBrowserItem::Network ),
                                             item->text( Smb4KNetworkBrowserItem::Network ),
                                             actionCollection()->action( "rescan_action" ) );
  }
  else
  {
    m_menu_title = m_menu->menu()->addTitle( KIcon( "network-workgroup" ),
                                             i18n( "Network" ),
                                             actionCollection()->action( "rescan_action" ) );
  }

  m_menu->menu()->popup( m_widget->viewport()->mapToGlobal( pos ) );
}


void Smb4KNetworkBrowserPart::slotItemSelectionChanged()
{
  // Get the selected item.
  QList<QTreeWidgetItem *> items = m_widget->selectedItems();

  if ( items.size() == 1 )
  {
    Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>( items.first() );

    if ( browser_item )
    {
      switch ( browser_item->type() )
      {
        case Smb4KNetworkBrowserItem::Host:
        {
          // Change the text of the rescan action:
          actionCollection()->action( "rescan_action" )->setText( i18n( "Scan Compute&r" ) );

          // Enable/disable the actions:
          actionCollection()->action( "bookmark_action" )->setEnabled( false );
          actionCollection()->action( "authentication_action" )->setEnabled( true );
          actionCollection()->action( "preview_action" )->setEnabled( false );
          actionCollection()->action( "mount_action" )->setEnabled( false );
          actionCollection()->action( "print_action" )->setEnabled( false );
          actionCollection()->action( "custom_action" )->setEnabled( true );
          break;
        }
        case Smb4KNetworkBrowserItem::Share:
        {
          // Change the text of the rescan action:
          actionCollection()->action( "rescan_action" )->setText( i18n( "Scan Compute&r" ) );

          // Enable/disable the actions:
          actionCollection()->action( "authentication_action" )->setEnabled( true );

          if ( !browser_item->shareItem()->isPrinter() )
          {
            actionCollection()->action( "bookmark_action" )->setEnabled( true );
            actionCollection()->action( "preview_action" )->setEnabled( true );
            actionCollection()->action( "mount_action" )->setEnabled( !browser_item->shareItem()->isMounted() ||
                                (browser_item->shareItem()->isMounted() && browser_item->shareItem()->isForeign()) );
            actionCollection()->action( "print_action" )->setEnabled( false );
            actionCollection()->action( "custom_action" )->setEnabled( true );
          }
          else
          {
            actionCollection()->action( "bookmark_action" )->setEnabled( false );
            actionCollection()->action( "preview_action" )->setEnabled( false );
            actionCollection()->action( "mount_action" )->setEnabled( false );
            actionCollection()->action( "print_action" )->setEnabled( 
              !Smb4KPrint::self()->isRunning( browser_item->shareItem() ) );
            actionCollection()->action( "custom_action" )->setEnabled( false );
          }
          break;
        }
        default:
        {
          // Change the text of the rescan action:
          actionCollection()->action( "rescan_action" )->setText( i18n( "Scan Wo&rkgroup" ) );
          actionCollection()->action( "bookmark_action" )->setEnabled( false );
          actionCollection()->action( "authentication_action" )->setEnabled( false );
          actionCollection()->action( "preview_action" )->setEnabled( false );
          actionCollection()->action( "mount_action" )->setEnabled( false );
          actionCollection()->action( "print_action" )->setEnabled( false );
          actionCollection()->action( "custom_action" )->setEnabled( false );
          break;
        }
      }
    }
    else
    {
      // Do nothing. This is managed elsewhere.
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotItemPressed( QTreeWidgetItem *item, int /*column*/ )
{
  // Enable/disable the actions.
  Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>( item );

  if ( !browser_item && m_widget->selectedItems().size() == 0 )
  {
    actionCollection()->action( "rescan_action" )->setText( i18n( "Scan Netwo&rk" ) );
    actionCollection()->action( "bookmark_action" )->setEnabled( false );
    actionCollection()->action( "authentication_action" )->setEnabled( false );
    actionCollection()->action( "preview_action" )->setEnabled( false );
    actionCollection()->action( "mount_action" )->setEnabled( false );
    actionCollection()->action( "print_action" )->setEnabled( false );
    actionCollection()->action( "custom_action" )->setEnabled( false );
  }
  else
  {
    if ( browser_item )
    {
      switch ( browser_item->type() )
      {
        case Smb4KNetworkBrowserItem::Share:
        {
          if ( browser_item->shareItem()->isPrinter() )
          {
            actionCollection()->action( "print_action" )->setEnabled( 
              !Smb4KPrint::self()->isRunning( browser_item->shareItem() ) );
            actionCollection()->action( "mount_action" )->setEnabled( false );
          }
          else
          {
            actionCollection()->action( "mount_action" )->setEnabled( !browser_item->shareItem()->isMounted() ||
                                (browser_item->shareItem()->isMounted() && browser_item->shareItem()->isForeign()) );
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
}


void Smb4KNetworkBrowserPart::slotItemExecuted( QTreeWidgetItem *item, int /*column*/ )
{
  Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>( item );

  if ( browserItem )
  {
    switch ( browserItem->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      {
        if ( browserItem->isExpanded() )
        {
          Smb4KScanner::self()->lookupDomainMembers( browserItem->workgroupItem(), m_widget );
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        if ( browserItem->isExpanded() )
        {
          Smb4KScanner::self()->lookupShares( browserItem->hostItem(), m_widget );
        }
        else
        {
          // Do nothing
        }
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        if ( !browserItem->shareItem()->isPrinter() )
        {
          slotMount( false );  // boolean is ignored
        }
        else
        {
          slotPrint( false );  // boolean is ignored
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


void Smb4KNetworkBrowserPart::slotAboutToShowToolTip( Smb4KBasicNetworkItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        // Check if additional information is needed and send a request to the scanner,
        // if necessary.
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        
        if ( !host->infoChecked() )
        {
          Smb4KScanner::self()->lookupInfo( host, m_widget );
        }
        else
        {
          // Do nothing
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
    // Do nothing --- BTW: Will this case occur at all?
  }
}


void Smb4KNetworkBrowserPart::slotAboutToHideToolTip( Smb4KBasicNetworkItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KBasicNetworkItem::Host:
      {
        // Kill the lookup process for the additional information.
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        Smb4KScanner::self()->abort( Smb4KScanner::LookupInfo, host );
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
    // Do nothing --- BTW: Will this case occur at all?
  }
}


void Smb4KNetworkBrowserPart::slotWorkgroups( const QList<Smb4KWorkgroup *> &list )
{
  // Process the list.
  if ( !list.isEmpty() )
  {
    // Remove obsolete workgroup items from the tree widget.
    for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
    {
      Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->topLevelItem( i ) );

      for ( int j = 0; j < list.size(); ++j )
      {
        bool found_workgroup = false;

        if ( QString::compare( list.at( j )->workgroupName(), workgroup_item->workgroupItem()->workgroupName() ) == 0 )
        {
          // Check if the master browser is still the same.
          if ( QString::compare( list.at( j )->masterBrowserName(), workgroup_item->workgroupItem()->masterBrowserName() ) != 0 )
          {
            // We found the workgroup.
            bool found_new_master_browser = false;

            // Find the old and the new master browser.
            for ( int k = 0; k < workgroup_item->childCount(); ++k )
            {
              Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>( workgroup_item->child( k ) );

              if ( QString::compare( workgroup_item->workgroupItem()->masterBrowserName(), host_item->hostItem()->hostName() ) == 0 )
              {
                // This is the old master browser. Update it.
                Smb4KHost *host = findHost( host_item->hostItem()->hostName(), host_item->hostItem()->workgroupName() );

                if ( host )
                {
                  // Update.
                  host_item->update( host );
                }
                else
                {
                  // The old master is not available anymore.
                  // Remove it.
                  delete host_item;
                }

                continue;
              }
              else if ( QString::compare( list.at( j )->masterBrowserName(), host_item->hostItem()->hostName() ) == 0 )
              {
                // This is the new master browser. Update it.
                Smb4KHost *host = findHost( host_item->hostItem()->hostName(), host_item->hostItem()->workgroupName() );

                if ( host )
                {
                  // Update.
                  host_item->update( host );
                }
                else
                {
                  // Huh???
                }

                continue;
              }
              else
              {
                continue;
              }
            }

            if ( !found_new_master_browser )
            {
              // The new master browser is not in the tree widget, so we have to put it there.
              Smb4KHost *host = findHost( workgroup_item->workgroupItem()->masterBrowserName(), workgroup_item->workgroupItem()->workgroupName() );
              (void) new Smb4KNetworkBrowserItem( workgroup_item, host );
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

          // Update the workgroup item.
          workgroup_item->update( list.at( j ) );

          // We found the workgroup.
          found_workgroup = true;

          break;
        }
        else
        {
          continue;
        }

        // Remove the workgroup from the tree widget if it is obsolete.
        if ( !found_workgroup )
        {
          delete workgroup_item;
        }
        else
        {
          // Do nothing
        }
      }
    }

    // Put the new workgroup items into the tree widget.
    for ( int i = 0; i < list.size(); ++i )
    {
      QList<QTreeWidgetItem *> items = m_widget->findItems( list.at( i )->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );

      if ( items.isEmpty() )
      {
        // The workgroup is not in the tree widget. Add it.
        (void) new Smb4KNetworkBrowserItem( m_widget, list.at( i ) );
        continue;
      }
      else
      {
        continue;
      }
    }

    // Sort the items.
    m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );
  }
  else
  {
    // If we are in IP scan mode, there might be the case, that
    // the master browser could not be discovered (That's right!
    // This happened in one of the networks that I used for testing.).
    // In this case, we should have a look whether the list of
    // host is empty or not.before clearing the tree widget.
    if ( !Smb4KSettings::scanBroadcastAreas() )
    {
      // Clear the tree widget.
      m_widget->clear();
    }
    else
    {
      if ( hostsList().isEmpty() )
      {
        // Clear the tree widget.
        m_widget->clear();
      }
      else
      {
        // Do nothing
      }
    }
  }
}


void Smb4KNetworkBrowserPart::slotWorkgroupMembers( Smb4KWorkgroup *workgroup, const QList<Smb4KHost *> &list )
{
  if ( workgroup )
  {
    // Find the workgroup.
    QList<QTreeWidgetItem *> workgroups = m_widget->findItems( workgroup->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );

    if ( !list.isEmpty() )
    {
      QMutableListIterator<QTreeWidgetItem *> it( workgroups );
      
      while ( it.hasNext() )
      {
        Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( it.next() );
        
        // Remove obsolete hosts from the workgroup.
        for ( int j = 0; j < workgroup_item->childCount(); ++j )
        {
          Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>( workgroup_item->child( j ) );
          bool found_host = false;

          for ( int k = 0; k < list.size(); ++k )
          {
            if ( QString::compare( list.at( k )->workgroupName(), host_item->hostItem()->workgroupName() ) == 0 &&
                 QString::compare( list.at( k )->hostName(), host_item->hostItem()->hostName() ) == 0 )
            {
              found_host = true;
              break;
            }
            else
            {
              continue;
            }
          }

          if ( !found_host )
          {
            delete host_item;
          }
          else
          {
            // Do nothing
          }
        }
        
        // Add new hosts to the workgroup and update the existing ones.
        for ( int j = 0; j < list.size(); ++j )
        {
          if ( QString::compare( list.at( j )->workgroupName(), workgroup_item->workgroupItem()->workgroupName() ) == 0 )
          {
            bool found_host = false;

            for ( int k = 0; k < workgroup_item->childCount(); ++k )
            {
              Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>( workgroup_item->child( k ) );

              if ( QString::compare( list.at( j )->hostName(), host_item->hostItem()->hostName() ) == 0 )
              {
                host_item->update( list.at( j ) );
                found_host = true;
                break;
              }
              else
              {
                continue;
              }
            }

            if ( !found_host )
            {
              (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( j ) );
            }
            else
            {
              // Do nothing
            }

            continue;
          }
          else
          {
            continue;
          }
        }
        
        // If the workgroup item has children, expand it if necessary. 
        // Otherwise there is no use in keeping it, so remove it.
        if ( workgroup_item->childCount() != 0 )
        {
          if ( Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded() )
          {
            m_widget->expandItem( workgroup_item );
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          delete workgroup_item;
        }

        // Sort the items.
        m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );     
      }
    }
    else
    {
      // The workgroup(s) has/have no children (anymore). Remove
      // it/them.
      while ( !workgroups.isEmpty() )
      {
        delete workgroups.takeFirst();
      }
    }
  }
  else
  {
    // This is the list of all hosts available on the network.
    if ( !list.isEmpty() )
    {
      // Loop through the network neighborhood tree. Remove the obsolete host
      // and update the still available ones.
      QTreeWidgetItemIterator it( m_widget );
      
      while ( *it )
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( *it );
        
        switch ( item->type() )
        {
          case Smb4KNetworkBrowserItem::Host:
          {
            bool found = false;
            
            for ( int i = 0; i < list.size(); ++i )
            {
              if ( QString::compare( list.at( i )->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive ) == 0 &&
                   QString::compare( list.at( i )->unc(), item->hostItem()->unc(), Qt::CaseInsensitive ) == 0 )
              {
                item->update( list.at( i ) );
                found = true;
                break;
              }
              else
              {
                continue;
              }
            }
            
            if ( !found )
            {
              delete item;
            }
            else
            {
              // Do nothing
            }            
            break;
          }
          default:
          {
            break;
          }
        }
        
        ++it;
      }
      
      // Now add the new hosts. For this we search the network neighborhood
      // tree and add the host if either there is no match or the found host(s)
      // belong(s) to another workgroup.
      for ( int i = 0; i < list.size(); ++i )
      {
        QList<QTreeWidgetItem *> hosts = m_widget->findItems( list.at( i )->hostName(), 
                                                              Qt::MatchFixedString|Qt::MatchRecursive, 
                                                              Smb4KNetworkBrowser::Network );
        
        if ( !hosts.isEmpty() )
        {
          bool match = false;
          
          for ( int j = 0; j < hosts.size(); ++j )
          {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( hosts.at( i ) );
            
            if ( item->type() == Smb4KNetworkBrowserItem::Host &&
                 QString::compare( list.at( i )->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive ) == 0 &&
                 QString::compare( list.at( i )->unc(), item->hostItem()->unc(), Qt::CaseInsensitive ) == 0 )
            {
              match = true;
              break;
            }
            else
            {
              continue;
            }
          }
          
          if ( !match )
          {
            // This host is new. Add it to the list widget and create the
            // workgroup item as well if needed.
            QList<QTreeWidgetItem *> workgroups = m_widget->findItems( list.at( i )->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );
            
            if ( !workgroups.isEmpty() )
            {
              for ( int j = 0; j < workgroups.size(); ++j )
              {
                Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( workgroups.at( j ) );

                if ( workgroup_item )
                {
                  // FIXME: Do we need to change the (pseudo) master browser here?
                  (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );
                  
                  if ( Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded() )
                  {
                    m_widget->expandItem( workgroup_item );
                  }
                  else
                  {
                    // Do nothing
                  }
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
              // We need to create the workgroup and the host item. Since the workgroup
              // was not found in the browser, we can assume, that it is also not available
              // in the global list. This may happen, if no master browser could be found
              // during an IP scan. So, we will create a temporary workgroup item now.
              Smb4KWorkgroup workgroup;
              workgroup.setWorkgroupName( list.at( i )->workgroupName() );

              if ( list.at( i )->isMasterBrowser() )
              {
                workgroup.setMasterBrowserName( list.at( i )->hostName() );
                workgroup.setMasterBrowserIP( list.at( i )->ip() );
                workgroup.setHasPseudoMasterBrowser( !list.at( i )->isMasterBrowser() );
              }
              else
              {
                // Do nothing
              }

              Smb4KNetworkBrowserItem *workgroup_item = new Smb4KNetworkBrowserItem( m_widget, &workgroup );
              (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );
              
              if ( Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded() )
              {
                m_widget->expandItem( workgroup_item );
              }
              else
              {
                // Do nothing
              }
              continue;
            }
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // This host is new. Add it to the list widget and create the
          // workgroup item as well if needed.
          QList<QTreeWidgetItem *> workgroups = m_widget->findItems( list.at( i )->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );
          
          if ( !workgroups.isEmpty() )
          {
            for ( int j = 0; j < workgroups.size(); ++j )
            {
              Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( workgroups.at( j ) );

              if ( workgroup_item )
              {
                // FIXME: Do we need to change the (pseudo) master browser here?
                (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );
                
                if ( Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded() )
                {
                  m_widget->expandItem( workgroup_item );
                }
                else
                {
                  // Do nothing
                }
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
            // We need to create the workgroup and the host item. Since the workgroup
            // was not found in the browser, we can assume, that it is also not available
            // in the global list. This may happen, if no master browser could be found
            // during an IP scan. So, we will create a temporary workgroup item now.
            Smb4KWorkgroup workgroup;
            workgroup.setWorkgroupName( list.at( i )->workgroupName() );

            if ( list.at( i )->isMasterBrowser() )
            {
              workgroup.setMasterBrowserName( list.at( i )->hostName() );
              workgroup.setMasterBrowserIP( list.at( i )->ip() );
              workgroup.setHasPseudoMasterBrowser( !list.at( i )->isMasterBrowser() );
            }
            else
            {
              // Do nothing
            }

            Smb4KNetworkBrowserItem *workgroup_item = new Smb4KNetworkBrowserItem( m_widget, &workgroup );
            (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );
            
            if ( Smb4KSettings::autoExpandNetworkItems() && !workgroup_item->isExpanded() )
            {
              m_widget->expandItem( workgroup_item );
            }
            else
            {
              // Do nothing
            }
            continue;
          }
        }
      }
      
      // Sort the items.
      m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );
    }
    else
    {
      // If the list of hosts is empty, we can clear the whole 
      // network neighborhood tree.
      m_widget->clear();
    }
  }
}


void Smb4KNetworkBrowserPart::slotShares( Smb4KHost *host, const QList<Smb4KShare *> &list )
{
  if ( host )
  {
    QList<QTreeWidgetItem *> network_items = m_widget->findItems( host->hostName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network );

    // There might be several entries in the list that have the same.
    // So, loop through the list, get the host(s) and process it/them.
    for ( int i = 0; i < network_items.size(); ++i )
    {
      Smb4KNetworkBrowserItem *network_item = static_cast<Smb4KNetworkBrowserItem *>( network_items.at( i ) );

      if ( network_item->type() == Smb4KNetworkBrowserItem::Host )
      {
        if ( !list.isEmpty() )
        {
          if ( Smb4KSettings::autoExpandNetworkItems() && !network_item->isExpanded() )
          {
            m_widget->expandItem( network_item );
          }
          else
          {
            // Do nothing
          }

          // Update and add the shares.
          for ( int j = 0; j < list.size(); ++j )
          {
            bool found_item = false;

            if ( QString::compare( network_item->hostItem()->hostName(), list.at( j )->hostName() ) == 0 )
            {
              for ( int k = 0; k < network_item->childCount(); ++k )
              {
                // In case the list also carries entries for other hosts, check that
                // the host name of the entry in the list and of the current host in
                // the tree widget are the same.
                Smb4KNetworkBrowserItem *share_item = static_cast<Smb4KNetworkBrowserItem *>( network_item->child( k ) );

                if ( QString::compare( list.at( j )->shareName(), share_item->shareItem()->shareName() ) == 0 )
                {
                  // We found the share. Now process it.
                  if ( !share_item->shareItem()->isHidden() )
                  {
                    if ( !share_item->shareItem()->isPrinter() || Smb4KSettings::showPrinterShares() )
                    {
                      share_item->update( list.at( j ) );
                    }
                    else
                    {
                      // Do nothing. The item will be deleted below.
                    }

                    found_item = true;
                    break;
                  }
                  else
                  {
                    if ( Smb4KSettings::showHiddenShares() )
                    {
                      if ( (share_item->shareItem()->isPrinter() && Smb4KSettings::showPrinterShares()) ||
                           (share_item->shareItem()->isIPC() && Smb4KSettings::showHiddenIPCShares()) ||
                           (share_item->shareItem()->isADMIN() && Smb4KSettings::showHiddenADMINShares()) )
                      {
                        share_item->update( list.at( j ) );
                      }
                      else
                      {
                        // Do nothing. The item will be deleted below.
                      }
                    }
                    else
                    {
                      // Do nothing. The item will be deleted below.
                    }

                    found_item = true;
                    break;
                  }
                }
                else
                {
                  continue;
                }
              }

              if ( !found_item )
              {
                if ( !list.at( j )->isHidden() )
                {
                  if ( !list.at( j )->isPrinter() || Smb4KSettings::showPrinterShares() )
                  {
                    (void) new Smb4KNetworkBrowserItem( network_item, list.at( j ) );
                  }
                  else
                  {
                    // Do nothing
                  }

                  continue;
                }
                else
                {
                  if ( Smb4KSettings::showHiddenShares() )
                  {
                    if ( (!list.at( j )->isPrinter() && !list.at( j )->isIPC() && !list.at( j )->isADMIN()) ||
                         (list.at( j )->isPrinter() && Smb4KSettings::showPrinterShares()) ||
                         (list.at( j )->isIPC() && Smb4KSettings::showHiddenIPCShares()) ||
                         (list.at( j )->isADMIN() && Smb4KSettings::showHiddenADMINShares()) )
                    {
                      (void) new Smb4KNetworkBrowserItem( network_item, list.at( j ) );
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

                  continue;
                }
              }
              else
              {
                continue;
              }
            }
          }

          // Delete obsolete shares.
          for ( int j = 0; j < network_item->childCount(); ++j )
          {
            Smb4KNetworkBrowserItem *share_item = static_cast<Smb4KNetworkBrowserItem *>( network_item->child( j ) );
            bool found_share = false;

            for ( int k = 0; k < list.size(); ++k )
            {
              if ( QString::compare( share_item->shareItem()->hostName(), list.at( k )->hostName() ) == 0 &&
                   QString::compare( share_item->shareItem()->shareName(), list.at( k )->shareName() ) == 0 )
              {
                // Test if the share needs to be deleted. If this is the case, we
                // won't set found_share to TRUE.

                // (a) Printer shares
                if ( !Smb4KSettings::showPrinterShares() && share_item->shareItem()->isPrinter() )
                {
                  break;
                }
                else
                {
                  // Do nothing.
                }

                // (b) Hidden shares
                if ( !Smb4KSettings::showHiddenShares() && share_item->shareItem()->isHidden() )
                {
                  break;
                }
                else
                {
                  if ( (!Smb4KSettings::showHiddenIPCShares() && share_item->shareItem()->isIPC()) ||
                       (!Smb4KSettings::showHiddenADMINShares() && share_item->shareItem()->isADMIN()) )
                  {
                    break;
                  }
                  else
                  {
                    // Do nothing
                  }
                }

                found_share = true;
              }
              else
              {
                continue;
              }
            }

            if ( !found_share )
            {
              delete share_item;
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
          // Collapse the item.
          m_widget->collapseItem( network_item );

          // Delete all share items from this host:
          while ( network_item->childCount() != 0 )
          {
            delete network_item->child( 0 );
          }
        }

        // Sort the items.
        m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );
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


void Smb4KNetworkBrowserPart::slotAddIPAddress( Smb4KHost *host )
{
  if ( host )
  {
    // Get the workgroup item where the host is located.
    Smb4KWorkgroup *workgroup = findWorkgroup( host->workgroupName() );

    // Find the workgroup of the host in the tree widget.
    QList<QTreeWidgetItem *> workgroups = m_widget->findItems( host->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );

    // Update the workgroup and the host.
    // This loop is most like absolutely unnecessary, because there should only
    // be one single workgroup with host->workgroupName(), but we want to be
    // prepared also for weird networks.
    for ( int i = 0; i < workgroups.size(); ++i )
    {
      Smb4KNetworkBrowserItem *workgroupItem = static_cast<Smb4KNetworkBrowserItem *>( workgroups.at( i ) );

      for ( int j = 0; j < workgroupItem->childCount(); ++j )
      {
        if ( QString::compare( host->hostName(), workgroupItem->child( j )->text( Smb4KNetworkBrowserItem::Network ) ) == 0 )
        {
          Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>( workgroupItem->child( j ) );

          if ( hostItem )
          {
            // Update host and workgroup.
            hostItem->update( host );
            workgroupItem->update( workgroup );

            // Now adjust the IP address column, if it is not hidden.
            if ( !m_widget->isColumnHidden( Smb4KNetworkBrowser::IP ) )
            {
              m_widget->resizeColumnToContents( Smb4KNetworkBrowser::IP );
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
          
          if ( m_widget->tooltip() && m_widget->tooltip()->isVisible() &&
               (QString::compare( m_widget->tooltip()->networkItem()->key(), hostItem->networkItem()->key() ) == 0 ||
                QString::compare( m_widget->tooltip()->networkItem()->key(), workgroupItem->networkItem()->key() ) == 0) )
          {
            m_widget->tooltip()->update();
          }
          else
          {
            // Do nothing
          }
          break;
        }
        else
        {
          continue;
        }
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotAddInformation( Smb4KHost *host )
{
  if ( host )
  {
    // Find the host.
    QList<QTreeWidgetItem *> hosts = m_widget->findItems( host->hostName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network );

    for ( int i = 0; i < hosts.size(); ++i )
    {
      Smb4KNetworkBrowserItem *workgroupItem = static_cast<Smb4KNetworkBrowserItem *>( hosts.at( i )->parent() );

      if ( QString::compare( host->workgroupName(), workgroupItem->text( Smb4KNetworkBrowserItem::Network ) ) == 0 )
      {
        Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>( hosts.at( i ) );
        hostItem->update( host );

        // Now update the tool tip in case it is shown:
        if ( m_widget->tooltip() && m_widget->tooltip()->isVisible() &&
             QString::compare( m_widget->tooltip()->networkItem()->key(), hostItem->networkItem()->key() ) == 0 )
        {
          m_widget->tooltip()->update();
        }
        else
        {
          // Do nothing
        }

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


void Smb4KNetworkBrowserPart::slotAuthError( Smb4KHost *host, int process )
{
  switch ( process )
  {
    case Smb4KScanner::LookupDomains:
    {
      // We queried a master browser from the list of domains and
      // workgroup. So, we can clear the whole list of domains.
      while ( m_widget->topLevelItemCount() != 0 )
      {
        delete m_widget->takeTopLevelItem( 0 );
      }
      break;
    }
    case Smb4KScanner::LookupDomainMembers:
    {
      // Get the workgroup where the master browser is not accessible 
      // and clear the whole list of hosts. Then, reinsert the master 
      // browser.
      if ( !m_widget->topLevelItemCount() != 0 )
      {
        for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
        {
          Smb4KNetworkBrowserItem *workgroup = static_cast<Smb4KNetworkBrowserItem *>( m_widget->topLevelItem( i ) );
          
          if ( workgroup && workgroup->type() == Smb4KNetworkBrowserItem::Workgroup &&
               QString::compare( host->workgroupName(), workgroup->workgroupItem()->workgroupName(), Qt::CaseInsensitive ) == 0 )
          {
            QList<QTreeWidgetItem *> hosts = workgroup->takeChildren();
            
            while ( !hosts.isEmpty() )
            {
              delete hosts.takeFirst();
            }
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
      break;
    }
    case Smb4KScanner::LookupShares:
    {
      // Get the host that could not be accessed.
      QTreeWidgetItemIterator it( m_widget );
      
      while ( *it )
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( *it );
        
        if ( item && item->type() == Smb4KNetworkBrowserItem::Host )
        {
          if ( QString::compare( host->hostName(), item->hostItem()->hostName(), Qt::CaseInsensitive ) == 0 &&
               QString::compare( host->workgroupName(), item->hostItem()->workgroupName(), Qt::CaseInsensitive ) == 0 )
          {
            QList<QTreeWidgetItem *> shares = item->takeChildren();
            
            while ( !shares.isEmpty() )
            {
              delete shares.takeFirst();
            }
            break;
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
        ++it;
      }
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KNetworkBrowserPart::slotRescan( bool /* checked */)
{
  // The mouse is inside the viewport. Let's see what we have to do.
  if ( m_widget->currentItem() && m_widget->currentItem()->isSelected() )
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      {
        Smb4KScanner::self()->lookupDomainMembers( item->workgroupItem(), m_widget );
        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        Smb4KScanner::self()->lookupShares( item->hostItem(), m_widget );
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KNetworkBrowserItem *parentItem = static_cast<Smb4KNetworkBrowserItem *>( item->parent() );
        Smb4KScanner::self()->lookupShares( parentItem->hostItem(), m_widget );
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
    Smb4KScanner::self()->lookupDomains( m_widget );
  }
}


void Smb4KNetworkBrowserPart::slotAbort( bool /*checked*/ )
{
  if ( Smb4KScanner::self()->isRunning() )
  {
    Smb4KScanner::self()->abortAll();
  }
  else
  {
    // Do nothing
  }

  if ( Smb4KMounter::self()->isRunning() )
  {
    Smb4KMounter::self()->abortAll();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotMountManually( bool /*checked*/ )
{
  Smb4KMounter::self()->openMountDialog( m_widget );
}


void Smb4KNetworkBrowserPart::slotAuthentication( bool /*checked*/ )
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        Smb4KWalletManager::self()->showPasswordDialog( item->hostItem(), m_widget );
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KWalletManager::self()->showPasswordDialog( item->shareItem(), m_widget );
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


void Smb4KNetworkBrowserPart::slotCustomOptions( bool /*checked*/ )
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog( item->hostItem(), m_widget );
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KCustomOptionsManager::self()->openCustomOptionsDialog( item->shareItem(), m_widget );
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


void Smb4KNetworkBrowserPart::slotAddBookmark( bool /*checked*/ )
{
  QList<QTreeWidgetItem *> selected_items = m_widget->selectedItems();
  QList<Smb4KShare *> shares;

  if ( !selected_items.isEmpty() )
  {
    for ( int i = 0; i < selected_items.size(); ++i )
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( selected_items.at( i ) );

      if ( item->type() == Smb4KNetworkBrowserItem::Share && !item->shareItem()->isPrinter() )
      {
        shares << item->shareItem();
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
    // No selected items. Just return.
    return;
  }

  if ( !shares.isEmpty() )
  {
    Smb4KBookmarkHandler::self()->addBookmarks( shares, m_widget );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotPreview( bool /*checked*/ )
{
  // Get the current item and pass the encapsulated Smb4KShare item
  // to the preview job.
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

  if ( item && !item->shareItem()->isPrinter() )
  {
    Smb4KPreviewer::self()->preview( item->shareItem(), m_widget );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotPrint( bool /*checked*/ )
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );
  
  if ( item->shareItem()->isPrinter() )
  {
    Smb4KPrint::self()->print( item->shareItem(), m_widget );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotMount( bool /*checked*/ )
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KMounter::self()->mountShare( item->shareItem() );
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


void Smb4KNetworkBrowserPart::slotScannerAboutToStart( Smb4KBasicNetworkItem *item, int process )
{
  switch ( process )
  {
    case Smb4KScanner::LookupDomains:
    {
      if ( !m_silent )
      {
        emit setStatusBarText( i18n( "Looking for workgroups and domains..." ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KScanner::LookupDomainMembers:
    {
      if ( !m_silent )
      {
        Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
        emit setStatusBarText( i18n( "Looking for hosts in domain %1..." ).arg( workgroup->workgroupName() ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KScanner::LookupShares:
    {
      if ( !m_silent )
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        emit setStatusBarText( i18n( "Looking for shares provided by host %1..." ).arg( host->hostName() ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KScanner::LookupInfo:
    {
      if ( !m_silent )
      {
        Smb4KHost *host = static_cast<Smb4KHost *>( item );
        emit setStatusBarText( i18n( "Looking for more information about host %1..." ).arg( host->hostName() ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      break;
    }
  }

  actionCollection()->action( "rescan_action" )->setEnabled( false );
  actionCollection()->action( "abort_action" )->setEnabled( true );
}


void Smb4KNetworkBrowserPart::slotScannerFinished( Smb4KBasicNetworkItem */*item*/, int /*process*/ )
{
  if ( !m_silent )
  {
    emit setStatusBarText( i18n( "Done." ) );
  }
  else
  {
    // Do nothing
  }

  actionCollection()->action( "rescan_action" )->setEnabled( true );
  actionCollection()->action( "abort_action" )->setEnabled( false );
}


void Smb4KNetworkBrowserPart::slotMounterAboutToStart( Smb4KShare */*share*/, int /*process*/ )
{
  // Do not change the state of the rescan action here, because it has
  // nothing to do with the mounter.
  actionCollection()->action( "abort_action" )->setEnabled( true );
}


void Smb4KNetworkBrowserPart::slotMounterFinished( Smb4KShare */*share*/, int /*process*/ )
{
  // Do not change the state of the rescan action here, because it has
  // nothing to do with the mounter.
  actionCollection()->action( "abort_action" )->setEnabled( false );
}


void Smb4KNetworkBrowserPart::slotShareMounted( Smb4KShare *share )
{
  QTreeWidgetItemIterator it( m_widget );
  
  while ( *it )
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( *it );
    
    if ( item->type() == Smb4KNetworkBrowserItem::Share )
    {
      if ( QString::compare( item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
      {
        item->update( share );
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserPart::slotShareUnmounted( Smb4KShare *share )
{
  QTreeWidgetItemIterator it( m_widget );
  
  while ( *it )
  {
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( *it );
    
    if ( item->type() == Smb4KNetworkBrowserItem::Share )
    {
      if ( QString::compare( item->shareItem()->unc(), share->unc(), Qt::CaseInsensitive ) == 0 )
      {
        item->update( share );
        break;
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
    
    ++it;
  }
}


void Smb4KNetworkBrowserPart::slotAboutToQuit()
{
  saveSettings();
}


void Smb4KNetworkBrowserPart::slotIconSizeChanged( int group )
{
  switch ( group )
  {
    case KIconLoader::Small:
    {
      int icon_size = KIconLoader::global()->currentSize( KIconLoader::Small );
      m_widget->setIconSize( QSize( icon_size, icon_size ) );
      break;
    }
    default:
    {
      break;
    }
  }
}


#include "smb4knetworkbrowser_part.moc"
