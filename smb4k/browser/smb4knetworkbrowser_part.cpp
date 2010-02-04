/***************************************************************************
    smb4knetworkbrowser_part  -  This Part encapsulates the network
    browser of Smb4K.
                             -------------------
    begin                : Fr Jan 5 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

// application specific includes
#include <smb4knetworkbrowser_part.h>
#include <smb4knetworkbrowser.h>
#include <smb4knetworkbrowseritem.h>
#include <smb4knetworkbrowsertooltip.h>
#include <../dialogs/smb4kcustomoptionsdialog.h>
#include <../dialogs/smb4kmountdialog.h>
#include <../dialogs/smb4kprintdialog.h>
#include <../dialogs/smb4kpreviewdialog.h>
#include <../dialogs/smb4kbookmarkdialog.h>
#include <core/smb4kcore.h>
#include <core/smb4kglobal.h>
#include <core/smb4ksettings.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kdefs.h>
#include <core/smb4kwalletmanager.h>
#include <core/smb4kauthinfo.h>

using namespace Smb4KGlobal;

typedef KParts::GenericFactory<Smb4KNetworkBrowserPart> Smb4KNetworkBrowserPartFactory;
K_EXPORT_COMPONENT_FACTORY( libsmb4knetworkbrowser, Smb4KNetworkBrowserPartFactory )


Smb4KNetworkBrowserPart::Smb4KNetworkBrowserPart( QWidget *parentWidget, QObject *parent, const QStringList &args )
: KParts::Part( parent ), m_bookmark_shortcut( true ), m_silent( false )
{
  // Parse arguments:
  for ( int i = 0; i < args.size(); ++i )
  {
    if ( args.at( i ).startsWith( "bookmark_shortcut" ) )
    {
      if ( QString::compare( args.at( i ).section( "=", 1, 1 ).trimmed(), "\"false\"" ) == 0 )
      {
        m_bookmark_shortcut = false;
      }
      else
      {
        // Do nothing
      }

      continue;
    }
    else if ( args.at( i ).startsWith( "silent" ) )
    {
      if ( QString::compare( args.at( i ).section( "=", 1, 1 ).trimmed(), "\"true\"" ) == 0 )
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

  connect( m_widget,               SIGNAL( itemExpanded( QTreeWidgetItem * ) ),
           this,                   SLOT( slotItemExpanded( QTreeWidgetItem * ) ) );

  connect( m_widget,               SIGNAL( itemCollapsed( QTreeWidgetItem * ) ),
           this,                   SLOT( slotItemCollapsed( QTreeWidgetItem * ) ) );

  connect( m_widget,               SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ),
           this,                   SLOT( slotItemExecuted( QTreeWidgetItem *, int ) ) );

  connect( m_widget->tooltip(),    SIGNAL( aboutToShow( Smb4KNetworkBrowserItem * ) ),
           this,                   SLOT( slotAboutToShowToolTip( Smb4KNetworkBrowserItem * ) ) );

  connect( m_widget->tooltip(),    SIGNAL( aboutToHide( Smb4KNetworkBrowserItem * ) ),
           this,                   SLOT( slotAboutToHideToolTip( Smb4KNetworkBrowserItem * ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( workgroups( const QList<Smb4KWorkgroup *> & ) ),
           this,                   SLOT( slotWorkgroups( const QList<Smb4KWorkgroup *> & ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( hosts( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ),
           this,                   SLOT( slotWorkgroupMembers( Smb4KWorkgroup *, const QList<Smb4KHost *> & ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( shares( Smb4KHost *, const QList<Smb4KShare *> & ) ),
           this,                   SLOT( slotShares( Smb4KHost *, const QList<Smb4KShare *> & ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( info( Smb4KHost * ) ),
           this,                   SLOT( slotAddInformation( Smb4KHost * ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( hostInserted( Smb4KHost * ) ),
           this,                   SLOT( slotInsertHost( Smb4KHost * ) ) );

  connect( Smb4KCore::ipScanner(), SIGNAL( ipAddress( Smb4KHost * ) ),
           this,                   SLOT( slotAddIPAddress( Smb4KHost * ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( aboutToStart( Smb4KBasicNetworkItem *, int ) ),
           this,                   SLOT( slotScannerAboutToStart( Smb4KBasicNetworkItem *, int ) ) );

  connect( Smb4KCore::scanner(),   SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
           this,                   SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );

  connect( Smb4KCore::mounter(),   SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                   SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );

  connect( Smb4KCore::mounter(),   SIGNAL( finished( Smb4KShare *, int ) ),
           this,                   SLOT( slotMounterFinished( Smb4KShare *, int ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( mounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareMounted( Smb4KShare * ) ) );

  connect( Smb4KMounter::self(),   SIGNAL( unmounted( Smb4KShare * ) ),
           this,                   SLOT( slotShareUnmounted( Smb4KShare * ) ) );

  connect( kapp,                   SIGNAL( aboutToQuit() ),
           this,                   SLOT( slotAboutToQuit() ) );
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

  KAction *manual_action   = new KAction( KIcon( "list-add" ), i18n( "M&ount Manually" ),
                             actionCollection() );
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

  KAction *mount_action    = new KAction( KIcon( "folder-remote" ), i18n( "&Mount" ),
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
  for ( int i = 0; i < mountedSharesList()->size(); ++i )
  {
    // We do not need to use slotShareUnmounted() here, too,
    // because slotShareMounted() will take care of everything
    // we need here.
    slotShareMounted( mountedSharesList()->at( i ) );
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
                          "2.0",
                          ki18n( "The network browser KPart of Smb4K" ),
                          KAboutData::License_GPL_V2,
                          ki18n( "\u00A9 2007-2009, Alexander Reinholdt" ),
                          KLocalizedString(),
                          "http://smb4k.berlios.de",
                          "smb4k-bugs@lists.berlios.de" );

  return aboutData;
}


void Smb4KNetworkBrowserPart::customEvent( QEvent *e )
{
  switch ( e->type() )
  {
    case EVENT_LOAD_SETTINGS:
    {
      loadSettings();

      break;
    }
    case EVENT_SET_FOCUS:
    {
      if ( m_widget->topLevelItemCount() != 0 )
      {
        kDebug() << "Do we need to port the selection stuff?" << endl;
      }

      m_widget->setFocus( Qt::OtherFocusReason );
      break;
    }
    case EVENT_SCAN_NETWORK:
    {
      slotRescan( false ); // boolean is ignored
      break;
    }
    case EVENT_ADD_BOOKMARK:
    {
      slotAddBookmark( false );
      break;
    }
    default:
    {
      break;
    }
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
            Smb4KPrintDialog *dlg = m_widget->findChild<Smb4KPrintDialog *>( "PrintDialog_"+browser_item->shareItem()->unc() );

            actionCollection()->action( "bookmark_action" )->setEnabled( false );
            actionCollection()->action( "preview_action" )->setEnabled( false );
            actionCollection()->action( "mount_action" )->setEnabled( false );
            actionCollection()->action( "print_action" )->setEnabled( !dlg );
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
            Smb4KPrintDialog *dlg = m_widget->findChild<Smb4KPrintDialog *>( "PrintDialog_"+browser_item->shareItem()->unc() );
            actionCollection()->action( "print_action" )->setEnabled( !dlg );
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


void Smb4KNetworkBrowserPart::slotItemExpanded( QTreeWidgetItem *item )
{
  Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>( item );

  if ( browserItem )
  {
    switch ( browserItem->type() )
    {
      case Smb4KNetworkBrowserItem::Workgroup:
      {
        Smb4KCore::scanner()->lookupDomainMembers( browserItem->workgroupItem() );
        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        Smb4KCore::scanner()->lookupShares( browserItem->hostItem() );
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


void Smb4KNetworkBrowserPart::slotItemCollapsed( QTreeWidgetItem *item )
{
  Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>( item );

  // Remove all children if we have a host item, because we
  // do not want shares already displayed before the user provided
  // the login data.
  if ( browserItem )
  {
    switch ( browserItem->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        QList<QTreeWidgetItem *> children = browserItem->takeChildren();

        while ( children.size() != 0 )
        {
          delete children.takeFirst();
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


void Smb4KNetworkBrowserPart::slotItemExecuted( QTreeWidgetItem *item, int /*column*/ )
{
  Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>( item );

  if ( browserItem )
  {
    switch ( browserItem->type() )
    {
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


void Smb4KNetworkBrowserPart::slotAboutToShowToolTip( Smb4KNetworkBrowserItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        // Check if additional information is needed and send a request to the scanner,
        // if necessary.
        if ( !item->hostItem()->infoChecked() )
        {
          Smb4KScanner::self()->lookupInfo( item->hostItem() );
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


void Smb4KNetworkBrowserPart::slotAboutToHideToolTip( Smb4KNetworkBrowserItem *item )
{
  if ( item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        // Kill the lookup process for the additional information
        // and nothing else.
        if ( Smb4KScanner::self()->isRunning( item->hostItem(), Smb4KScanner::LookupInfo ) )
        {
          Smb4KScanner::self()->abort( item->hostItem(), Smb4KScanner::LookupInfo );
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
      if ( hostsList()->isEmpty() )
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
      for ( int i = 0; i < workgroups.size(); ++i )
      {
        Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( workgroups.at( i ) );

        // Remove obsolete hosts from the workgroup.
        for ( int j = 0; j < workgroups.at( i )->childCount(); ++j )
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

        // If the workgroup item has children, expand it. Otherwise collapse it.
        if ( workgroups.at( i )->childCount() != 0 )
        {
          m_widget->expandItem( workgroups.at( i ) );
        }
        else
        {
          m_widget->collapseItem( workgroups.at( i ) );
        }

        // Sort the items.
        m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );
      }
    }
    else
    {
      // The workgroup(s) has/have no children (anymore).
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
      // We will run through all items in the tree widget to find the
      // obsolete hosts.
      QTreeWidgetItemIterator it( m_widget );

      while ( *it )
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( *it );
        bool found_host = false;

        switch ( item->type() )
        {
          case Smb4KNetworkBrowserItem::Host:
          {
            for ( int i = 0; i < list.size(); ++i )
            {
              if ( QString::compare( list.at( i )->workgroupName(), item->hostItem()->workgroupName() ) == 0 &&
                   QString::compare( list.at( i )->hostName(), item->hostItem()->hostName() ) == 0 )
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

      // Find the hosts in the tree widget.
      for ( int i = 0; i < list.size(); ++i )
      {
        QList<QTreeWidgetItem *> hosts = m_widget->findItems( list.at( i )->hostName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network );

        if ( !hosts.isEmpty() )
        {
          for ( int j = 0; j < hosts.size(); ++j )
          {
            Smb4KNetworkBrowserItem *host_item = static_cast<Smb4KNetworkBrowserItem *>( hosts.at( j ) );

            if ( QString::compare( list.at( i )->workgroupName(), host_item->hostItem()->workgroupName(), Qt::CaseInsensitive ) == 0 )
            {
              // This is the host. Update it.
              host_item->update( list.at( i ) );
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
          // The host is new. Add it to the list widget and create the
          // workgroup item as well if needed.
          QList<QTreeWidgetItem *> workgroups = m_widget->findItems( list.at( i )->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network );

          if ( !workgroups.isEmpty() )
          {
            for ( int j = 0; j < workgroups.size(); ++j )
            {
              Smb4KNetworkBrowserItem *workgroup_item = static_cast<Smb4KNetworkBrowserItem *>( workgroups.at( j ) );

              if ( workgroup_item )
              {
                // FIXME: Do we need to change the (pseudo)  master browser here?
                (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );
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
              workgroup.setMasterBrowser( list.at( i )->hostName(), list.at( i )->ip(), !list.at( i )->isMasterBrowser() /*pseudo master?*/ );
            }
            else
            {
              // Do nothing
            }

            Smb4KNetworkBrowserItem *workgroup_item = new Smb4KNetworkBrowserItem( m_widget, &workgroup );
            (void) new Smb4KNetworkBrowserItem( workgroup_item, list.at( i ) );


            continue;
          }
        }
      }

      // If the workgroup item has children, expand it. Otherwise collapse it.
      for ( int i = 0; i < m_widget->topLevelItemCount(); ++i )
      {
        if ( m_widget->topLevelItem( i )->childCount() != 0 )
        {
          m_widget->expandItem( m_widget->topLevelItem( i ) );
        }
        else
        {
          m_widget->collapseItem( m_widget->topLevelItem( i ) );
        }
      }

      // Sort the items.
      m_widget->sortItems( Smb4KNetworkBrowser::Network, Qt::AscendingOrder );
    }
    else
    {
      while ( m_widget->topLevelItemCount() != 0 )
      {
        delete m_widget->takeTopLevelItem( 0 );
      }
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
          // Expand the host item, if it is collapsed.
          if ( !network_item->isExpanded() )
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

            // If the tool tip is currently shown, update it.
          if ( m_widget->tooltip() && m_widget->tooltip()->isVisible() &&
               (m_widget->tooltip()->item() == hostItem || m_widget->tooltip()->item() == workgroupItem) )
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
             m_widget->tooltip()->item() == hostItem )
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


void Smb4KNetworkBrowserPart::slotInsertHost( Smb4KHost *host )
{
  if ( host )
  {
    Smb4KNetworkBrowserItem *workgroupItem = NULL;

    // Get the correct workgroup.
    QList<QTreeWidgetItem *> items = m_widget->findItems( host->workgroupName(), Qt::MatchFixedString,
                                     Smb4KNetworkBrowser::Network );

    for ( int i = 0; i < items.size(); ++i )
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( items.at( i ) );

      switch ( item->type() )
      {
        case Smb4KNetworkBrowserItem::Workgroup:
        {
          workgroupItem = item;
          break;
        }
        default:
        {
          break;
        }
      }
    }

    if ( workgroupItem )
    {
      // Check all children of this workgroup if the host is already
      // there.
      Smb4KNetworkBrowserItem *hostItem = NULL;

      for ( int i = 0; i < workgroupItem->childCount(); ++i )
      {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( workgroupItem->child( i ) );

        switch( item->type() )
        {
          case Smb4KNetworkBrowserItem::Host:
          {
            if ( QString::compare( host->hostName(), item->hostItem()->workgroupName() ) == 0 )
            {
              hostItem = item;
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

      if ( !hostItem )
      {
        // The host item is not in the list. Add it.
        (void) new Smb4KNetworkBrowserItem( workgroupItem, host );
      }
      else
      {
        // Do nothing. The host is already there.
      }
    }
    else
    {
      // The workgroup is not present. Add it.
      Smb4KWorkgroup *workgroup = findWorkgroup( host->workgroupName() );

      if ( workgroup )
      {
        workgroupItem = new Smb4KNetworkBrowserItem( m_widget, workgroup );

        (void) new Smb4KNetworkBrowserItem( workgroupItem, host );
      }
      else
      {
        // Do nothing (This should never happen...)
      }
    }
  }
  else
  {
    // Do nothing
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
        Smb4KCore::scanner()->lookupDomainMembers( item->workgroupItem() );
        break;
      }
      case Smb4KNetworkBrowserItem::Host:
      {
        Smb4KCore::scanner()->lookupShares( item->hostItem() );
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KNetworkBrowserItem *parentItem = static_cast<Smb4KNetworkBrowserItem *>( item->parent() );
        Smb4KCore::scanner()->lookupShares( parentItem->hostItem() );
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
    Smb4KCore::scanner()->lookupDomains();
  }
}


void Smb4KNetworkBrowserPart::slotAbort( bool /*checked*/ )
{
  if ( Smb4KCore::scanner()->isRunning() )
  {
    Smb4KCore::scanner()->abortAll();
  }
  else
  {
    // Do nothing
  }

  if ( Smb4KCore::mounter()->isRunning() &&
       Smb4KCore::mounter()->currentState() != MOUNTER_UNMOUNT )
  {
    Smb4KCore::mounter()->abortAll();
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotMountManually( bool /*checked*/ )
{
  Smb4KMountDialog *dlg = m_widget->findChild<Smb4KMountDialog *>();

  if ( !dlg )
  {
    dlg = new Smb4KMountDialog( m_widget );
  }
  else
  {
    // Do nothing
  }

  if ( !dlg->isVisible() )
  {
    dlg->setVisible( true );
  }
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
        Smb4KAuthInfo authInfo( item->hostItem() );
        Smb4KWalletManager::self()->showPasswordDialog( &authInfo, m_widget );
        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        Smb4KAuthInfo authInfo( item->shareItem() );
        Smb4KWalletManager::self()->showPasswordDialog( &authInfo, m_widget );
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
  Smb4KCustomOptionsDialog *dlg = m_widget->findChild<Smb4KCustomOptionsDialog *>();
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );

  if ( !dlg && item )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Host:
      {
        dlg = new Smb4KCustomOptionsDialog( item->hostItem(), m_widget );

        break;
      }
      case Smb4KNetworkBrowserItem::Share:
      {
        dlg = new Smb4KCustomOptionsDialog( item->shareItem(), m_widget );

        break;
      }
      default:
      {
        break;
      }
    }
  }

  if ( dlg && !dlg->isVisible() )
  {
    if ( dlg->isInitialized() )
    {
      dlg->setVisible( true );
    }
    else
    {
      delete dlg;
    }
  }
}


void Smb4KNetworkBrowserPart::slotAddBookmark( bool /*checked*/ )
{
  Smb4KBookmarkDialog *dlg = m_widget->findChild<Smb4KBookmarkDialog *>();
  QList<QTreeWidgetItem *> selected_items = m_widget->selectedItems();
  QList<Smb4KBookmark *> bookmarks;

  if ( !selected_items.isEmpty() )
  {
    for ( int i = 0; i < selected_items.size(); ++i )
    {
      Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( selected_items.at( i ) );

      if ( item->type() == Smb4KNetworkBrowserItem::Share && !item->shareItem()->isPrinter() )
      {
        bookmarks << new Smb4KBookmark( item->shareItem() );
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

  if ( !bookmarks.isEmpty() )
  {
    if ( !dlg )
    {
      dlg = new Smb4KBookmarkDialog( m_widget );
    }
    else
    {
      // Do nothing
    }

    dlg->setBookmarks( bookmarks );
    dlg->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotPreview( bool /*checked*/ )
{
  // The user should be able to open several dialogs at a time, so
  // do not check for existing dialogs and use show() here.
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );
  Smb4KPreviewDialog *dlg = NULL;

  if ( item && !item->shareItem()->isPrinter() )
  {
    switch ( item->type() )
    {
      case Smb4KNetworkBrowserItem::Share:
      {
        dlg = new Smb4KPreviewDialog( item->shareItem(), m_widget );

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

  if ( dlg && !dlg->isVisible() )
  {
    dlg->getPreview();
    dlg->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotPrint( bool /*checked*/ )
{
  Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>( m_widget->currentItem() );
  Smb4KPrintDialog *dlg = m_widget->findChild<Smb4KPrintDialog *>( "PrintDialog_"+item->shareItem()->unc() );

  if ( !dlg && item )
  {
    switch( item->type() )
    {
      case Smb4KNetworkBrowserItem::Share:
      {
        if ( item->shareItem()->isPrinter() )
        {
          dlg = new Smb4KPrintDialog( item->shareItem(), m_widget );
          dlg->setObjectName( "PrintDialog_"+item->shareItem()->unc() );
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
    // Do nothing
  }

  if ( dlg && !dlg->isVisible() )
  {
    dlg->setVisible( true );
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
        Smb4KCore::mounter()->mountShare( item->shareItem() );

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
  QList<QTreeWidgetItem *> list = m_widget->findItems( share->shareName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network );

  if ( !list.isEmpty() )
  {
    for ( int i = 0; i < list.size(); ++i )
    {
      Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>( list.at( i ) );

      if ( browser_item->type() == Smb4KNetworkBrowserItem::Share &&
           QString::compare( browser_item->shareItem()->workgroupName(), share->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( browser_item->shareItem()->hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 )
      {
        if ( !share->isForeign() || Smb4KSettings::showAllShares() )
        {
          browser_item->setMounted( share, Smb4KNetworkBrowserItem::Mounted );
          break;
        }
        else if ( share->isForeign() && !Smb4KSettings::showAllShares() )
        {
          browser_item->setMounted( browser_item->shareItem(), Smb4KNetworkBrowserItem::NotMounted );
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
  }
  else
  {
    // Do nothing
  }
}


void Smb4KNetworkBrowserPart::slotShareUnmounted( Smb4KShare *share )
{
  QList<QTreeWidgetItem *> list = m_widget->findItems( share->shareName(), Qt::MatchFixedString|Qt::MatchRecursive, Smb4KNetworkBrowser::Network );

  if ( !list.isEmpty() )
  {
    for ( int i = 0; i < list.size(); ++i )
    {
      Smb4KNetworkBrowserItem *browser_item = static_cast<Smb4KNetworkBrowserItem *>( list.at( i ) );

      if ( browser_item->type() == Smb4KNetworkBrowserItem::Share &&
           QString::compare( browser_item->shareItem()->workgroupName(), share->workgroupName(), Qt::CaseInsensitive ) == 0 &&
           QString::compare( browser_item->shareItem()->hostName(), share->hostName(), Qt::CaseInsensitive ) == 0 )
      {
        // Check if we really have to unmark the share.
        QList<Smb4KShare *> mounted_shares = findShareByUNC( browser_item->shareItem()->unc() );

        if ( !mounted_shares.isEmpty() )
        {
          bool have_only_foreign = true;

          for ( int j = 0; j < mounted_shares.size(); ++j )
          {
            if ( !mounted_shares.at( j )->isForeign() )
            {
              have_only_foreign = false;
              break;
            }
            else
            {
              continue;
            }
          }
          
          if ( !have_only_foreign )
          {
            browser_item->setMounted( browser_item->shareItem(), Smb4KNetworkBrowserItem::NotMounted );
          }
          else
          {
            if ( !Smb4KSettings::showAllShares() )
            {
              browser_item->setMounted( browser_item->shareItem(), Smb4KNetworkBrowserItem::NotMounted );
            }
            else
            {
              // Do nothing
            }
          }
          break;
        }
        else
        {
          browser_item->setMounted( browser_item->shareItem(), Smb4KNetworkBrowserItem::NotMounted );
          break;
        }
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


void Smb4KNetworkBrowserPart::slotAboutToQuit()
{
  saveSettings();
}

#include "smb4knetworkbrowser_part.moc"
