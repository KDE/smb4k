/***************************************************************************
    The main window of Smb4K
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2018 by Alexander Reinholdt
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
#include "smb4kmainwindow.h"
#include "smb4ksystemtray.h"
#include "smb4kbookmarkmenu.h"
#include "smb4kprofilesmenu.h"
#include "smb4knetworkbrowserdockwidget.h"
#include "smb4knetworksearchdockwidget.h"
#include "smb4ksharesviewdockwidget.h"
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kfile.h"
#include "core/smb4kmounter.h"
#include "core/smb4kprint.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4ksearch.h"
#include "core/smb4kclient.h"

// Qt includes
#include <QVariantList>
#include <QTimer>
#include <QSize>
#include <QDockWidget>
#include <QMenu>
#include <QActionGroup>
#include <QLabel>
#include <QTabBar>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>

// KDE includes
#include <KConfigWidgets/KStandardAction>
#include <KConfigWidgets/KConfigDialog>
#include <KIconThemes/KIconLoader>
#include <KI18n/KLocalizedString>
#include <KCoreAddons/KPluginLoader>
#include <KCoreAddons/KPluginFactory>
#include <KXmlGui/KXMLGUIFactory>
#include <KWidgetsAddons/KMessageBox>

using namespace Smb4KGlobal;
using namespace KParts;


Smb4KMainWindow::Smb4KMainWindow()
: KXmlGuiWindow(), m_system_tray_widget(0)
{
  //
  // The widget (embedded into the dock widgets) that has the focus
  // 
  m_focusWidget = 0;

  // 
  // Set up main window
  // 
  setStandardToolBarMenuEnabled(true);
  createStandardStatusBarAction();
  setDockNestingEnabled(true);
  setupActions();
  setupGUI(QSize(800, 600), Default, "smb4k_shell.rc");
  setupView();
  setupMenuBar();
  setupStatusBar();
  setupSystemTrayWidget();

  // Apply the main window settings
  setAutoSaveSettings(KConfigGroup(Smb4KSettings::self()->config(), "MainWindow"), true);
}


Smb4KMainWindow::~Smb4KMainWindow()
{
}


void Smb4KMainWindow::setupActions()
{
  // 
  // Quit action
  // 
  QAction *quit_action = KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
  actionCollection()->addAction("quit_action", quit_action);

  // 
  // Configure action
  // 
  QAction *configure_action = KStandardAction::preferences(this, SLOT(slotConfigDialog()), actionCollection());
  actionCollection()->addAction("configure_action", configure_action);

  // 
  // Dock widgets action menu
  // 
  KActionMenu *dock_widgets_menu = new KActionMenu(KDE::icon("tab-duplicate"), i18n("Dock Widgets"), actionCollection());
  actionCollection()->addAction("dock_widgets_menu", dock_widgets_menu);

  m_dockWidgets = new QActionGroup(actionCollection());
  m_dockWidgets->setExclusive(false);

  // 
  // Bookmarks menu and action
  // 
  Smb4KBookmarkMenu *bookmarksMenu = new Smb4KBookmarkMenu(Smb4KBookmarkMenu::MainWindow, this, this);
  QAction *addBookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), actionCollection());
  addBookmarkAction->setEnabled(false);
  actionCollection()->addAction("bookmarks_menu", bookmarksMenu);
  actionCollection()->addAction("bookmark_action", addBookmarkAction);
  connect(addBookmarkAction, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmarks()));
  connect(bookmarksMenu, SIGNAL(addBookmark()), this, SLOT(slotAddBookmarks()));
  
  // 
  // Profiles menu
  // 
  Smb4KProfilesMenu *profiles = new Smb4KProfilesMenu(this);
  actionCollection()->addAction("profiles_menu", profiles);
}


void Smb4KMainWindow::setupStatusBar()
{
  // Set up the progress bar.
  m_progress_bar = new QProgressBar(statusBar());
  m_progress_bar->setFixedWidth(100);
  m_progress_bar->setMaximum(0);
  m_progress_bar->setMinimum(0);
  m_progress_bar->setFixedHeight(statusBar()->fontMetrics().height());
  m_progress_bar->setAlignment(Qt::AlignCenter);
  m_progress_bar->setVisible(false);

  // Set the icon on the right side that represents the initial
  // state of the wallet manager.
  m_pass_icon = new QLabel(statusBar());
  m_pass_icon->setContentsMargins(0, 0, 0, 0);
  m_pass_icon->setAlignment(Qt::AlignCenter);

  // The feedback icon.
  m_feedback_icon = new QLabel(statusBar());
  m_feedback_icon->setContentsMargins(0, 0, 0, 0);
  m_feedback_icon->setAlignment(Qt::AlignCenter);

  statusBar()->addPermanentWidget(m_progress_bar);
  statusBar()->addPermanentWidget(m_feedback_icon);
  statusBar()->addPermanentWidget(m_pass_icon);

  slotWalletManagerInitialized();
  setupMountIndicator();

  //
  // Connections
  //
  connect(Smb4KClient::self(), SIGNAL(aboutToStart(NetworkItemPtr,int)), this, SLOT(slotClientAboutToStart(NetworkItemPtr,int)));
  connect(Smb4KClient::self(), SIGNAL(finished(NetworkItemPtr,int)), this, SLOT(slotClientFinished(NetworkItemPtr,int)));
  
  
  connect(Smb4KWalletManager::self(), SIGNAL(initialized()),
          this, SLOT(slotWalletManagerInitialized()));
  
  connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)),
          this, SLOT(slotVisualMountFeedback(SharePtr)));
  
  connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)),
          this, SLOT(slotVisualUnmountFeedback(SharePtr)));
  
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)),
          this, SLOT(slotMounterAboutToStart(int)));
  
  connect(Smb4KMounter::self(), SIGNAL(finished(int)),
          this, SLOT(slotMounterFinished(int)));
  
  connect(Smb4KSearch::self(), SIGNAL(aboutToStart(QString)),
          this, SLOT(slotSearchAboutToStart(QString)));

  connect(Smb4KSearch::self(), SIGNAL(finished(QString)),
          this, SLOT(slotSearchFinished(QString)));

  connect(Smb4KPrint::self(), SIGNAL(aboutToStart(SharePtr)),
          this, SLOT(slotPrintingAboutToStart(SharePtr)));

  connect(Smb4KPrint::self(), SIGNAL(finished(SharePtr)),
          this, SLOT(slotPrintingFinished(SharePtr)));

  connect(Smb4KSynchronizer::self(), SIGNAL(aboutToStart(QString)),
          this, SLOT(slotSynchronizerAboutToStart(QString)));

  connect(Smb4KSynchronizer::self(), SIGNAL(finished(QString)),
          this, SLOT(slotSynchronizerFinished(QString)));
}


void Smb4KMainWindow::setupView()
{
  //
  // We do not set a central widget, because it causes "problems"
  // with the dock widgets. We have the nested dock widget property
  // set to true, so we can arrange the dock widgets as we like,
  // nonetheless.
  //

  //
  // Network browser dock widget
  //
  Smb4KNetworkBrowserDockWidget *networkBrowserDock = new Smb4KNetworkBrowserDockWidget(i18n("Network Neighborhood"), this);
  networkBrowserDock->setObjectName("NetworkBrowserDockWidget");
  networkBrowserDock->setAllowedAreas(Qt::LeftDockWidgetArea);
 
  // Install event filter
  networkBrowserDock->widget()->installEventFilter(this);
  
  // Connections
  connect(networkBrowserDock, SIGNAL(visibilityChanged(bool)), SLOT(slotNetworkBrowserVisibilityChanged(bool)));
  
  // Add dock widget
  addDockWidget(Qt::LeftDockWidgetArea, networkBrowserDock);
  
  // Insert the toggle view mode action to the action group.
  m_dockWidgets->addAction(networkBrowserDock->toggleViewAction());
  static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(networkBrowserDock->toggleViewAction());
  
  // Insert the Network menu
  plugActionList("network_menu", networkBrowserDock->actionCollection()->actions());

  //
  // Network search dock widget
  //
  Smb4KNetworkSearchDockWidget *networkSearchDock = new Smb4KNetworkSearchDockWidget(i18n("Network Search"), this);
  networkSearchDock->setObjectName("NetworkSearchDockWidget");
  networkSearchDock->setAllowedAreas(Qt::LeftDockWidgetArea);
  
  // Install event filters
  for (QObject *obj : networkSearchDock->widget()->children())
  {
    obj->installEventFilter(this);
  }

  networkSearchDock->widget()->installEventFilter(this);
  
  // Connections
  connect(networkSearchDock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotSearchDialogVisibilityChanged(bool)));
  
  // Add dock widget
  addDockWidget(Qt::LeftDockWidgetArea, networkSearchDock);
  
  // Insert the toggle view mode action to the action group.
  m_dockWidgets->addAction(networkSearchDock->toggleViewAction());
  static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(networkSearchDock->toggleViewAction());
  
  // Insert the Search menu
  plugActionList("search_menu", networkSearchDock->actionCollection()->actions());
  
  //
  // Shares view dock widget
  //
  Smb4KSharesViewDockWidget *sharesViewDock = new Smb4KSharesViewDockWidget(i18n("Mounted Shares"), this);
  sharesViewDock->setObjectName("SharesViewDockWidget");
  sharesViewDock->setAllowedAreas(Qt::LeftDockWidgetArea);
  
  // Install event filter
  sharesViewDock->widget()->installEventFilter(this);
  
  // Connections
  connect(sharesViewDock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotSharesViewVisibilityChanged(bool)));
  
  // Add dock widget
  addDockWidget(Qt::LeftDockWidgetArea, sharesViewDock);
  
  // Insert the toggle view mode action to the action group.
  m_dockWidgets->addAction(sharesViewDock->toggleViewAction());
  static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(sharesViewDock->toggleViewAction());
  
  // Insert the Shares menu
  plugActionList("shares_menu", sharesViewDock->actionCollection()->actions());
  
  //
  // Initial main window look
  //
  KConfigGroup configGroup(Smb4KSettings::self()->config(), "MainWindow");

  if (!configGroup.exists())
  {
    QList<QDockWidget *> docks = findChildren<QDockWidget *>();
    
    for (int i = 1; i < docks.size(); ++i)
    {
      tabifyDockWidget(docks.at(i-1), docks.at(i));
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::setupMenuBar()
{
  // Get the "Bookmarks" menu
  QList<QAction *> actions = menuBar()->actions();
  QListIterator<QAction *> it(actions);

  while (it.hasNext())
  {
    QAction *action = it.next();

    if (QString::compare("bookmarks", action->objectName()) == 0)
    {
      Smb4KBookmarkMenu *menu = static_cast<Smb4KBookmarkMenu *>(actionCollection()->action("bookmarks_menu"));
      action->setMenu(menu->menu());
      break;
    }
    else
    {
      continue;
    }
  }
}


void Smb4KMainWindow::setupSystemTrayWidget()
{
  if (!m_system_tray_widget)
  {
    m_system_tray_widget = new Smb4KSystemTray(this);
  }
  else
  {
    // Do nothing
  }

  connect(m_system_tray_widget, SIGNAL(settingsChanged(QString)),
          this,                 SLOT(slotSettingsChanged(QString)));
}


void Smb4KMainWindow::loadSettings()
{
  //
  // Let the network browser load its settings
  // 
  Smb4KNetworkBrowserDockWidget *networkBrowserDock = findChild<Smb4KNetworkBrowserDockWidget *>();
  
  if (networkBrowserDock)
  {
    networkBrowserDock->loadSettings();
  }
  else
  {
    // Do nothing
  }
  
  //
  // Let the network search widget load its settings
  // 
  Smb4KNetworkSearchDockWidget *networkSearchDock = findChild<Smb4KNetworkSearchDockWidget *>();
  
  if (networkSearchDock)
  {
    networkSearchDock->loadSettings();
  }
  else
  {
    // Do nothing
  }

  // 
  // Let the shares view load its settings
  // 
  Smb4KSharesViewDockWidget *sharesViewDock = findChild<Smb4KSharesViewDockWidget *>();
  
  if (sharesViewDock)
  {
    sharesViewDock->loadSettings();
  }
  else
  {
    // Do nothing
  }
  
  //
  // Reload the list of bookmarks
  // 
  Smb4KBookmarkMenu *bookmarkMenu = findChild<Smb4KBookmarkMenu *>();

  if (bookmarkMenu)
  {
    bookmarkMenu->refreshMenu();
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
  //
  // Save the settings of the network browser
  // 
  Smb4KNetworkBrowserDockWidget *networkBrowserDock = findChild<Smb4KNetworkBrowserDockWidget *>();
  
  if (networkBrowserDock)
  {
    networkBrowserDock->saveSettings();
  }
  else
  {
    // Do nothing
  }
  
  //
  // Save the settings of the network search widget
  // 
  Smb4KNetworkSearchDockWidget *networkSearchDock = findChild<Smb4KNetworkSearchDockWidget *>();
  
  if (networkSearchDock)
  {
    networkSearchDock->saveSettings();
  }
  else
  {
    // Do nothing
  }
  
  // 
  // Let the shares view load its settings
  // 
  Smb4KSharesViewDockWidget *sharesViewDock = findChild<Smb4KSharesViewDockWidget *>();
  
  if (sharesViewDock)
  {
    sharesViewDock->saveSettings();
  }
  else
  {
    // Do nothing
  }
  
  // 
  // Save if the main window should be started docked.
  // 
  Smb4KSettings::setStartMainWindowDocked(!isVisible());
}


bool Smb4KMainWindow::queryClose()
{
  if (!qApp->isSavingSession() && isVisible())
  {
    // This part has been 'stolen' from JuK application.
    KMessageBox::information(this,
            i18n("<qt>Closing the main window will keep Smb4K running in the system tray. "
                  "Use <i>Quit</i> from the <i>File</i> menu to quit the application.</qt>"),
            i18n("Docking"), "DockToSystemTrayInfo");
    setVisible(false);
    return false;
  }
  else
  {
    saveSettings();
    return true;
  }
}


bool Smb4KMainWindow::eventFilter(QObject *obj, QEvent* e)
{
  switch (e->type())
  {
    case QEvent::FocusIn:
    {
      if (QString::compare(obj->metaObject()->className(), "Smb4KNetworkBrowser") == 0 && m_focusWidget != obj)
      {
        m_focusWidget = static_cast<QWidget *>(obj);
        setupDynamicActionList(static_cast<QDockWidget *>(m_focusWidget->parent()));
      }
      else if (QString::compare(obj->metaObject()->className(), "Smb4KNetworkSearch") == 0 && m_focusWidget != obj)
      {
        m_focusWidget = static_cast<QWidget *>(obj);
        setupDynamicActionList(static_cast<QDockWidget *>(m_focusWidget->parent()));
      }
      else if (QString::compare(obj->parent()->metaObject()->className(), "Smb4KNetworkSearch") == 0 && m_focusWidget != obj->parent())
      {
        m_focusWidget = static_cast<QWidget *>(obj->parent());
        setupDynamicActionList(static_cast<QDockWidget *>(m_focusWidget->parent()));
      }
      else if (QString::compare(obj->metaObject()->className(), "Smb4KSharesView") == 0 && m_focusWidget != obj)
      {
        m_focusWidget = static_cast<QWidget *>(obj);
        setupDynamicActionList(static_cast<QDockWidget *>(m_focusWidget->parent()));
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
  
  return KXmlGuiWindow::eventFilter(obj, e);
}


void Smb4KMainWindow::setupMountIndicator()
{
  QStringList overlays;

  if (mountedSharesList().size() == 0)
  {
    m_feedback_icon->setToolTip(i18n("There are currently no shares mounted."));
  }
  else
  {
    overlays.append("emblem-mounted");
    m_feedback_icon->setToolTip(i18np("There is currently %1 share mounted.", "There are currently %1 shares mounted.", mountedSharesList().size()));
  }

  m_feedback_icon->setPixmap(KIconLoader::global()->loadIcon("folder-network", KIconLoader::Small, 0, KIconLoader::DefaultState, overlays));
}


void Smb4KMainWindow::setupDynamicActionList(QDockWidget* dock)
{
  if (dock)
  {
    //
    // Remove all connections to Smb4KMainWindow::slotEnableBookmarkAction() and
    // disable the bookmark action.
    //
    disconnect(this, SLOT(slotEnableBookmarkAction()));
    actionCollection()->action("bookmark_action")->setEnabled(false);

    // 
    // Get also the bookmark menu and disable the bookmark action
    // there, too.
    // 
    Smb4KBookmarkMenu *bookmarkMenu = findChild<Smb4KBookmarkMenu *>();

    if (bookmarkMenu)
    {
      bookmarkMenu->setBookmarkActionEnabled(false);
    }
    else
    {
      // Do nothing
    }
    
    // 
    // Prepare the dynamic action list for the main window
    //
    QList<QAction *> dynamicList;
    KActionCollection *dockActionCollection = 0;
    
    if (dock->objectName() == "NetworkBrowserDockWidget")
    {
      dockActionCollection = static_cast<Smb4KNetworkBrowserDockWidget *>(dock)->actionCollection();
    }
    else if (dock->objectName() == "NetworkSearchDockWidget")
    {
      dockActionCollection = static_cast<Smb4KNetworkSearchDockWidget *>(dock)->actionCollection();
    }
    else if (dock->objectName() == "SharesViewDockWidget")
    {
      dockActionCollection = static_cast<Smb4KSharesViewDockWidget *>(dock)->actionCollection();
    }
    else
    {
      // Do nothing
    }
    
    for (QAction *action : dockActionCollection->actions())
    {
      if (action->objectName() == "bookmark_action")
      {
        if (bookmarkMenu)
        {
          bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
          connect(action, SIGNAL(changed()), this, SLOT(slotEnableBookmarkAction()));
          continue;
        }
        else
        {
          // Do nothing
        }
      }
      else if (QString::compare(action->objectName(), "filemanager_action") == 0)
      {
        continue;
      }
      else if (QString::compare(action->objectName(), "konsole_action") == 0)
      {
        continue;
      }
      else if (QString::compare(action->objectName(), "icon_view_action") == 0)
      {
        continue;
      }
      else if (QString::compare(action->objectName(), "list_view_action") == 0)
      {
        continue;
      }
      else
      {
        // Do nothing
      }
      
      dynamicList << action;
    }
    
    //
    // Remove old and insert new dynamic action list
    // 
    unplugActionList("dynamic_list");
    plugActionList("dynamic_list", dynamicList);
  }
  else
  {
    // Do nothing
  }
}



/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMainWindow::slotQuit()
{
  qApp->quit();
}


void Smb4KMainWindow::slotConfigDialog()
{
  //
  // Check if the configuration dialog exists and try to show it.
  //
  if (KConfigDialog::exists("ConfigDialog"))
  {
    KConfigDialog::showDialog("ConfigDialog");
    return;
  }
  else
  {
    // Do nothing
  }
  
  //
  // If the dialog does not exist, load and show it:
  //
  KPluginLoader loader("smb4kconfigdialog");
  KPluginFactory *configFactory = loader.factory();

  if (configFactory)
  {
    KConfigDialog *dlg = configFactory->create<KConfigDialog>(this);
    
    if (dlg)
    {
      dlg->setObjectName("ConfigDialog");
      connect(dlg, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged(QString)), Qt::UniqueConnection);
      connect(dlg, SIGNAL(settingsChanged(QString)), m_system_tray_widget, SLOT(slotSettingsChanged(QString)), Qt::UniqueConnection);
      dlg->show();
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    KMessageBox::error(0, "<qt>"+loader.errorString()+"</qt>");
    return;
  }
}


void Smb4KMainWindow::slotSettingsChanged(const QString &)
{
  loadSettings();
}


void Smb4KMainWindow::slotAddBookmarks()
{
  //
  // If we have a widget that has the focus, trigger its 'Add Bookmark' 
  // action to add bookmarks.
  // 
  if (m_focusWidget)
  {
    if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KNetworkBrowserDockWidget") == 0)
    {
      Smb4KNetworkBrowserDockWidget *dockWidget = static_cast<Smb4KNetworkBrowserDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        // Only trigger the action if it is enabled
        if (action && action->isEnabled())
        {
          action->trigger();
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
    else if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KNetworkSearchDockWidget") == 0)
    {
      Smb4KNetworkSearchDockWidget *dockWidget = static_cast<Smb4KNetworkSearchDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        // Only trigger the action if it is enabled
        if (action && action->isEnabled())
        {
          action->trigger();
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
    else if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KSharesViewDockWidget") == 0)
    {
      Smb4KSharesViewDockWidget *dockWidget = static_cast<Smb4KSharesViewDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        // Only trigger the action if it is enabled
        if (action && action->isEnabled())
        {
          action->trigger();
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
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotWalletManagerInitialized()
{
  if (Smb4KWalletManager::self()->useWalletSystem())
  {
    if (KIconLoader::global()->hasIcon("kwalletmanager"))
    {
      m_pass_icon->setPixmap(KIconLoader::global()->loadIcon("kwalletmanager",
			     KIconLoader::Small, 0, KIconLoader::DefaultState));
    }
    else
    {
      m_pass_icon->setPixmap(KIconLoader::global()->loadIcon("security-high",
			     KIconLoader::Small, 0, KIconLoader::DefaultState));
    }
    
    m_pass_icon->setToolTip(i18n("The wallet is used."));
  }
  else
  {
    m_pass_icon->setPixmap(KIconLoader::global()->loadIcon("dialog-password",
                           KIconLoader::Small, 0, KIconLoader::DefaultState));
    m_pass_icon->setToolTip(i18n("The password dialog is used."));
  }
}


void Smb4KMainWindow::slotClientAboutToStart(const NetworkItemPtr &item, int process)
{
  Q_ASSERT(item);

  switch (process)
  {
    case LookupDomains:
    {
      statusBar()->showMessage(i18n("Looking for workgroups and domains..."), 0);
      break;
    }
    case LookupDomainMembers:
    {
      WorkgroupPtr workgroup = item.staticCast<Smb4KWorkgroup>();
      statusBar()->showMessage(i18n("Looking for hosts in domain %1...", workgroup->workgroupName()), 0);
      break;
    }
    case LookupShares:
    {
      HostPtr host = item.staticCast<Smb4KHost>();
      statusBar()->showMessage(i18n("Looking for shares provided by host %1...", host->hostName()), 0);
      break;
    }
    case LookupFiles:
    {
      QString message;
      
      switch (item->type())
      {
        case Share:
        {
          message = i18n("Looking for files and directories in %1...", item.staticCast<Smb4KShare>()->displayString());
          break;
        }
        case Directory:
        {
          qDebug() << "Smb4KMainWindow::slotClientAboutToStart(): Use Smb4KGlobal::findShare()";
          
          FilePtr file = item.staticCast<Smb4KFile>();
          
          for (const SharePtr &s : sharesList())
          {
            if (s->workgroupName() == file->workgroupName() && s->hostName() == file->hostName() && s->shareName() == file->shareName())
            {
              message = i18n("Looking for files and directories in %1...", s->displayString());   
              break;
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
      
      statusBar()->showMessage(message, 0);
      
      break;
    }
    case WakeUp:
    {
      statusBar()->showMessage(i18n("Waking up remote hosts..."), 0);
      break;
    }
    default:
    {
      break;
    }
  }

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotClientFinished(const NetworkItemPtr &/*item*/, int /*process*/)
{
  if (!coreIsRunning())
  {
    m_progress_bar->setVisible(false);
    m_progress_bar->reset();
    statusBar()->showMessage(i18n("Done."), 2000);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotMounterAboutToStart(int process)
{
  // Tell the user which action is performed by the mounter:
  // mounting, unmounting or waking up.
  switch (process)
  {
    case MountShare:
    {
      statusBar()->showMessage(i18n("Mounting..."), 0);
      break;
    }
    case UnmountShare:
    {
      statusBar()->showMessage(i18n("Unmounting..."), 0);
      break;
    }
    case WakeUp:
    {
      statusBar()->showMessage(i18n("Waking up host..."), 0);
      break;
    }
    default:
    {
      break;
    }
  }

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotMounterFinished(int /*process*/)
{
  if (!coreIsRunning())
  {
    m_progress_bar->setVisible(false);
    m_progress_bar->reset();
    statusBar()->showMessage(i18n("Done."), 2000);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotVisualMountFeedback(const SharePtr &share)
{
  Q_ASSERT(share);
  
  if (share)
  {
    m_feedback_icon->setPixmap(KIconLoader::global()->loadIcon("dialog-ok",
                              KIconLoader::Small, 0, KIconLoader::DefaultState));
    m_feedback_icon->setToolTip(i18n("%1 has been mounted successfully.", share->unc()));

    QList<QTabBar *> list = findChildren<QTabBar *>();
    QDockWidget *shares_dock = findChild<QDockWidget *>("SharesViewDockWidget");

    if (shares_dock)
    {
      for (int i = 0; i < list.size(); ++i)
      {
        if (list.at(i)->count() != 0)
        {
          for (int j = 0; j < list.at(i)->count(); ++j)
          {
            if (QString::compare(shares_dock->windowTitle(), list.at(i)->tabText(j)) == 0 &&
                list.at(i)->currentIndex() != j)
            {
              list.at(i)->setTabTextColor(j, palette().highlightedText().color()) ;
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

    QTimer::singleShot(2000, this, SLOT(slotEndVisualFeedback()));
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotVisualUnmountFeedback(const SharePtr &share)
{
  Q_ASSERT(share);
  
  if (share)
  {
    m_feedback_icon->setPixmap(KIconLoader::global()->loadIcon("dialog-ok",
                                KIconLoader::Small, 0, KIconLoader::DefaultState));
    m_feedback_icon->setToolTip(i18n("%1 has been unmounted successfully.", share->unc()));

    QList<QTabBar *> list = findChildren<QTabBar *>();
    QDockWidget *shares_dock = findChild<QDockWidget *>("SharesViewDockWidget");

    if (shares_dock)
    {
      for (int i = 0; i < list.size(); ++i)
      {
        if (list.at(i)->count() != 0)
        {
          for (int j = 0; j < list.at(i)->count(); ++j)
          {
            if (QString::compare(shares_dock->windowTitle(), list.at(i)->tabText(j)) == 0 &&
                list.at(i)->currentIndex() != j)
            {
              list.at(i)->setTabTextColor(j, palette().highlightedText().color()) ;
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

    QTimer::singleShot(2000, this, SLOT(slotEndVisualFeedback()));
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotEndVisualFeedback()
{
  QList<QTabBar *> list = findChildren<QTabBar *>();
  QDockWidget *shares_dock = findChild<QDockWidget *>("SharesViewDockWidget");

  if (shares_dock)
  {
    for (int i = 0; i < list.size(); ++i)
    {
      if (list.at(i)->count() != 0)
      {
        for (int j = 0; j < list.at(i)->count(); ++j)
        {
          if (QString::compare(shares_dock->windowTitle(), list.at(i)->tabText(j)) == 0)
          {
            list.at(i)->setTabTextColor(j, palette().text().color()) ;
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


void Smb4KMainWindow::slotSearchAboutToStart(const QString &string)
{
  Q_ASSERT(!string.isEmpty());

  statusBar()->showMessage(i18n("Searching for \"%1\"...", string));

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSearchFinished(const QString &/*string*/)
{
  if (!coreIsRunning())
  {
    m_progress_bar->setVisible(false);
    m_progress_bar->reset();
    statusBar()->showMessage(i18n("Done."), 2000);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPrintingAboutToStart(const SharePtr &printer)
{
  statusBar()->showMessage(i18n("Sending file to printer %1...", printer->unc()), 0);

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPrintingFinished(const SharePtr &/*printer*/)
{
  if (!coreIsRunning())
  {
    m_progress_bar->setVisible(false);
    m_progress_bar->reset();
    statusBar()->showMessage(i18n("Done."), 2000);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSynchronizerAboutToStart(const QString &dest)
{
  statusBar()->showMessage(i18n("Synchronizing %1", dest), 0);

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSynchronizerFinished(const QString &/*dest*/)
{
  if (!coreIsRunning())
  {
    m_progress_bar->setVisible(false);
    m_progress_bar->reset();
    statusBar()->showMessage(i18n("Done."), 2000);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotEnableBookmarkAction()
{
  //
  // Get the focused widget's 'Add Bookmark' action and read its 
  // isEnabled() property. Set the action of the main window and the
  // bookmark menu respectively.
  // 
  if (m_focusWidget)
  {
    if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KNetworkBrowserDockWidget") == 0)
    {
      Smb4KNetworkBrowserDockWidget *dockWidget = static_cast<Smb4KNetworkBrowserDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        if (action)
        {
          // Bookmark action of the main window
          actionCollection()->action("bookmark_action")->setEnabled(action->isEnabled());
          
          // Bookmark action of the bookmark menu
          Smb4KBookmarkMenu *bookmarkMenu = findChild<Smb4KBookmarkMenu *>();
          
          if (bookmarkMenu)
          {
            bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
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
    else if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KNetworkSearchDockWidget") == 0)
    {
      Smb4KNetworkSearchDockWidget *dockWidget = static_cast<Smb4KNetworkSearchDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        if (action)
        {
          // Bookmark action of the main window
          actionCollection()->action("bookmark_action")->setEnabled(action->isEnabled());
          
          // Bookmark action of the bookmark menu
          Smb4KBookmarkMenu *bookmarkMenu = findChild<Smb4KBookmarkMenu *>();
          
          if (bookmarkMenu)
          {
            bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
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
    else if (QString::compare(m_focusWidget->parent()->metaObject()->className(), "Smb4KSharesViewDockWidget") == 0)
    {
      Smb4KSharesViewDockWidget *dockWidget = static_cast<Smb4KSharesViewDockWidget *>(m_focusWidget->parent());
      
      if (dockWidget)
      {
        QAction *action = dockWidget->actionCollection()->action("bookmark_action");
        
        if (action)
        {
          // Bookmark action of the main window
          actionCollection()->action("bookmark_action")->setEnabled(action->isEnabled());
          
          // Bookmark action of the bookmark menu
          Smb4KBookmarkMenu *bookmarkMenu = findChild<Smb4KBookmarkMenu *>();
          
          if (bookmarkMenu)
          {
            bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
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


void Smb4KMainWindow::slotNetworkBrowserVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<Smb4KNetworkBrowserDockWidget *>();
  
  if (dock)
  {
    if (visible)
    {
      dock->widget()->setFocus();
    }
    else
    {
      dock->widget()->clearFocus();
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSharesViewVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<Smb4KSharesViewDockWidget *>();
  
  if (dock)
  {
    if (visible)
    {
      dock->widget()->setFocus();
    }
    else
    {
      dock->widget()->clearFocus();
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotSearchDialogVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<Smb4KNetworkSearchDockWidget *>();
  
  if (dock)
  {
    if (visible)
    {
      dock->widget()->setFocus();
    }
    else
    {
      dock->widget()->clearFocus();
    }
  }
  else
  {
    // Do nothing
  }
}

