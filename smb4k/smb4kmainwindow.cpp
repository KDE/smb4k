/***************************************************************************
    smb4kmainwindow  -  The main window of Smb4K.
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2011 by Alexander Reinholdt
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
#include <QSize>
#include <QVariantList>
#include <QDockWidget>
#include <QMenu>
#include <QActionGroup>
#include <QLabel>
#include <QTimer>
#include <QTabBar>
#include <QMenuBar>

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
#include <kmenubar.h>
#include <kmenu.h>

// application specific includes
#include <smb4kmainwindow.h>
#include <smb4ksystemtray.h>
#include <smb4kbookmarkmenu.h>
#include <core/smb4ksettings.h>
#include <core/smb4kdefs.h>
#include <core/smb4kcore.h>
#include <core/smb4kglobal.h>
#include <core/smb4kwalletmanager.h>
#include <core/smb4kworkgroup.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4kscanner.h>
#include <core/smb4kmounter.h>
#include <core/smb4kprint.h>
#include <core/smb4ksynchronizer.h>
#include <core/smb4kpreviewer.h>
#include <core/smb4ksearch.h>


using namespace Smb4KGlobal;

Smb4KMainWindow::Smb4KMainWindow()
: KParts::MainWindow(), m_system_tray_widget( NULL )
{
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
  m_progress_bar->setMaximum( 0 );
  m_progress_bar->setMinimum( 0 );
  m_progress_bar->setFixedHeight( statusBar()->fontMetrics().height() );
  m_progress_bar->setAlignment( Qt::AlignCenter );
  m_progress_bar->setVisible( false );

  // Set the icon on the right side that represents the initial
  // state of the wallet manager.
  m_pass_icon = new QLabel( statusBar() );
  m_pass_icon->setContentsMargins( 0, 0, 0, 0 );
  m_pass_icon->setAlignment( Qt::AlignCenter );

  // The feedback icon.
  m_feedback_icon = new QLabel( statusBar() );
  m_feedback_icon->setContentsMargins( 0, 0, 0, 0 );
  m_feedback_icon->setAlignment( Qt::AlignCenter );
  
  statusBar()->addPermanentWidget( m_progress_bar );
  statusBar()->addPermanentWidget( m_feedback_icon );
  statusBar()->addPermanentWidget( m_pass_icon );

  slotWalletManagerInitialized();
  setupMountIndicator();

  connect( Smb4KWalletManager::self(), SIGNAL( initialized() ),
           this,                       SLOT( slotWalletManagerInitialized() ) );
           
  connect( Smb4KMounter::self(),       SIGNAL( mounted( Smb4KShare * ) ),
           this,                       SLOT( slotVisualMountFeedback( Smb4KShare * ) ) );
           
  connect( Smb4KMounter::self(),       SIGNAL( unmounted( Smb4KShare * ) ),
           this,                       SLOT( slotVisualUnmountFeedback( Smb4KShare * ) ) );
           
  connect( Smb4KScanner::self(),       SIGNAL( aboutToStart( Smb4KBasicNetworkItem *, int ) ),
           this,                       SLOT( slotScannerAboutToStart( Smb4KBasicNetworkItem *, int ) ) );
      
  connect( Smb4KScanner::self(),       SIGNAL( finished( Smb4KBasicNetworkItem *, int ) ),
           this,                       SLOT( slotScannerFinished( Smb4KBasicNetworkItem *, int ) ) );
           
  connect( Smb4KMounter::self(),       SIGNAL( aboutToStart( Smb4KShare *, int ) ),
           this,                       SLOT( slotMounterAboutToStart( Smb4KShare *, int ) ) );
  
  connect( Smb4KMounter::self(),       SIGNAL( finished( Smb4KShare *, int ) ),
           this,                       SLOT( slotMounterFinished( Smb4KShare *, int ) ) );
           
  connect( Smb4KSearch::self(),        SIGNAL( aboutToStart( const QString & ) ),
           this,                       SLOT( slotSearchAboutToStart( const QString & ) ) );
           
  connect( Smb4KSearch::self(),        SIGNAL( finished( const QString & ) ),
           this,                       SLOT( slotSearchFinished( const QString & ) ) );
           
  connect( Smb4KPrint::self(),         SIGNAL( aboutToStart( Smb4KShare * ) ),
           this,                       SLOT( slotPrintingAboutToStart( Smb4KShare * ) ) );

  connect( Smb4KPrint::self(),         SIGNAL( finished( Smb4KShare * ) ),
           this,                       SLOT( slotPrintingFinished( Smb4KShare * ) ) );

  connect( Smb4KSynchronizer::self(),  SIGNAL( aboutToStart( const QString & ) ),
           this,                       SLOT( slotSynchronizerAboutToStart( const QString & ) ) );

  connect( Smb4KSynchronizer::self(),  SIGNAL( finished( const QString & ) ),
           this,                       SLOT( slotSynchronizerFinished( const QString & ) ) );

  connect( Smb4KPreviewer::self(),     SIGNAL( aboutToStart( Smb4KShare *, const QUrl & ) ),
           this,                       SLOT( slotPreviewerAboutToStart( Smb4KShare *, const QUrl & ) ) );

  connect( Smb4KPreviewer::self(),     SIGNAL( finished( Smb4KShare *, const QUrl & ) ),
           this,                       SLOT( slotPreviewerFinished( Smb4KShare *, const QUrl & ) ) );
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
  KPluginLoader browser_loader( "smb4knetworkbrowser" );
  KPluginFactory *browser_factory = browser_loader.factory();

  if ( browser_factory )
  {
    QVariantList args;
    args << QString( "bookmark_shortcut=\"false\"" );
    args << QString( "silent=\"true\"" );

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
  KPluginLoader search_loader( "smb4knetworksearch" );
  KPluginFactory *search_factory = search_loader.factory();

  if ( search_factory )
  {
    QVariantList args;
    args << QString( "silent=\"true\"" );
    
    m_search_part = search_factory->create<KParts::Part>( this, args );

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

  KPluginLoader shares_loader( "smb4ksharesview" );
  KPluginFactory *shares_factory = shares_loader.factory();

  if ( shares_factory )
  {
    QVariantList args;
    args << QString( "bookmark_shortcut=\"false\"" );
    args << QString( "silent=\"true\"" );

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

  connect( m_system_tray_widget, SIGNAL( settingsChanged( const QString & ) ),
           this,                 SLOT( slotSettingsChanged( const QString & ) ) );
}


void Smb4KMainWindow::setupBookmarksMenu()
{
  // Get the "Bookmarks" menu
  QList<QAction *> actions = menuBar()->actions();
  QListIterator<QAction *> it( actions );
  
  while ( it.hasNext() )
  {
    QAction *action = it.next();
    
    if ( QString::compare( "bookmarks", action->objectName() ) == 0 )
    {
      Smb4KBookmarkMenu *menu = new Smb4KBookmarkMenu( Smb4KBookmarkMenu::MainWindow, this, this );
      action->setMenu( menu->menu() );
      connect( menu->addBookmarkAction(), SIGNAL( triggered( bool ) ), SLOT( slotAddBookmark() ) );
      break;
    }
    else
    {
      continue;
    }
  }
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
  Smb4KBookmarkMenu *bookmark_menu = findChild<Smb4KBookmarkMenu *>();

  if ( bookmark_menu )
  {
    bookmark_menu->refreshMenu();
  }
  else
  {
    // Do nothing
  }

  // Check the state of the password handler and the wallet settings and
  // set the pixmap in the status bar accordingly.
  slotWalletManagerInitialized();
  
  // Set up the mount indicator icon.
  setupMountIndicator();
}


void Smb4KMainWindow::saveSettings()
{
}


bool Smb4KMainWindow::queryClose()
{
  if ( !kapp->sessionSaving() && isVisible() )
  {
    // This part has been 'stolen' from JuK application.
    KMessageBox::information(this,
            i18n( "<p>Closing the main window will keep Smb4K running in the system tray. "
                  "Use 'Quit' from the 'File' menu to quit the application.</p>"),
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


void Smb4KMainWindow::setupMountIndicator()
{
  QStringList overlays;
  
  if ( mountedSharesList().size() == 0 )
  {
    m_feedback_icon->setToolTip( i18n( "There are currently no shares mounted." ) );
  }
  else
  {
    overlays.append( "emblem-mounted" );
    m_feedback_icon->setToolTip( i18np( "There is currently %1 share mounted.", "There are currently %1 shares mounted.", mountedSharesList().size() ) );
  }
  
  m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "folder-remote", KIconLoader::Small, 0, KIconLoader::DefaultState, overlays ) );
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
  KPluginLoader loader( "smb4kconfigdialog" );
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


void Smb4KMainWindow::slotAddBookmark()
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
        m_pass_icon->setToolTip( i18n( "The wallet is in use." ) );
        break;
      }
      default:
      {
        m_pass_icon->setPixmap( KIconLoader::global()->loadIcon( "wallet-closed",
                                KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_pass_icon->setToolTip( i18n( "The wallet is not in use." ) );
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
      m_pass_icon->setToolTip( i18n( "The password dialog mode is used and logins are remembered." ) );
    }
    else
    {
      m_pass_icon->setToolTip( i18n( "The password dialog mode is used and logins are not remembered." ) );
    }
  }
}


void Smb4KMainWindow::slotScannerAboutToStart( Smb4KBasicNetworkItem *item, int process )
{
  Q_ASSERT( item );
  
  switch ( process )
  {
    case Smb4KScanner::LookupDomains:
    {
      statusBar()->showMessage( i18n( "Looking for workgroups and domains..." ), 0 );
      break;
    }
    case Smb4KScanner::LookupDomainMembers:
    {
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>( item );
      statusBar()->showMessage( i18n( "Looking for hosts in domain %1..." ).arg( workgroup->workgroupName() ), 0 );
      break;
    }
    case Smb4KScanner::LookupShares:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      statusBar()->showMessage( i18n( "Looking for shares provided by host %1..." ).arg( host->hostName() ), 0 );
      break;
    }
    case Smb4KScanner::LookupInfo:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>( item );
      statusBar()->showMessage( i18n( "Looking for more information about host %1..." ).arg( host->hostName() ), 0 );
      break;
    }
    default:
    {
      break;
    }
  }
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotScannerFinished( Smb4KBasicNetworkItem */*item*/, int /*process*/ )
{
  if ( !Smb4KCore::self()->isRunning() )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotMounterAboutToStart( Smb4KShare *share, int process )
{
  Q_ASSERT( share );
  
  switch ( process )
  {
    case Smb4KMounter::MountShare:
    {
      statusBar()->showMessage( i18n( "Mounting share %1..." ).arg( share->unc() ), 0 );
      break;
    }
    case Smb4KMounter::UnmountShare:
    {
      statusBar()->showMessage( i18n( "Unmounting share %1..." ).arg( share->unc() ), 0 );
      break;
    }
    default:
    {
      break;
    }
  }
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotVisualMountFeedback( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-ok",
                              KIconLoader::Small, 0, KIconLoader::DefaultState ) );
  m_feedback_icon->setToolTip( i18n( "%1 has been mounted successfully." ).arg( share->unc() ) );
    
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
  
  QTimer::singleShot( 2000, this, SLOT( slotEndVisualFeedback() ) );
}


void Smb4KMainWindow::slotVisualUnmountFeedback( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-ok",
                              KIconLoader::Small, 0, KIconLoader::DefaultState ) );
  m_feedback_icon->setToolTip( i18n( "%1 has been unmounted successfully." ).arg( share->unc() ) );
    
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

  QTimer::singleShot( 2000, this, SLOT( slotEndVisualFeedback() ) );
}


void Smb4KMainWindow::slotMounterFinished( Smb4KShare *share, int process )
{
  Q_ASSERT( share );
  
  // Give visual feedback if the mounting/unmounting failed.
  switch( process )
  {
    case Smb4KMounter::MountShare:
    {
      if ( !share->isMounted() )
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-cancel",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "Mounting %1 failed." ).arg( share->unc() ) );
        
        QTimer::singleShot( 2000, this, SLOT( slotEndVisualFeedback() ) );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    case Smb4KMounter::UnmountShare:
    {
      if ( share->isMounted() )
      {
        m_feedback_icon->setPixmap( KIconLoader::global()->loadIcon( "dialog-cancel",
                                    KIconLoader::Small, 0, KIconLoader::DefaultState ) );
        m_feedback_icon->setToolTip( i18n( "Unmounting %1 failed." ).arg( share->unc() ) );
        
        QTimer::singleShot( 2000, this, SLOT( slotEndVisualFeedback() ) );
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
  
  if ( !Smb4KCore::self()->isRunning()  )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSearchAboutToStart( const QString &string )
{
  Q_ASSERT( !string.isEmpty() );
  
  statusBar()->showMessage( i18n( "Searching for \"%1\"..." ).arg( string ) );
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSearchFinished( const QString &/*string*/ )
{
  if ( !Smb4KCore::self()->isRunning()  )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPrintingAboutToStart( Smb4KShare *printer )
{
  statusBar()->showMessage( i18n( "Sending file to printer %1..." ).arg( printer->unc() ), 0 );
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPrintingFinished( Smb4KShare */*printer*/ )
{
  if ( !Smb4KCore::self()->isRunning()  )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSynchronizerAboutToStart( const QString &dest )
{
  statusBar()->showMessage( i18n( "Synchronizing %1", dest ), 0 );
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSynchronizerFinished( const QString &/*dest*/ )
{
  if ( !Smb4KCore::self()->isRunning()  )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPreviewerAboutToStart( Smb4KShare *share, const QUrl &/*url*/ )
{
  Q_ASSERT( share );
  
  statusBar()->showMessage( i18n( "Retrieving preview from %1..." ).arg( share->unc() ), 0 );
  
  if ( !m_progress_bar->isVisible() )
  {
    m_progress_bar->setVisible( true );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPreviewerFinished( Smb4KShare */*share*/, const QUrl &/*url*/ )
{
  if ( !Smb4KCore::self()->isRunning()  )
  {
    m_progress_bar->setVisible( false );
    m_progress_bar->reset();
    statusBar()->showMessage( i18n( "Done." ), 2000 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotEndVisualFeedback()
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

  setupMountIndicator();
}

#include "smb4kmainwindow.moc"
