/***************************************************************************
    smb4ksharesview_part  -  This Part includes the shares view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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
#include <QHeaderView>
#include <QDropEvent>

// KDE includes
#include <klocale.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kaction.h>
#include <kdebug.h>
#include <kicon.h>
#include <kactioncollection.h>
#include <kmenu.h>
#include <kapplication.h>
#include <kurl.h>
#include <kio/copyjob.h>
#include <kjobuidelegate.h>
#include <kstandarddirs.h>

// application specific includes
#include <smb4ksharesview_part.h>
#include <smb4kshareslistviewitem.h>
#include <smb4ksharesiconviewitem.h>
#include <../dialogs/smb4ksynchronizationdialog.h>
#include <../dialogs/smb4kbookmarkdialog.h>
#include <core/smb4kshare.h>
#include <core/smb4kcore.h>
#include <core/smb4ksettings.h>
#include <core/smb4kglobal.h>
#include <core/smb4kdefs.h>
#include <core/smb4kmounter.h>

using namespace Smb4KGlobal;

K_PLUGIN_FACTORY( Smb4KSharesViewPartFactory, registerPlugin<Smb4KSharesViewPart>(); )
K_EXPORT_PLUGIN( Smb4KSharesViewPartFactory( "Smb4KSharesViewPart" ) );


Smb4KSharesViewPart::Smb4KSharesViewPart( QWidget *parentWidget, QObject *parent, const QList<QVariant> &args )
: KParts::Part( parent ), m_mode( IconMode ), m_bookmark_shortcut( true ), m_silent( false )
{
  // Parse the arguments.
  for ( int i = 0; i < args.size(); ++i )
  {
    if ( args.at( i ).toString().startsWith( "viewmode" ) )
    {
      if ( QString::compare( args.at( i ).toString().section( "=", 1, 1 ).trimmed(), "list" ) == 0 )
      {
        m_mode = ListMode;
      }
      else
      {
        // Do nothing
      }

      continue;
    }
    else if ( args.at( i ).toString().startsWith( "bookmark_shortcut" ) )
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

      continue;
    }
    else
    {
      continue;
    }
  }

  // Set the XML file:
  setXMLFile( "smb4ksharesview_part.rc" );

  // Set the container widget and its layout.
  m_container = new QWidget( parentWidget );

  m_layout = new QGridLayout( m_container );
  m_layout->setMargin( 0 );
  m_layout->setSpacing( 0 );

  setWidget( m_container );

  // Set up the actual view.
  m_icon_view = NULL;
  m_list_view = NULL;

  setupView();

  // Set up the actions.
  // Do not put this before setWidget() or the shortcuts
  // will not be shown.
  setupActions();

  // Load settings:
  loadSettings();

  // Add some connections:
  connect( Smb4KMounter::self(), SIGNAL( mounted( Smb4KShare * ) ),
           this,                 SLOT( slotShareMounted( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(), SIGNAL( unmounted( Smb4KShare * ) ),
           this,                 SLOT( slotShareUnmounted( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(), SIGNAL( updated( Smb4KShare *) ),
           this,                 SLOT( slotShareUpdated( Smb4KShare * ) ) );

  connect( Smb4KMounter::self(), SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );

  connect( Smb4KMounter::self(), SIGNAL( finished( Smb4KShare *, int ) ),
           this,                 SLOT( slotMounterFinished( Smb4KShare *, int ) ) );

  connect( kapp,                 SIGNAL( aboutToQuit() ),
           this,                 SLOT( slotAboutToQuit() ) );
}


Smb4KSharesViewPart::~Smb4KSharesViewPart()
{
}


void Smb4KSharesViewPart::setupView()
{
  // First disconnect the signals, so that we do not
  // get multiple connections.
  if ( m_icon_view )
  {
    disconnect( m_icon_view, SIGNAL( customContextMenuRequested( const QPoint & ) ),
                this,        SLOT( slotContextMenuRequested( const QPoint & ) ) );

    disconnect( m_icon_view, SIGNAL( itemSelectionChanged() ),
                this,        SLOT( slotItemSelectionChanged() ) );

    disconnect( m_icon_view, SIGNAL( itemPressed( QListWidgetItem * ) ),
               this,        SLOT( slotItemPressed( QListWidgetItem * ) ) );

    disconnect( m_icon_view, SIGNAL( itemExecuted( QListWidgetItem * ) ),
                this,        SLOT( slotItemExecuted( QListWidgetItem * ) ) );

    disconnect( m_icon_view, SIGNAL( acceptedDropEvent( Smb4KSharesIconViewItem *, QDropEvent * ) ),
                this,        SLOT( slotIconViewDropEvent( Smb4KSharesIconViewItem *, QDropEvent * ) ) );
  }
  else
  {
    // Do nothing
  }

  if ( m_list_view )
  {
    disconnect( m_list_view, SIGNAL( customContextMenuRequested( const QPoint & ) ),
                this,        SLOT( slotContextMenuRequested( const QPoint & ) ) );

    disconnect( m_list_view, SIGNAL( itemSelectionChanged() ),
                this,        SLOT( slotItemSelectionChanged() ) );

    disconnect( m_list_view, SIGNAL( itemPressed( QTreeWidgetItem *, int ) ),
                this,        SLOT( slotItemPressed( QTreeWidgetItem *, int ) ) );

    disconnect( m_list_view, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ),
                this,        SLOT( slotItemExecuted( QTreeWidgetItem *, int ) ) );

    disconnect( m_list_view, SIGNAL( acceptedDropEvent( Smb4KSharesListViewItem *, QDropEvent * ) ),
                this,        SLOT( slotListViewDropEvent( Smb4KSharesListViewItem *, QDropEvent * ) ) );
  }
  else
  {
    // Do nothing
  }

  // Set the widget of this part:
  switch ( m_mode )
  {
    case IconMode:
    {
      if ( m_list_view )
      {
        delete m_list_view;
        m_list_view = NULL;
      }
      else
      {
        // Do nothing
      }

      if ( !m_icon_view )
      {
        m_icon_view = new Smb4KSharesIconView( m_container );
        m_layout->addWidget( m_icon_view, 0, 0, 0 );
        m_icon_view->setVisible( true );
        m_container->setFocusProxy( m_icon_view );
      }
      else
      {
        // Do nothing
      }

      connect( m_icon_view, SIGNAL( customContextMenuRequested( const QPoint & ) ),
               this,        SLOT( slotContextMenuRequested( const QPoint & ) ) );

      connect( m_icon_view, SIGNAL( itemSelectionChanged() ),
               this,        SLOT( slotItemSelectionChanged() ) );

      connect( m_icon_view, SIGNAL( itemPressed( QListWidgetItem * ) ),
               this,        SLOT( slotItemPressed( QListWidgetItem * ) ) );

      connect( m_icon_view, SIGNAL( itemExecuted( QListWidgetItem * ) ),
               this,        SLOT( slotItemExecuted( QListWidgetItem * ) ) );

      connect( m_icon_view, SIGNAL( acceptedDropEvent( Smb4KSharesIconViewItem *, QDropEvent * ) ),
               this,        SLOT( slotIconViewDropEvent( Smb4KSharesIconViewItem *, QDropEvent * ) ) );

      break;
    }
    case ListMode:
    {
      if ( m_icon_view )
      {
        delete m_icon_view;
        m_icon_view = NULL;
      }
      else
      {
        // Do nothing
      }

      if ( !m_list_view )
      {
        m_list_view = new Smb4KSharesListView( m_container );
        m_layout->addWidget( m_list_view, 0, 0, 0 );
        m_list_view->setVisible( true );
        m_container->setFocusProxy( m_list_view );
      }
      else
      {
        // Do nothing
      }

      connect( m_list_view, SIGNAL( customContextMenuRequested( const QPoint & ) ),
               this,        SLOT( slotContextMenuRequested( const QPoint & ) ) );

      connect( m_list_view, SIGNAL( itemSelectionChanged() ),
               this,        SLOT( slotItemSelectionChanged() ) );

      connect( m_list_view, SIGNAL( itemPressed( QTreeWidgetItem *, int ) ),
               this,        SLOT( slotItemPressed( QTreeWidgetItem *, int ) ) );

      connect( m_list_view, SIGNAL( itemExecuted( QTreeWidgetItem *, int ) ),
               this,        SLOT( slotItemExecuted( QTreeWidgetItem *, int ) ) );

      connect( m_list_view, SIGNAL( acceptedDropEvent( Smb4KSharesListViewItem *, QDropEvent * ) ),
               this,        SLOT( slotListViewDropEvent( Smb4KSharesListViewItem *, QDropEvent * ) ) );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::setupActions()
{
  KAction *unmount_action     = new KAction( KIcon( "media-eject" ), i18n( "&Unmount" ),
                                actionCollection() );
  unmount_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_U ) );
  connect( unmount_action, SIGNAL( triggered( bool ) ), this, SLOT( slotUnmountShare( bool ) ) );

#ifdef __linux__
  KAction *force_action       = new KAction( KIcon( "media-eject" ), i18n( "&Force Unmounting" ),
                                actionCollection() );
  force_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_F ) );
  connect( force_action, SIGNAL( triggered( bool ) ), this, SLOT( slotForceUnmountShare( bool ) ) );
#endif

  KAction *unmount_all_action = new KAction( KIcon( "system-run" ), i18n( "U&nmount All" ),
                                actionCollection() );
  unmount_all_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_N ) );
  connect( unmount_all_action, SIGNAL( triggered( bool ) ), this, SLOT( slotUnmountAllShares( bool ) ) );

  KAction *synchronize_action = new KAction( KIcon( "go-bottom" ), i18n( "S&ynchronize" ),
                                actionCollection() );
  synchronize_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_Y ) );
  connect( synchronize_action, SIGNAL( triggered( bool ) ), this, SLOT( slotSynchronize( bool ) ) );

  KAction *konsole_action     = new KAction( KIcon( "utilities-terminal" ), i18n( "Open with Konso&le" ),
                                actionCollection() );
  konsole_action->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_L ) );
  connect( konsole_action, SIGNAL( triggered( bool ) ), this, SLOT( slotKonsole( bool ) ) );

  KAction *filemanager_action = new KAction( KIcon( "system-file-manager" ), i18n( "Open with F&ile Manager" ),
                                actionCollection() );
  QList<QKeySequence> shortcuts;
  shortcuts.append( QKeySequence( Qt::CTRL+Qt::Key_I ) );
  shortcuts.append( QKeySequence( Qt::CTRL+Qt::Key_K ) );  // legacy shortcut
  filemanager_action->setShortcuts( shortcuts );
  connect( filemanager_action, SIGNAL( triggered( bool ) ), this, SLOT( slotFileManager( bool ) ) );

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

  actionCollection()->addAction( "unmount_action", unmount_action );
#ifdef __linux__
  actionCollection()->addAction( "force_unmount_action", force_action );
#endif
  actionCollection()->addAction( "unmount_all_action", unmount_all_action );
  actionCollection()->addAction( "bookmark_action", bookmark_action );
  actionCollection()->addAction( "synchronize_action", synchronize_action );
  actionCollection()->addAction( "konsole_action", konsole_action );
  actionCollection()->addAction( "filemanager_action", filemanager_action );

  // Disable all actions for now:
  unmount_action->setEnabled( false );
#ifdef __linux__
  force_action->setEnabled( false );
#endif
  unmount_all_action->setEnabled( false );
  bookmark_action->setEnabled( false );
  synchronize_action->setEnabled( false );
  konsole_action->setEnabled( false );
  filemanager_action->setEnabled( false );

  // Insert the actions into the menu:
  m_menu = new KActionMenu( this );
  m_menu_title = m_menu->menu()->addTitle( KIcon( "folder-remote" ), i18n( "Shares" ) );
  m_menu->addAction( unmount_action );
#ifdef __linux__
  m_menu->addAction( force_action );
#endif
  m_menu->addAction( unmount_all_action );
  m_menu->addSeparator();
  m_menu->addAction( bookmark_action );
  m_menu->addAction( synchronize_action );
  m_menu->addSeparator();
  m_menu->addAction( konsole_action );
  m_menu->addAction( filemanager_action );
}


void Smb4KSharesViewPart::loadSettings()
{
  if ( Smb4KSettings::sharesIconView() )
  {
    m_mode = IconMode;

    setupView();

    // Change the text of the share (first column):
    Smb4KSharesIconViewItem *item = NULL;

    for ( int i = 0; i < m_icon_view->count(); ++i )
    {
      item = static_cast<Smb4KSharesIconViewItem *>( m_icon_view->item( i ) );

      item->setShowMountPoint( Smb4KSettings::showMountPoint() );
    }
  }
  else
  {
    m_mode = ListMode;

    setupView();

    m_list_view->setColumnHidden( Smb4KSharesListView::Owner, !Smb4KSettings::showOwner() );
#ifndef __FreeBSD__
    m_list_view->setColumnHidden( Smb4KSharesListView::Login, !Smb4KSettings::showLoginName() );
#endif
    m_list_view->setColumnHidden( Smb4KSharesListView::FileSystem, !Smb4KSettings::showFileSystem() );
    m_list_view->setColumnHidden( Smb4KSharesListView::Free, !Smb4KSettings::showFreeDiskSpace() );
    m_list_view->setColumnHidden( Smb4KSharesListView::Used, !Smb4KSettings::showUsedDiskSpace() );
    m_list_view->setColumnHidden( Smb4KSharesListView::Total, !Smb4KSettings::showTotalDiskSpace() );
    m_list_view->setColumnHidden( Smb4KSharesListView::Usage, !Smb4KSettings::showDiskUsage() );

    // Change the text of the share (first column):
    Smb4KSharesListViewItem *item = NULL;

    for ( int i = 0; i < m_list_view->topLevelItemCount(); ++i )
    {
      item = static_cast<Smb4KSharesListViewItem *>( m_list_view->topLevelItem( i ) );

      item->setShowMountPoint( Smb4KSettings::showMountPoint() );
    }

    // Load and apply the positions of the columns.
    KConfigGroup configGroup( Smb4KSettings::self()->config(), "SharesViewPart" );

    QMap<int, int> map;
    map.insert( configGroup.readEntry( "ColumnPositionItem", (int)Smb4KSharesListView::Item ), Smb4KSharesListView::Item );
#ifndef __FreeBSD__
    map.insert( configGroup.readEntry( "ColumnPositionLogin", (int)Smb4KSharesListView::Login ), Smb4KSharesListView::Login );
#endif
    map.insert( configGroup.readEntry( "ColumnPositionFileSystem", (int)Smb4KSharesListView::FileSystem ), Smb4KSharesListView::FileSystem );
    map.insert( configGroup.readEntry( "ColumnPositionOwner", (int)Smb4KSharesListView::Owner ), Smb4KSharesListView::Owner );
    map.insert( configGroup.readEntry( "ColumnPositionFree", (int)Smb4KSharesListView::Free ), Smb4KSharesListView::Free );
    map.insert( configGroup.readEntry( "ColumnPositionUsed", (int)Smb4KSharesListView::Used ), Smb4KSharesListView::Used );
    map.insert( configGroup.readEntry( "ColumnPositionTotal", (int)Smb4KSharesListView::Total ), Smb4KSharesListView::Total );
    map.insert( configGroup.readEntry( "ColumnPositionUsage", (int)Smb4KSharesListView::Usage ), Smb4KSharesListView::Usage );

    QMap<int, int>::const_iterator it = map.constBegin();

    while ( it != map.constEnd() )
    {
      if ( it.key() != m_list_view->header()->visualIndex( it.value() ) )
      {
        m_list_view->header()->moveSection( m_list_view->header()->visualIndex( it.value() ), it.key() );
      }
      else
      {
        // Do nothing
      }

      ++it;
    }
  }

#ifdef __linux__
  if ( Smb4KSettings::sharesIconView() )
  {
    Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( m_icon_view->currentItem() );

    actionCollection()->action( "force_unmount_action" )->setEnabled( Smb4KSettings::useForceUnmount() &&
    (item && (!item->itemData()->share()->isForeign() || Smb4KSettings::unmountForeignShares())) );
  }
  else
  {
    Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( m_list_view->currentItem() );

    actionCollection()->action( "force_unmount_action" )->setEnabled( Smb4KSettings::useForceUnmount() &&
    (item && (!item->itemData()->share()->isForeign() || Smb4KSettings::unmountForeignShares())) );
  }
#endif


  // The rest of the settings will be applied on the fly.
}


void Smb4KSharesViewPart::saveSettings()
{
  switch ( m_mode )
  {
    case IconMode:
    {
      break;
    }
    case ListMode:
    {
      // Save the position of the columns.
      KConfigGroup configGroup( Smb4KSettings::self()->config(), "SharesViewPart" );

      configGroup.writeEntry( "ColumnPositionItem", m_list_view->header()->visualIndex( Smb4KSharesListView::Item ) );
#ifndef __FreeBSD__
      configGroup.writeEntry( "ColumnPositionLogin", m_list_view->header()->visualIndex( Smb4KSharesListView::Login ) );
#endif
      configGroup.writeEntry( "ColumnPositionFileSystem", m_list_view->header()->visualIndex( Smb4KSharesListView::FileSystem ) );
      configGroup.writeEntry( "ColumnPositionOwner", m_list_view->header()->visualIndex( Smb4KSharesListView::Owner ) );
      configGroup.writeEntry( "ColumnPositionFree", m_list_view->header()->visualIndex( Smb4KSharesListView::Free ) );
      configGroup.writeEntry( "ColumnPositionUsed", m_list_view->header()->visualIndex( Smb4KSharesListView::Used ) );
      configGroup.writeEntry( "ColumnPositionTotal", m_list_view->header()->visualIndex( Smb4KSharesListView::Total ) );
      configGroup.writeEntry( "ColumnPositionUsage", m_list_view->header()->visualIndex( Smb4KSharesListView::Usage ) );

      configGroup.sync();

      break;
    }
    default:
    {
      break;
    }
  }
}


KAboutData *Smb4KSharesViewPart::createAboutData()
{
  KAboutData *aboutData = new KAboutData( "smb4ksharesviewpart",
                          "smb4k",
                          ki18n( "Smb4KSharesViewPart" ),
                          "2.0",
                          ki18n( "The shares view KPart of Smb4K" ),
                          KAboutData::License_GPL_V2,
                          ki18n( "\u00A9 2007-2009, Alexander Reinholdt" ),
                          KLocalizedString(),
                          "http://smb4k.berlios.de",
                          "smb4k-bugs@lists.berlios.de" );

  return aboutData;
}


void Smb4KSharesViewPart::customEvent( QEvent *e )
{
  switch ( e->type() )
  {
    case EVENT_LOAD_SETTINGS:
    {
      // Before we reread the settings, let's save
      // widget specific things.
      saveSettings();

      // Load settings.
      loadSettings();

      // (Re-)load the list of shares.
      switch ( m_mode )
      {
        case IconMode:
        {
          while ( m_icon_view->count() != 0 )
          {
            delete m_icon_view->takeItem( 0 );
          }
          
          break;
        }
        case ListMode:
        {
          while ( m_list_view->topLevelItemCount() != 0 )
          {
            delete m_list_view->takeTopLevelItem( 0 );
          }
          
          break;
        }
        default:
        {
          break;
        }
      }
      
      for ( int i = 0; i < mountedSharesList().size(); ++i )
      {
        slotShareMounted( mountedSharesList().at( i ) );
      }

      break;
    }
    case EVENT_SET_FOCUS:
    {
      switch ( m_mode )
      {
        case IconMode:
        {
          if ( m_icon_view->count() != 0 )
          {
            kDebug() << "Do we need to port the selection stuff?" << endl;
          }
          else
          {
            // Do nothing
          }

          m_icon_view->setFocus( Qt::OtherFocusReason );

          break;
        }
        case ListMode:
        {
          if ( m_list_view->topLevelItemCount() != 0 )
          {
            kDebug() << "Do we need to port the selection stuff?" << endl;
          }
          else
          {
            // Do nothing
          }

          m_list_view->setFocus( Qt::OtherFocusReason );

          break;
        }
        default:
        {
          break;
        }
      }

//       KListView *view = static_cast<KListView *>( m_widget );
//
//       if ( view->childCount() != 0 )
//       {
//         view->setSelected( !view->currentItem() ?
//                            view->firstChild() :
//                            view->currentItem(), true );
//       }
//
//       view->setFocus();

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
// SLOT IMPLEMENTATIONS (Smb4KSharesViewPart)
/////////////////////////////////////////////////////////////////////////////

void Smb4KSharesViewPart::slotContextMenuRequested( const QPoint &pos )
{
  m_menu->removeAction( m_menu_title );
  delete m_menu_title;

  switch ( m_mode )
  {
    case IconMode:
    {
      QListWidgetItem *item = m_icon_view->itemAt( pos );

      if ( item )
      {
        m_menu_title = m_menu->menu()->addTitle( item->icon(),
                                                 item->text(),
                                                 actionCollection()->action( "unmount_action" ) );
      }
      else
      {
        m_menu_title = m_menu->menu()->addTitle( KIcon( "folder-remote" ),
                                                 i18n( "Shares" ),
                                                 actionCollection()->action( "unmount_action" ) );
      }

      m_menu->menu()->popup( m_icon_view->viewport()->mapToGlobal( pos ) );

      break;
    }
    case ListMode:
    {
      QTreeWidgetItem *item = m_list_view->itemAt( pos );

      if ( item )
      {
        m_menu_title = m_menu->menu()->addTitle( item->icon( Smb4KSharesListViewItem::Item ),
                                                 item->text( Smb4KSharesListViewItem::Item ),
                                                 actionCollection()->action( "unmount_action" ) );
      }
      else
      {
        m_menu_title = m_menu->menu()->addTitle( KIcon( "folder-remote" ),
                                                 i18n( "Shares" ),
                                                 actionCollection()->action( "unmount_action" ) );
      }

      m_menu->menu()->popup( m_list_view->viewport()->mapToGlobal( pos ) );

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotItemSelectionChanged()
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> items = m_icon_view->selectedItems();

      if ( !items.isEmpty() )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( items.first() );
        Smb4KSynchronizationDialog *dlg = m_icon_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );

        actionCollection()->action( "unmount_action" )->setEnabled( (!item->itemData()->share()->isForeign() ||
                                                                    Smb4KSettings::unmountForeignShares()) );
#ifdef __linux__
        actionCollection()->action( "force_unmount_action" )->setEnabled( Smb4KSettings::useForceUnmount() &&
                                                                          (!item->itemData()->share()->isForeign() ||
                                                                          Smb4KSettings::unmountForeignShares()) );
#endif
        actionCollection()->action( "bookmark_action" )->setEnabled( true );

        if ( !item->itemData()->share()->isInaccessible() )
        {
          actionCollection()->action( "synchronize_action" )->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() && !dlg );
          actionCollection()->action( "konsole_action" )->setEnabled( !KGlobal::dirs()->findResource( "exe", "konsole" ).isEmpty() );
          actionCollection()->action( "filemanager_action" )->setEnabled( true );
        }
        else
        {
          actionCollection()->action( "synchronize_action" )->setEnabled( false );
          actionCollection()->action( "konsole_action" )->setEnabled( false );
          actionCollection()->action( "filemanager_action" )->setEnabled( false );
        }
      }
      else
      {
        actionCollection()->action( "unmount_action" )->setEnabled( false );
#ifdef __linux__
        actionCollection()->action( "force_unmount_action" )->setEnabled( false );
#endif
        actionCollection()->action( "bookmark_action" )->setEnabled( false );
        actionCollection()->action( "synchronize_action" )->setEnabled( false );
        actionCollection()->action( "konsole_action" )->setEnabled( false );
        actionCollection()->action( "filemanager_action" )->setEnabled( false );
      }

      break;
    }
    case ListMode:
    {
      // Get the selected item.
      QList<QTreeWidgetItem *> items = m_list_view->selectedItems();

      if ( !items.isEmpty() )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( items.first() );
        Smb4KSynchronizationDialog *dlg = m_list_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );

        actionCollection()->action( "unmount_action" )->setEnabled( (!item->itemData()->share()->isForeign() ||
                                                                    Smb4KSettings::unmountForeignShares()) );
#ifdef __linux__
        actionCollection()->action( "force_unmount_action" )->setEnabled( Smb4KSettings::useForceUnmount() &&
                                                                          (!item->itemData()->share()->isForeign() ||
                                                                          Smb4KSettings::unmountForeignShares()) );
#endif
        actionCollection()->action( "bookmark_action" )->setEnabled( true );

        if ( !item->itemData()->share()->isInaccessible() )
        {
          actionCollection()->action( "synchronize_action" )->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() && !dlg );
          actionCollection()->action( "konsole_action" )->setEnabled( !KGlobal::dirs()->findResource( "exe", "konsole" ).isEmpty() );
          actionCollection()->action( "filemanager_action" )->setEnabled( true );
        }
        else
        {
          actionCollection()->action( "synchronize_action" )->setEnabled( false );
          actionCollection()->action( "konsole_action" )->setEnabled( false );
          actionCollection()->action( "filemanager_action" )->setEnabled( false );
        }
      }
      else
      {
        actionCollection()->action( "unmount_action" )->setEnabled( false );
#ifdef __linux__
        actionCollection()->action( "force_unmount_action" )->setEnabled( false );
#endif
        actionCollection()->action( "bookmark_action" )->setEnabled( false );
        actionCollection()->action( "synchronize_action" )->setEnabled( false );
        actionCollection()->action( "konsole_action" )->setEnabled( false );
        actionCollection()->action( "filemanager_action" )->setEnabled( false );
      }

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotItemPressed( QTreeWidgetItem *item, int /*column*/ )
{
  Smb4KSharesListViewItem *shareItem = static_cast<Smb4KSharesListViewItem *>( item );

  if ( !shareItem && m_list_view->selectedItems().isEmpty() )
  {
    actionCollection()->action( "unmount_action" )->setEnabled( false );
#ifndef __linux__
    actionCollection()->action( "force_unmount_action" )->setEnabled( false );
#endif
    actionCollection()->action( "bookmark_action" )->setEnabled( false );
    actionCollection()->action( "synchronize_action" )->setEnabled( false );
    actionCollection()->action( "konsole_action" )->setEnabled( false );
    actionCollection()->action( "filemanager_action" )->setEnabled( false );
  }
  else
  {
    if ( shareItem )
    {
      Smb4KSynchronizationDialog *dlg = m_list_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+shareItem->itemData()->share()->canonicalPath() );

      actionCollection()->action( "synchronize_action" )->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() &&
                                                                      !dlg &&
                                                                      !shareItem->itemData()->share()->isInaccessible() );
    }
    else
    {
      // Do nothing
    }

    // The rest will be done elsewhere.
  }
}


void Smb4KSharesViewPart::slotItemPressed( QListWidgetItem *item )
{
  Smb4KSharesIconViewItem *shareItem = static_cast<Smb4KSharesIconViewItem *>( item );

  if ( !shareItem && m_icon_view->selectedItems().isEmpty() )
  {
    actionCollection()->action( "unmount_action" )->setEnabled( false );
#ifndef __linux__
    actionCollection()->action( "force_unmount_action" )->setEnabled( false );
#endif
    actionCollection()->action( "bookmark_action" )->setEnabled( false );
    actionCollection()->action( "synchronize_action" )->setEnabled( false );
    actionCollection()->action( "konsole_action" )->setEnabled( false );
    actionCollection()->action( "filemanager_action" )->setEnabled( false );
  }
  else
  {
    if ( shareItem )
    {
      Smb4KSynchronizationDialog *dlg = m_icon_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+shareItem->itemData()->share()->canonicalPath() );

      actionCollection()->action( "synchronize_action" )->setEnabled( !KStandardDirs::findExe( "rsync" ).isEmpty() &&
                                                                      !dlg &&
                                                                      !shareItem->itemData()->share()->isInaccessible() );
    }
    else
    {
      // Do nothing
    }

    // The rest will be done elsewhere.
  }
}


void Smb4KSharesViewPart::slotItemExecuted( QTreeWidgetItem *item, int /*column*/ )
{
  if ( QApplication::keyboardModifiers() == Qt::NoModifier )
  {
    // This is a precaution.
    if ( item != m_list_view->currentItem() )
    {
      m_list_view->setCurrentItem( item );
    }
    else
    {
      // Do nothing
    }

    slotFileManager( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotItemExecuted( QListWidgetItem *item )
{
  // Do not execute the item when keyboard modifiers were pressed
  // or the mouse button is not the left one.
  if ( QApplication::keyboardModifiers() == Qt::NoModifier )
  {
    // This is a precaution.
    if ( item != m_icon_view->currentItem() )
    {
      m_icon_view->setCurrentItem( item );
    }
    else
    {
      // Do nothing
    }

    slotFileManager( false );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotListViewDropEvent( Smb4KSharesListViewItem *item, QDropEvent *e )
{
  if ( item && e )
  {
    switch ( e->proposedAction() )
    {
      case Qt::CopyAction:
      {
        if ( KUrl::List::canDecode( e->mimeData() ) )
        {
          KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );

          KUrl dest;
          dest.setPath( item->itemData()->share()->path() );

          KIO::CopyJob *job = KIO::copy( urlList, dest, KIO::DefaultFlags );

          job->uiDelegate()->setAutoErrorHandlingEnabled( true );
          job->uiDelegate()->setAutoWarningHandlingEnabled( true );
        }
        else
        {
          // Do nothing
        }

        break;
      }
      case Qt::MoveAction:
      {
        if ( KUrl::List::canDecode( e->mimeData() ) )
        {
          KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );

          KUrl dest;
          dest.setPath( item->itemData()->share()->path() );

          KIO::CopyJob *job = KIO::move( urlList, dest, KIO::DefaultFlags );

          job->uiDelegate()->setAutoErrorHandlingEnabled( true );
          job->uiDelegate()->setAutoWarningHandlingEnabled( true );
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
}


void Smb4KSharesViewPart::slotIconViewDropEvent( Smb4KSharesIconViewItem *item, QDropEvent *e )
{
  if ( item && e )
  {
    switch ( e->proposedAction() )
    {
      case Qt::CopyAction:
      {
        if ( KUrl::List::canDecode( e->mimeData() ) )
        {
          KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );

          KUrl dest;
          dest.setPath( item->itemData()->share()->path() );

          KIO::CopyJob *job = KIO::copy( urlList, dest, KIO::DefaultFlags );

          job->uiDelegate()->setAutoErrorHandlingEnabled( true );
          job->uiDelegate()->setAutoWarningHandlingEnabled( true );
        }
        else
        {
          // Do nothing
        }

        break;
      }
      case Qt::MoveAction:
      {
        if ( KUrl::List::canDecode( e->mimeData() ) )
        {
          KUrl::List urlList = KUrl::List::fromMimeData( e->mimeData() );

          KUrl dest;
          dest.setPath( item->itemData()->share()->path() );

          KIO::CopyJob *job = KIO::move( urlList, dest, KIO::DefaultFlags );

          job->uiDelegate()->setAutoErrorHandlingEnabled( true );
          job->uiDelegate()->setAutoWarningHandlingEnabled( true );
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
}


void Smb4KSharesViewPart::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  switch ( m_mode )
  {
    case IconMode:
    {
      if ( !share->isForeign() || Smb4KSettings::showAllShares() )
      {
        Smb4KSharesIconViewItem *item = new Smb4KSharesIconViewItem( share, m_icon_view );
        item->setShowMountPoint( Smb4KSettings::showMountPoint() );
        
        m_icon_view->sortItems( Qt::AscendingOrder );
      }
      else
      {
        // Do nothing
      }
      
      actionCollection()->action( "unmount_all_action" )->setEnabled( (m_icon_view->count() != 0) );
      
      break;
    }
    case ListMode:
    {
      if ( !share->isForeign() || Smb4KSettings::showAllShares() )
      {
        Smb4KSharesListViewItem *item = new Smb4KSharesListViewItem( share, m_list_view );
        item->setShowMountPoint( Smb4KSettings::showMountPoint() );
        
        m_list_view->sortItems( Smb4KSharesListView::Item, Qt::AscendingOrder );
      }
      else
      {
        // Do nothing
      }
      
      actionCollection()->action( "unmount_all_action" )->setEnabled( (m_list_view->topLevelItemCount() != 0) );
      
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotShareUnmounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  switch ( m_mode )
  {
    case IconMode:
    {
      Smb4KSharesIconViewItem *item = NULL;
      
      for ( int i = 0; i < m_icon_view->count(); ++i )
      {
        item = static_cast<Smb4KSharesIconViewItem *>( m_icon_view->item( i ) );
        
        if ( item && (QString::compare( item->itemData()->share()->path(), share->path() ) == 0 ||
             QString::compare( item->itemData()->share()->canonicalPath(), share->canonicalPath() ) == 0) )
        {
          if ( item == m_icon_view->currentItem() )
          {
            m_icon_view->setCurrentItem( NULL );
          }
          else
          {
            // Do nothing
          }
          
          delete m_icon_view->takeItem( i );
          break;
        }
        else
        {
          continue;
        }
      }
      
      actionCollection()->action( "unmount_all_action" )->setEnabled( (m_icon_view->count() != 0) );
      
      break;
    }
    case ListMode:
    {
      Smb4KSharesListViewItem *item = NULL;
      
      for ( int i = 0; i < m_list_view->topLevelItemCount(); ++i )
      {
        item = static_cast<Smb4KSharesListViewItem *>( m_list_view->topLevelItem( i ) );
        
        if ( item && (QString::compare( item->itemData()->share()->path(), share->path() ) == 0 ||
             QString::compare( item->itemData()->share()->canonicalPath(), share->canonicalPath() ) == 0) )
        {
          if ( item == m_list_view->currentItem() )
          {
            m_list_view->setCurrentItem( NULL );
          }
          else
          {
            // Do nothing
          }
          
          delete m_list_view->takeTopLevelItem( i );
          break;
        }
        else
        {
          continue;
        }
      }
      
      actionCollection()->action( "unmount_all_action" )->setEnabled( (m_list_view->topLevelItemCount() != 0) );
      
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotShareUpdated( Smb4KShare *share )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      Smb4KSharesIconViewItem *item = NULL;
      
      for ( int i = 0; i < m_icon_view->count(); ++i )
      {
        item = static_cast<Smb4KSharesIconViewItem *>( m_icon_view->item( i ) );
        
        if ( item && (QString::compare( item->itemData()->share()->path(), share->path() ) == 0 ||
             QString::compare( item->itemData()->share()->canonicalPath(), share->canonicalPath() ) == 0) )
        {
          if ( !item->sameShareObject( share ) )
          {
            item->replaceShareObject( share );
          }
          else
          {
            // Do nothing
          }
          
          m_icon_view->updateToolTip();
          break;
        }
        else
        {
          continue;
        }
      }
      break;
    }
    case ListMode:
    {
      Smb4KSharesListViewItem *item = NULL;
      
      for ( int i = 0; i < m_list_view->topLevelItemCount(); ++i )
      {
        item = static_cast<Smb4KSharesListViewItem *>( m_list_view->topLevelItem( i ) );
        
        if ( item && (QString::compare( item->itemData()->share()->path(), share->path() ) == 0 ||
             QString::compare( item->itemData()->share()->canonicalPath(), share->canonicalPath() ) == 0) )
        {
          if ( !item->sameShareObject( share ) )
          {
            item->replaceShareObject( share );
          }
          else
          {
            // Do nothing
          }
          
          m_list_view->updateToolTip();
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


void Smb4KSharesViewPart::slotUnmountShare( bool /*checked*/ )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );

        if ( item )
        {
          Smb4KMounter::self()->unmountShare( item->itemData()->share(), false );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );

        if ( item )
        {
          Smb4KMounter::self()->unmountShare( item->itemData()->share(), false );
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotForceUnmountShare( bool /*checked*/ )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );

        if ( item )
        {
          Smb4KMounter::self()->unmountShare( item->itemData()->share(), true );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );

        if ( item )
        {
          Smb4KMounter::self()->unmountShare( item->itemData()->share(), true );
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotUnmountAllShares( bool /*checked*/ )
{
  Smb4KMounter::self()->unmountAllShares();
}


void Smb4KSharesViewPart::slotSynchronize( bool /*checked*/ )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );
        Smb4KSynchronizationDialog *dlg = m_icon_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );

        if ( item && !item->itemData()->share()->isInaccessible() && !dlg )
        {
          dlg = new Smb4KSynchronizationDialog( item->itemData()->share(), m_icon_view );
          dlg->setObjectName( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );
          dlg->setVisible( true );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );
        Smb4KSynchronizationDialog *dlg = m_list_view->findChild<Smb4KSynchronizationDialog *>( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );

        if ( item && !item->itemData()->share()->isInaccessible() && !dlg )
        {
          dlg = new Smb4KSynchronizationDialog( item->itemData()->share(), m_list_view );
          dlg->setObjectName( "SynchronizationDialog_"+item->itemData()->share()->canonicalPath() );
          dlg->setVisible( true );
        }
        else
        {
          // Do nothing
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

void Smb4KSharesViewPart::slotKonsole( bool /*checked*/ )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );

        if ( item && !item->itemData()->share()->isInaccessible() )
        {
          Smb4KCore::open( item->itemData()->share(), Smb4KCore::Konsole );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );

        if ( item && !item->itemData()->share()->isInaccessible() )
        {
          Smb4KCore::open( item->itemData()->share(), Smb4KCore::Konsole );
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotFileManager( bool /*checked*/ )
{
  switch ( m_mode )
  {
    case IconMode:
    {
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );

        if ( item && !item->itemData()->share()->isInaccessible() )
        {
          Smb4KCore::open( item->itemData()->share(), Smb4KCore::FileManager );
        }
        else
        {
          // Do nothing
        }
      }

      break;
    }
    case ListMode:
    {
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();

      for ( int i = 0; i < selected_items.size(); ++i )
      {
        Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );

        if ( item && !item->itemData()->share()->isInaccessible() )
        {
          Smb4KCore::open( item->itemData()->share(), Smb4KCore::FileManager );
        }
        else
        {
          // Do nothing
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


void Smb4KSharesViewPart::slotAddBookmark( bool /*checked */)
{
  switch ( m_mode )
  {
    case IconMode:
    {
      Smb4KBookmarkDialog *dlg = m_icon_view->findChild<Smb4KBookmarkDialog *>();
      QList<QListWidgetItem *> selected_items = m_icon_view->selectedItems();
      QList<Smb4KBookmark *> bookmarks;

      if ( !selected_items.isEmpty() )
      {
        for ( int i = 0; i < selected_items.size(); ++i )
        {
          Smb4KSharesIconViewItem *item = static_cast<Smb4KSharesIconViewItem *>( selected_items.at( i ) );
          bookmarks << new Smb4KBookmark( item->itemData()->share() );

          continue;
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
          dlg = new Smb4KBookmarkDialog( m_icon_view );
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

      break;
    }
    case ListMode:
    {
      Smb4KBookmarkDialog *dlg = m_list_view->findChild<Smb4KBookmarkDialog *>();
      QList<QTreeWidgetItem *> selected_items = m_list_view->selectedItems();
      QList<Smb4KBookmark *> bookmarks;

      if ( !selected_items.isEmpty() )
      {
        for ( int i = 0; i < selected_items.size(); ++i )
        {
          Smb4KSharesListViewItem *item = static_cast<Smb4KSharesListViewItem *>( selected_items.at( i ) );
          bookmarks << new Smb4KBookmark( item->itemData()->share() );

          continue;
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
          dlg = new Smb4KBookmarkDialog( m_icon_view );
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

      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KSharesViewPart::slotMounterAboutToStart( Smb4KShare *share, int process )
{
  switch ( process )
  {
    case Smb4KMounter::MountShare:
    {
      if ( !m_silent )
      {
        emit setStatusBarText( i18n( "Mounting share %1..." ).arg( share->unc() ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KMounter::UnmountShare:
    {
      if ( !m_silent )
      {
        emit setStatusBarText( i18n( "Unmounting share %1..." ).arg( share->unc() ) );
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


void Smb4KSharesViewPart::slotMounterFinished( Smb4KShare */*share*/, int /*process*/ )
{
  if ( !m_silent )
  {
    emit setStatusBarText( i18n( "Done." ) );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KSharesViewPart::slotAboutToQuit()
{
  saveSettings();
}


#include "smb4ksharesview_part.moc"
