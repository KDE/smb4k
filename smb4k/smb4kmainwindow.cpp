/***************************************************************************
    smb4kmainwindow  -  The main window of Smb4K.
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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
#include <QSize>
#include <QVariantList>
#include <QDockWidget>
#include <QMenu>
#include <QActionGroup>
#include <QLabel>
#include <QTimer>
#include <QTabBar>

// KDE includes
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kpluginloader.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kxmlguifactory.h>
#include <kconfigdialog.h>
#include <kactionmenu.h>
#include <kstatusbar.h>
#include <kiconeffect.h>

// application specific includes
#include <smb4kmainwindow.h>
#include <smb4ksystemtray.h>
#include <core/smb4ksettings.h>
#include <core/smb4kdefs.h>
#include <core/smb4kcore.h>
#include <core/smb4kbookmark.h>
#include <core/smb4kglobal.h>
#include <core/smb4kwalletmanager.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4kscanner.h>
#include <core/smb4kmounter.h>
#include <core/smb4kprintinfo.h>
#include <core/smb4kprint.h>
#include <core/smb4ksynchronizationinfo.h>
#include <core/smb4ksynchronizer.h>
#include <core/smb4kpreviewitem.h>
#include <core/smb4kpreviewer.h>
#include <core/smb4ksearch.h>
#include <dialogs/smb4kbookmarkeditor.h>


using namespace Smb4KGlobal;

Smb4KMainWindow::Smb4KMainWindow()
: KParts::MainWindow(), m_system_tray_widget( NULL )
{
  m_bookmarks = new QActionGroup( actionCollection() );

  setStandardToolBarMenuEnabled( true );
  createStandardStatusBarAction();
  setDockNestingEnabled( true );
  setupActions();
  setupGUI( QSize( 800, 600 ), Default, "smb4k_shell.rc" );
  setupView();
  setupStatusBar();
  setupSystemTrayWidget();
  setupBookmarksMenu();

  // Apply the main window settings.
  setAutoSaveSettings( KConfigGroup( Smb4KSettings::self()->config(), "MainWindow" ), true );
}


Smb4KMainWindow::~Smb4KMainWindow()
{
}


void Smb4KMainWindow::setupActions()
{
  // Quit action
  KAction *quit_action = KStandardAction::quit( this, SLOT( slotQuit() ), actionCollection() );
  actionCollection()->addAction( "quit_action", quit_action );

  // Configure action
  KAction *configure_action = KStandardAction::preferences( this, SLOT( slotConfigDialog() ), actionCollection() );
  actionCollection()->addAction( "configure_action", configure_action );

  // Dock widgets menu
  KActionMenu *dock_widgets_menu = new KActionMenu( KIcon( "tab-duplicate" ), i18n( "Dock Widgets" ), actionCollection() );
  actionCollection()->addAction( "dock_widgets_menu", dock_widgets_menu );

  m_dock_widgets = new QActionGroup( actionCollection() );
  m_dock_widgets->setExclusive( false );

  // Shares view
  KActionMenu *shares_view_menu = new KActionMenu( KIcon( "view-choose" ), i18n( "Shares View" ), actionCollection() );
  actionCollection()->addAction( "shares_view_menu", shares_view_menu );

  QActionGroup *view_modes_group = new QActionGroup( actionCollection() );
  view_modes_group->setExclusive( true );
  connect( view_modes_group, SIGNAL( triggered( QAction * ) ), this, SLOT( slotViewModeTriggered( QAction * ) ) );

  KAction *icon_view_action = new KAction( i18n( "Icon View" ), view_modes_group );
  icon_view_action->setCheckable( true );
  view_modes_group->addAction( icon_view_action );
  actionCollection()->addAction( "icon_view_action", icon_view_action );

  KAction *list_view_action = new KAction( i18n( "List View" ), view_modes_group );
  list_view_action->setCheckable( true );
  view_modes_group->addAction( list_view_action );
  actionCollection()->addAction( "list_view_action", list_view_action );

  shares_view_menu->addAction( icon_view_action );
  shares_view_menu->addAction( list_view_action );

  if ( Smb4KSettings::sharesIconView() )
  {
    actionCollection()->action( "icon_view_action" )->setChecked( true );
  }
  else if ( Smb4KSettings::sharesListView() )
  {
    actionCollection()->action( "list_view_action" )->setChecked( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::setupStatusBar()
{
  // Set up the progress bar.
  m_progress_bar = new QProgressBar( statusBar() );
  m_progress_bar->setFixedWidth( 100 );
  m_progress_bar->setFixedHeight( statusBar()->fontMetrics().height() );
  m_progress_bar->setAlignment( Qt::AlignCenter );
  m_progress_bar->setVisible( false );

  // Set the icon on the right side that represents the initial
  // state of the wallet manager.
  m_pass_icon = new QLabel( statusBar() );
  m_pass_icon->setContentsMargins( 0, 0, 4, 0 );
  m_pass_icon->setAlignment( Qt::AlignCenter );

  // The feedback icon.
  m_feedback_icon = new QLabel( statusBar() );
  m_feedback_icon->setContentsMargins( 0, 0, 0, 0 );
  m_feedback_icon->setAlignment( Qt::AlignCenter );

  statusBar()->addPermanentWidget( m_progress_bar );
  statusBar()->addPermanentWidget( m_feedback_icon );
  statusBar()->addPermanentWidget( m_pass_icon );

  slotWalletManagerInitialized();

  // Since we have an uncommon main window, connect the status
  // bar text signals to the respective slot.
  if ( m_browser_part )
  {
    connect( m_browser_part, SIGNAL( setStatusBarText( const QString & ) ),
             this,           SLOT( slotSetStatusBarText( const QString & ) ) );
  }
  else
  {
    // Do nothing
  }

  if ( m_shares_part )
  {
    connect( m_shares_part, SIGNAL( setStatusBarText( const QString & ) ),
             this,          SLOT( slotSetStatusBarText( const QString & ) ) );
  }
  else
  {
    // Do nothing
  }

  if ( m_search_part )
  {
    connect( m_search_part, SIGNAL( setStatusBarText( const QString & ) ),
             this,          SLOT( slotSetStatusBarText( const QString & ) ) );
  }
  else
  {
    // Do nothing
  }


  connect( Smb4KWalletManager::self(), SIGNAL( initialized() ),
           this,                       SLOT( slotWalletManagerInitialized() ) );

  connect( Smb4KMounter::self(), SIGNAL( finished( Smb4KShare *, int ) ),
           this,                 SLOT( slotVisualMountFeedback( Smb4KShare *, int ) ) );

  connect( Smb4KPrint::self(),   SIGNAL( aboutToStart( Smb4KPrintInfo * ) ),
           this,                 SLOT( slotPrintStartMessages( Smb4KPrintInfo * ) ) );

  connect( Smb4KPrint::self(),   SIGNAL( finished( Smb4KPrintInfo * ) ),
           this,                 SLOT( slotPrintFinishMessages( Smb4KPrintInfo * ) ) );

  connect( Smb4KSynchronizer::self(), SIGNAL( aboutToStart( Smb4KSynchronizationInfo * ) ),
           this,                      SLOT( slotSynchronizerStartMessages( Smb4KSynchronizationInfo * ) ) );

  connect( Smb4KSynchronizer::self(), SIGNAL( finished( Smb4KSynchronizationInfo* ) ),
           this,                      SLOT( slotSynchronizerFinishMessages( Smb4KSynchronizationInfo * ) ) );

  connect( Smb4KPreviewer::self(), SIGNAL( aboutToStart( Smb4KPreviewItem * ) ),
           this,                   SLOT( slotPreviewerStartMessages( Smb4KPreviewItem * ) ) );

  connect( Smb4KPreviewer::self(), SIGNAL( finished( Smb4KPreviewItem * ) ),
           this,                   SLOT( slotPreviewerFinishMessages( Smb4KPreviewItem * ) ) );
}


void Smb4KMainWindow::setupView()
{
  // We do not set a central widget, because it causes "problems"
  // with the dock widgets. We have the nested dock widget property
  // set to true, so we can arrange the dock widgets as we like,
  // nonetheless.

  QDockWidget *browser_dock = NULL;
  QDockWidget *search_dock = NULL;
  QDockWidget *shares_dock = NULL;

  // Load the network browser.
  KPluginLoader browser_loader( "libsmb4knetworkbrowser" );
  KPluginFactory *browser_factory = browser_loader.factory();

  if ( browser_factory )
  {
    QVariantList args;
    args << QString( "bookmark_shortcut=\"false\"" );

    m_browser_part = browser_factory->create<KParts::Part>( this, args );

    if ( m_browser_part )
    {
      // Add dock widget to the main window.
      browser_dock = new QDockWidget( i18n( "Network Neighborhood" ), this );
      browser_dock->setObjectName( "NetworkBrowserDockWidget" );
      browser_dock->setWidget( m_browser_part->widget() );
      browser_dock->setAllowedAreas( Qt::LeftDockWidgetArea );

      addDockWidget( Qt::LeftDockWidgetArea, browser_dock );

      // Make the menu and the tool bar working.
      guiFactory()->addClient( m_browser_part );

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction( browser_dock->toggleViewAction() );
      static_cast<KActionMenu *>( actionCollection()->action( "dock_widgets_menu" ) )->addAction( browser_dock->toggleViewAction() );
    }
    else
    {
      KMessageBox::error( this, i18n( "The network browser could not be created." ) );

      // We will continue without the network browser.
    }
  }
  else
  {
    KMessageBox::error( this, "<qt>"+browser_loader.errorString()+"</qt>" );
    KApplication::exit( 1 );
    return;
  }

  // Load the search dialog.
  KPluginLoader search_loader( "libsmb4ksearchdialog" );
  KPluginFactory *search_factory = search_loader.factory();

  if ( search_factory )
  {
    m_search_part = search_factory->create<KParts::Part>( this, QVariantList() );

    if ( m_search_part )
    {
      // Add dock widget to the main window.
      search_dock = new QDockWidget( i18n( "Network Search" ), this );
      search_dock->setObjectName( "SearchDialogDockWidget" );
      search_dock->setWidget( m_search_part->widget() );
      search_dock->setAllowedAreas( Qt::LeftDockWidgetArea );

      addDockWidget( Qt::LeftDockWidgetArea, search_dock );

      // Make the menu and the tool bar working.
      guiFactory()->addClient( m_search_part );

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction( search_dock->toggleViewAction() );
      static_cast<KActionMenu *>( actionCollection()->action( "dock_widgets_menu" ) )->addAction( search_dock->toggleViewAction() );
    }
    else
    {
      KMessageBox::error( this, i18n( "The search dialog could not be created." ) );

      // We will continue without the search dialog.
    }
  }
  else
  {
    KMessageBox::error( this, "<qt>"+search_loader.errorString()+"</qt>" );
    KApplication::exit( 1 );
    return;
  }

  KPluginLoader shares_loader( "libsmb4ksharesview" );
  KPluginFactory *shares_factory = shares_loader.factory();

  if ( shares_factory )
  {
    QVariantList args;
    args << QString( "bookmark_shortcut=\"false\"" );

    m_shares_part = shares_factory->create<KParts::Part>( this, args );

    if ( m_shares_part )
    {
      // Add dock widget to the main window.
      shares_dock = new QDockWidget( i18n( "Mounted Shares" ), this );
      shares_dock->setObjectName( "SharesViewDockWidget" );
      shares_dock->setWidget( m_shares_part->widget() );
      shares_dock->setAllowedAreas( Qt::LeftDockWidgetArea );

      addDockWidget( Qt::LeftDockWidgetArea, shares_dock );

      // Make the menu and the tool bar working.
      guiFactory()->addClient( m_shares_part );

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction( shares_dock->toggleViewAction() );
      static_cast<KActionMenu *>( actionCollection()->action( "dock_widgets_menu" ) )->addAction( shares_dock->toggleViewAction() );
    }
    else
    {
      KMessageBox::error( this, i18n( "The shares view could not be created." ) );

      // We will continue without the search dialog.
    }
  }
  else
  {
    KMessageBox::error( this, "<qt>"+shares_loader.errorString()+"</qt>" );
    KApplication::exit( 1 );
    return;
  }

  KConfigGroup config_group( Smb4KSettings::self()->config(), "MainWindow" );

  if ( !config_group.exists() )
  {
    // Create a tab widget from the parts at first start.
    // Afterwards, let the autosaving take over.
    if ( browser_dock && search_dock )
    {
      tabifyDockWidget( browser_dock, search_dock );
    }
    else
    {
      // Do nothing
    }

    if ( search_dock && shares_dock )
    {
      tabifyDockWidget( search_dock, shares_dock );
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


void Smb4KMainWindow::setupSystemTrayWidget()
{
  if ( !m_system_tray_widget )
  {
    m_system_tray_widget = new Smb4KSystemTray( this );
  }
  else
  {
    // Do nothing
  }

  connect( m_system_tray_widget, SIGNAL( quitSelected() ),
           this,                 SLOT( slotQuit() ) );

  connect( m_system_tray_widget, SIGNAL( settingsChanged( const QString & ) ),
           this,                 SLOT( slotSettingsChanged( const QString & ) ) );

  m_system_tray_widget->embed( Smb4KSettings::embedIntoSystemTray() );
}


void Smb4KMainWindow::setupBookmarksMenu()
{
  // Action to invoke the bookmark editor
  KAction *edit_bookmarks = new KAction( KIcon( "bookmarks-organize" ), i18n( "&Edit Bookmarks" ),
                            actionCollection() );
  actionCollection()->addAction( "edit_bookmarks_action", edit_bookmarks );

  // The enhanced bookmark action for the main window
  KAction *add_bookmark = new KAction( KIcon( "bookmark-new" ), i18n( "Add &Bookmark" ),
                          actionCollection() );
  add_bookmark->setShortcut( QKeySequence( Qt::CTRL+Qt::Key_B ) );
  actionCollection()->addAction( "main_bookmark_action", add_bookmark );

  // This is a workaround (see smb4k_shell.rc).
  KAction *separator      = new KAction( actionCollection() );
  separator->setSeparator( true );
  actionCollection()->addAction( "bookmarks_menu_separator", separator );

  QList<QAction *> bookmark_actions;
  bookmark_actions.append( edit_bookmarks );
  bookmark_actions.append( add_bookmark );
  bookmark_actions.append( separator );

  plugActionList( "bookmark_actions", bookmark_actions );

  connect( edit_bookmarks,               SIGNAL( triggered( bool ) ),
           this,                         SLOT( slotOpenBookmarkEditor( bool ) ) );

  connect( add_bookmark,                 SIGNAL( triggered( bool ) ),
           this,                         SLOT( slotAddBookmark( bool ) ) );

  connect( Smb4KCore::bookmarkHandler(), SIGNAL( updated() ),
           this,                         SLOT( slotBookmarksUpdated() ) );

  connect( Smb4KCore::mounter(),         SIGNAL( updated() ),
           this,                         SLOT( slotEnableBookmarks() ) );

  connect( m_bookmarks,                  SIGNAL( triggered( QAction * ) ),
           this,                         SLOT( slotBookmarkTriggered( QAction * ) ) );

  // Setup bookmarks.
  slotBookmarksUpdated();
}


void Smb4KMainWindow::loadSettings()
{
  // Send custom event to the dock widgets, so that they know
  // that they have to reload their settings.
  if ( m_browser_part )
  {
    QEvent *customBrowserEvent = new QEvent( (QEvent::Type)EVENT_LOAD_SETTINGS );
    KApplication::postEvent( m_browser_part, customBrowserEvent );
  }
  else
  {
    // Do nothing
  }

  if ( m_search_part )
  {
    QEvent *customSearchEvent = new QEvent( (QEvent::Type)EVENT_LOAD_SETTINGS );
    KApplication::postEvent( m_search_part, customSearchEvent );
  }
  else
  {
    // Do nothing
  }

  if ( m_shares_part )
  {
    QEvent *customSharesEvent = new QEvent( (QEvent::Type)EVENT_LOAD_SETTINGS );
    KApplication::postEvent( m_shares_part, customSharesEvent );
  }
  else
  {
    // Do nothing
  }

  // Check the right view mode action.
  if ( Smb4KSettings::sharesIconView() )
  {
    actionCollection()->action( "icon_view_action" )->setChecked( true );
  }
  else if ( Smb4KSettings::sharesListView() )
  {
    actionCollection()->action( "list_view_action" )->setChecked( true );
  }
  else
  {
    // Do nothing
  }

  // Reload the list of bookmarks.
  slotBookmarksUpdated();

  // Check the state of the password handler and the wallet settings and
  // set the pixmap in the status bar accordingly.
  slotWalletManagerInitialized();
}


void Smb4KMainWindow::saveSettings()
{
}


bool Smb4KMainWindow::queryClose()
{
  if ( !kapp->sessionSaving() && isVisible() && m_system_tray_widget->isEmbedded() &&
       Smb4KSettings::embedIntoSystemTray() )
  {
    // This part has been 'stolen' from JuK application.
    KMessageBox::information(this,
            i18n( "<qt>Closing the main window will keep Smb4K running in the system tray. "
                  "Use 'Quit' from the 'File' menu to quit the application.</qt>"),
            i18n( "Docking" ), "DockToSystemTrayInfo" );
    setVisible( false );
    return false;

  }
  else
  {
    return true;
  }
}


bool Smb4KMainWindow::queryExit()
{
  Smb4KSettings::setStartMainWindowDocked( !isVisible() );
  return true;
}


void Smb4KMainWindow::timerEvent( QTimerEvent */*e*/ )
{
  m_progress_bar->setValue( m_progress_bar->value() + 1 );
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMainWindow::slotQuit()
{
  kapp->quit();
}


void Smb4KMainWindow::slotConfigDialog()
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
  KPluginLoader loader( "libsmb4kconfigdialog" );
  KPluginFactory *config_factory = loader.factory();

  if ( config_factory )
  {
    dlg = config_factory->create<KConfigDialog>( this );
    dlg->setObjectName( "ConfigDialog" );

    // ... and show it.
    if ( dlg )
    {
      connect( dlg,  SIGNAL( settingsChanged( const QString & ) ),
               this, SLOT( slotSettingsChanged( const QString & ) ) );

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


void Smb4KMainWindow::slotSettingsChanged( const QString & )
{
  loadSettings();
}


void Smb4KMainWindow::slotOpenBookmarkEditor( bool /*checked*/ )
{
  Smb4KBookmarkEditor *dlg = NULL;

  // Do not open the bookmark editor twice. So, look if there
  // is already one. (This will also catch a bookmark editor
  // that was opened with m_system_tray->contextMenu() as
  // parent).
  dlg = findChild<Smb4KBookmarkEditor *>();

  if ( !dlg )
  {
    dlg = new Smb4KBookmarkEditor( this );

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


void Smb4KMainWindow::slotBookmarksUpdated()
{
  // First of all, unplug the bookmarks.
  unplugActionList( "bookmarks" );

  // Get the list of bookmark actions and delete all entries. We could
  // also try to keep those actions that are not obsolete, but I think
  // this is the cleanest way.
  while ( !m_bookmarks->actions().isEmpty() )
  {
    actionCollection()->removeAction( m_bookmarks->actions().first() );
    delete m_bookmarks->actions().takeFirst();
  }

  // Get the list of bookmarks:
  QList<Smb4KBookmark *> bookmarks = Smb4KCore::bookmarkHandler()->getBookmarks();
  QMap<QString, bool> actions;

  // Prepare the list of bookmarks for display:
  if ( !bookmarks.isEmpty() )
  {
    // Enable the "Edit Bookmarks" action:
    actionCollection()->action( "edit_bookmarks_action" )->setEnabled( true );

    // Work around sorting problems:
    for ( int i = 0; i < bookmarks.size(); ++i )
    {
      QList<Smb4KShare *> shares_list = findShareByUNC( bookmarks.at( i )->unc() );
      bool enable = true;

      for ( int j = 0; j < shares_list.size(); ++j )
      {
        if ( !shares_list.at( j )->isForeign() )
        {
          enable = false;
          break;
        }
        else
        {
          continue;
        }
      }

      if ( !bookmarks.at( i )->label().isEmpty() && Smb4KSettings::showCustomBookmarkLabel() )
      {
        actions.insert( bookmarks.at( i )->label(), enable );
      }
      else
      {
        actions.insert( bookmarks.at( i )->unc(), enable );
      }
    }
  }
  else
  {
    // Disable the "Edit Bookmarks" action:
    actionCollection()->action( "edit_bookmarks_action" )->setEnabled( false );
  }

  // Now create the actions and put them into the action group
  // and the menu.
  QList<QAction *> bookmarks_list;

  QMapIterator<QString, bool> it( actions );

  while ( it.hasNext() )
  {
    it.next();

    KAction *bm_action = new KAction( KIcon( "folder-remote" ), it.key(), m_bookmarks );
    bm_action->setData( it.key() );
    bm_action->setEnabled( it.value() );
    bookmarks_list.append( bm_action );
  }

  plugActionList( "bookmarks", bookmarks_list );
}


void Smb4KMainWindow::slotEnableBookmarks()
{
  // Enable/disable the bookmark actions.
  for ( int i = 0; i < m_bookmarks->actions().size(); ++i )
  {
    QList<Smb4KShare *> shares_list = findShareByUNC( m_bookmarks->actions().at( i )->data().toString() );

    bool enable = true;

    for ( int j = 0; j < shares_list.size(); ++j )
    {
      if ( !shares_list.at( j )->isForeign() )
      {
        enable = false;

        break;
      }
      else
      {
        continue;
      }
    }

    m_bookmarks->actions().at( i )->setEnabled( enable );
  }
}


void Smb4KMainWindow::slotBookmarkTriggered( QAction *action )
{
  if ( action )
  {
    Smb4KBookmark *bookmark = Smb4KCore::bookmarkHandler()->findBookmarkByUNC( action->data().toString() );

    if ( bookmark )
    {
      Smb4KShare share( bookmark->hostName(), bookmark->shareName() );
      share.setWorkgroupName( bookmark->workgroupName() );
      share.setHostIP( bookmark->hostIP() );
      share.setLogin( bookmark->login() );

      Smb4KCore::mounter()->mountShare( &share );
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotAddBookmark( bool /*checked*/ )
{
  if ( m_browser_part )
  {
    if ( m_browser_part->widget()->hasFocus() )
    {
      QEvent *customBrowserEvent = new QEvent( (QEvent::Type)EVENT_ADD_BOOKMARK );
      KApplication::postEvent( m_browser_part, customBrowserEvent );
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

  if ( m_shares_part )
  {
    if ( m_shares_part->widget()->hasFocus() )
    {
      QEvent *customSharesEvent = new QEvent( (QEvent::Type)EVENT_ADD_BOOKMARK );
      KApplication::postEvent( m_shares_part, customSharesEvent );
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


void Smb4KMainWindow::slotViewModeTriggered( QAction *action )
{
  // Change the settings if necessary.
  if ( QString::compare( action->objectName(), "icon_view_action" ) == 0 )
  {
    if ( !Smb4KSettings::sharesIconView() )
    {
      Smb4KSettings::setSharesIconView( true );
      Smb4KSettings::setSharesListView( false );
    }
    else
    {
      return;
    }
  }
  else if ( QString::compare( action->objectName(), "list_view_action" ) == 0 )
  {
    if ( !Smb4KSettings::sharesListView() )
    {
      Smb4KSettings::setSharesIconView( false );
      Smb4KSettings::setSharesListView( true );
    }
    else
    {
      return;
    }
  }
  else
  {
    return;
  }

  // Write the new configuration to the configuration file.
  Smb4KSettings::self()->writeConfig();

  // Notify the part that it has to reload its settings.
  if ( m_shares_part )
  {
    QEvent *customSharesEvent = new QEvent( (QEvent::Type)EVENT_LOAD_SETTINGS );
    KApplication::postEvent( m_shares_part, customSharesEvent );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotWalletManagerInitialized()
{
  if ( Smb4KWalletManager::self()->useWalletSystem() )
  {
    switch ( Smb4KWalletManager::self()->currentState() )
    {
      case Smb4KWalletManager::UseWallet:
      {
        m_pass_icon->setPixmap( KIconLoader::global()->loadIcon( "wallet-open",
                                KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_pass_icon->setToolTip( i18n( "Wallet is in use." ) );
        break;
      }
      default:
      {
        m_pass_icon->setPixmap( KIconLoader::global()->loadIcon( "wallet-closed",
                                KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_pass_icon->setToolTip( i18n( "Wallet is not in use." ) );
        break;
      }
    }
  }
  else
  {
    m_pass_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-password",
                            KIconLoader::Small, 0, KIconLoader::DefaultState ) );

    if ( Smb4KSettings::rememberLogins() )
    {
      m_pass_icon->setToolTip( i18n( "Password dialog mode is used and logins are remembered." ) );
    }
    else
    {
      m_pass_icon->setToolTip( i18n( "Password dialog mode is used and logins are not remembered." ) );
    }
  }
}


void Smb4KMainWindow::slotSetStatusBarText( const QString &text )
{
  statusBar()->showMessage( text, 2000 );
}


void Smb4KMainWindow::slotVisualMountFeedback( Smb4KShare *share, int process )
{
  bool successful = true;

  // Visual feedback.
  switch ( process )
  {
    case Smb4KMounter::MountShare:
    {
      if ( share && share->isMounted() )
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-ok",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "The share was successfully mounted." ) );
      }
      else
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-cancel",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "The share could not be mounted." ) );
        successful = false;
      }
      break;
    }
    case Smb4KMounter::UnmountShare:
    {
      if ( share && share->isMounted() )
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-cancel",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "The share could not be unmounted." ) );
        successful = false;
      }
      else
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-ok",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "The share was successfully unmounted." ) );
      }
      break;
    }
    default:
    {
      break;
    }
  }

  if ( successful )
  {
    QList<QTabBar *> list = findChildren<QTabBar *>();
    QDockWidget *shares_dock = findChild<QDockWidget *>( "SharesViewDockWidget" );

    if ( shares_dock )
    {
      for ( int i = 0; i < list.size(); ++i )
      {
        if ( list.at( i )->count() != 0 )
        {
          for ( int j = 0; j < list.at( i )->count(); ++j )
          {
            if ( QString::compare( shares_dock->windowTitle(), list.at( i )->tabText( j ) ) == 0 &&
                list.at( i )->currentIndex() != j )
            {
              list.at( i )->setTabTextColor( j, palette().highlightedText().color() ) ;
              break;
            }
            else
            {
              continue;
            }
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
      // Do nothing
    }
  }
  else
  {
    // Do nothing
  }

  QTimer::singleShot( 2000, this, SLOT( slotEndVisualFeedback() ) );
}


void Smb4KMainWindow::slotPrintStartMessages( Smb4KPrintInfo *info )
{
  statusBar()->showMessage( i18n( "Sending file to printer %1..." ).arg( info->printer()->unc() ), 0 );
}


void Smb4KMainWindow::slotPrintFinishMessages( Smb4KPrintInfo */*info*/ )
{
  statusBar()->showMessage( i18n( "Done." ), 2000 );
}


void Smb4KMainWindow::slotSynchronizerStartMessages( Smb4KSynchronizationInfo */*info*/ )
{
  // We do not need to be very verbose here, because the
  // user can see what he/she is synchronizing in the
  // synchronization dialog.
  statusBar()->showMessage( i18n( "Synchronizing..." ), 0 );
}


void Smb4KMainWindow::slotSynchronizerFinishMessages( Smb4KSynchronizationInfo */*info*/ )
{
  statusBar()->showMessage( i18n( "Done." ), 2000 );
}


void Smb4KMainWindow::slotPreviewerStartMessages( Smb4KPreviewItem *item )
{
  statusBar()->showMessage( i18n( "Retrieving preview from %1..." ).arg( item->share()->unc() ), 0 );
}


void Smb4KMainWindow::slotPreviewerFinishMessages( Smb4KPreviewItem */*item*/ )
{
  statusBar()->showMessage( i18n( "Done." ), 2000 );
}


void Smb4KMainWindow::slotEndVisualFeedback()
{
  m_feedback_icon->setPixmap( QPixmap() );

  QList<QTabBar *> list = findChildren<QTabBar *>();
  QDockWidget *shares_dock = findChild<QDockWidget *>( "SharesViewDockWidget" );

  if ( shares_dock )
  {
    for ( int i = 0; i < list.size(); ++i )
    {
      if ( list.at( i )->count() != 0 )
      {
        for ( int j = 0; j < list.at( i )->count(); ++j )
        {
          if ( QString::compare( shares_dock->windowTitle(), list.at( i )->tabText( j ) ) == 0 )
          {
            list.at( i )->setTabTextColor( j, palette().text().color() ) ;
            break;
          }
          else
          {
            continue;
          }
        }

        continue;
      }
      else
      {
        continue;
      }
    }
  }
}

#include "smb4kmainwindow.moc"
