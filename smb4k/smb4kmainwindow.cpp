/***************************************************************************
    The main window of Smb4K.
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2016 by Alexander Reinholdt
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
#include "core/smb4ksettings.h"
#include "core/smb4kglobal.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"
#include "core/smb4kscanner.h"
#include "core/smb4kmounter.h"
#include "core/smb4kprint.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kpreviewer.h"
#include "core/smb4ksearch.h"

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
: KParts::MainWindow(), m_system_tray_widget(0)
{
  // Part manager
  m_manager = new KParts::PartManager(this);
  m_manager->setAllowNestedParts(true);
  connect(m_manager, SIGNAL(activePartChanged(KParts::Part*)), SLOT(slotActivePartChanged(KParts::Part*)));

  // Set up main window
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
  // Quit action
  QAction *quit_action = KStandardAction::quit(this, SLOT(slotQuit()), actionCollection());
  actionCollection()->addAction("quit_action", quit_action);

  // Configure action
  QAction *configure_action = KStandardAction::preferences(this, SLOT(slotConfigDialog()), actionCollection());
  actionCollection()->addAction("configure_action", configure_action);

  // Dock widgets action menu
  KActionMenu *dock_widgets_menu = new KActionMenu(KDE::icon("tab-duplicate"), i18n("Dock Widgets"), actionCollection());
  actionCollection()->addAction("dock_widgets_menu", dock_widgets_menu);

  m_dock_widgets = new QActionGroup(actionCollection());
  m_dock_widgets->setExclusive(false);

  // Bookmarks menu and action
  Smb4KBookmarkMenu *bookmarks = new Smb4KBookmarkMenu(Smb4KBookmarkMenu::MainWindow, this, this);
  bookmarks->addBookmarkAction()->setEnabled(false);
  actionCollection()->addAction("bookmarks_menu", bookmarks);
  actionCollection()->addAction("bookmark_action", bookmarks->addBookmarkAction());
  connect(bookmarks->addBookmarkAction(), SIGNAL(triggered(bool)), SLOT(slotAddBookmark()));
  
  // Profiles menu
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
  connect(Smb4KWalletManager::self(), SIGNAL(initialized()),
          this, SLOT(slotWalletManagerInitialized()));
  
  connect(Smb4KMounter::self(), SIGNAL(mounted(Smb4KShare*)),
          this, SLOT(slotVisualMountFeedback(Smb4KShare*)));
  
  connect(Smb4KMounter::self(), SIGNAL(unmounted(Smb4KShare*)),
          this, SLOT(slotVisualUnmountFeedback(Smb4KShare*)));
  
  connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)),
          this, SLOT(slotMounterAboutToStart(int)));
  
  connect(Smb4KMounter::self(), SIGNAL(finished(int)),
          this, SLOT(slotMounterFinished(int)));
  
  connect(Smb4KScanner::self(), SIGNAL(aboutToStart(Smb4KBasicNetworkItem*,int)),
          this, SLOT(slotScannerAboutToStart(Smb4KBasicNetworkItem*,int)));
  
  connect(Smb4KScanner::self(), SIGNAL(finished(Smb4KBasicNetworkItem*,int)),
          this, SLOT(slotScannerFinished(Smb4KBasicNetworkItem*,int)));
  
  connect(Smb4KSearch::self(), SIGNAL(aboutToStart(QString)),
          this, SLOT(slotSearchAboutToStart(QString)));

  connect(Smb4KSearch::self(), SIGNAL(finished(QString)),
          this, SLOT(slotSearchFinished(QString)));

  connect(Smb4KPrint::self(), SIGNAL(aboutToStart(Smb4KShare*)),
          this, SLOT(slotPrintingAboutToStart(Smb4KShare*)));

  connect(Smb4KPrint::self(), SIGNAL(finished(Smb4KShare*)),
          this, SLOT(slotPrintingFinished(Smb4KShare*)));

  connect(Smb4KSynchronizer::self(), SIGNAL(aboutToStart(QString)),
          this, SLOT(slotSynchronizerAboutToStart(QString)));

  connect(Smb4KSynchronizer::self(), SIGNAL(finished(QString)),
          this, SLOT(slotSynchronizerFinished(QString)));

  connect(Smb4KPreviewer::self(), SIGNAL(aboutToStart(Smb4KShare*,QUrl)),
          this, SLOT(slotPreviewerAboutToStart(Smb4KShare*,QUrl)));

  connect(Smb4KPreviewer::self(), SIGNAL(finished(Smb4KShare*,QUrl)),
          this, SLOT(slotPreviewerFinished(Smb4KShare*,QUrl)));
}


void Smb4KMainWindow::setupView()
{
  // There is no active part initially. Set it 0.
  m_active_part = 0;

  // We do not set a central widget, because it causes "problems"
  // with the dock widgets. We have the nested dock widget property
  // set to true, so we can arrange the dock widgets as we like,
  // nonetheless.

  QDockWidget *browser_dock = 0;
  QDockWidget *search_dock = 0;
  QDockWidget *shares_dock = 0;

  //
  // Network browser part
  //
  KPluginLoader browser_loader("smb4knetworkbrowser");
  KPluginFactory *browser_factory = browser_loader.factory();

  if (browser_factory)
  {
    QVariantList args;
    args << QString("bookmark_shortcut=\"false\"");
    args << QString("silent=\"true\"");

    m_browser_part = browser_factory->create<KParts::Part>(this, args);
    m_browser_part->setObjectName("NetworkBrowserPart");

    if (m_browser_part)
    {
      // Add dock widget to the main window.
      browser_dock = new QDockWidget(i18n("Network Neighborhood"), this);
      browser_dock->setObjectName("NetworkBrowserDockWidget");
      browser_dock->setWidget(m_browser_part->widget());
      browser_dock->setAllowedAreas(Qt::LeftDockWidgetArea);
      connect(browser_dock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotNetworkBrowserVisibilityChanged(bool)));

      addDockWidget(Qt::LeftDockWidgetArea, browser_dock);

      // Make the menu and the tool bar working.
      guiFactory()->addClient(m_browser_part);

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction(browser_dock->toggleViewAction());
      static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(browser_dock->toggleViewAction());

      // Add the Part object to the manager
      m_manager->addPart(m_browser_part, false);
    }
    else
    {
      KMessageBox::error(this, i18n("The network browser could not be created."));

      // We will continue without the network browser.
    }
  }
  else
  {
    KMessageBox::error(this, "<qt>"+browser_loader.errorString()+"</qt>");
    QApplication::exit(1);
    return;
  }

  //
  // Network search part
  //
  KPluginLoader search_loader("smb4knetworksearch");
  KPluginFactory *search_factory = search_loader.factory();

  if (search_factory)
  {
    QVariantList args;
    args << QString("silent=\"true\"");

    m_search_part = search_factory->create<KParts::Part>(this, args);
    m_search_part->setObjectName("NetworkSearchPart");

    if (m_search_part)
    {
      // Add dock widget to the main window.
      search_dock = new QDockWidget(i18n("Network Search"), this);
      search_dock->setObjectName("NetworkSearchDockWidget");
      search_dock->setWidget(m_search_part->widget());
      search_dock->setAllowedAreas(Qt::LeftDockWidgetArea);
      connect(search_dock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotSearchDialogVisibilityChanged(bool)));

      addDockWidget(Qt::LeftDockWidgetArea, search_dock);

      // Make the menu and the tool bar working.
      guiFactory()->addClient(m_search_part);

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction(search_dock->toggleViewAction());
      static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(search_dock->toggleViewAction());

      // Add the Part object to the manager
      m_manager->addPart(m_search_part, false);
    }
    else
    {
      KMessageBox::error(this, i18n("The search dialog could not be created."));

      // We will continue without the search dialog.
    }
  }
  else
  {
    KMessageBox::error(this, "<qt>"+search_loader.errorString()+"</qt>");
    QApplication::exit(1);
    return;
  }
  
  //
  // Shares view part
  //
  KPluginLoader shares_loader("smb4ksharesview");
  KPluginFactory *shares_factory = shares_loader.factory();

  if (shares_factory)
  {
    QVariantList args;
    args << QString("bookmark_shortcut=\"false\"");
    args << QString("silent=\"true\"");

    m_shares_part = shares_factory->create<KParts::Part>(this, args);
    m_shares_part->setObjectName("SharesViewPart");

    if (m_shares_part)
    {
      // Add dock widget to the main window.
      shares_dock = new QDockWidget(i18n("Mounted Shares"), this);
      shares_dock->setObjectName("SharesViewDockWidget");
      shares_dock->setWidget(m_shares_part->widget());
      shares_dock->setAllowedAreas(Qt::LeftDockWidgetArea);
      connect(shares_dock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotSharesViewVisibilityChanged(bool)));

      addDockWidget(Qt::LeftDockWidgetArea, shares_dock);

      // Make the menu and the tool bar working.
      guiFactory()->addClient(m_shares_part);

      // Insert the toggle view mode action to the action group.
      m_dock_widgets->addAction(shares_dock->toggleViewAction());
      static_cast<KActionMenu *>(actionCollection()->action("dock_widgets_menu"))->addAction(shares_dock->toggleViewAction());

      // Add the Part object to the manager
      m_manager->addPart(m_shares_part, false);
    }
    else
    {
      KMessageBox::error(this, i18n("The shares view could not be created."));

      // We will continue without the shares view.
    }
  }
  else
  {
    KMessageBox::error(this, "<qt>"+search_loader.errorString()+"</qt>");
    QApplication::exit(1);
    return;
  }
  
  //
  // Initial main window look
  //
  KConfigGroup config_group(Smb4KSettings::self()->config(), "MainWindow");

  if (!config_group.exists())
  {
    QList<QDockWidget *> docks = findChildren<QDockWidget *>();
    
    for (int i = 1; i < docks.size(); ++i)
    {
      tabifyDockWidget(docks.at(i-1), docks.at(i));
    }
    
    // Set the part of the last tabified dock widget active.
    for (int i = 0; i < m_manager->parts().size(); ++i)
    {
      if (m_manager->parts().at(i)->widget() == docks.last()->widget())
      {
        m_manager->setActivePart(m_manager->parts().at(i));
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
    QString active_part = config_group.readEntry("ActivePart", QString());
    
    if (!active_part.isEmpty())
    {
      // Set the part of the last tabified dock widget active.
      for (int i = 0; i < m_manager->parts().size(); ++i)
      {
        if (QString::compare(active_part, m_manager->parts().at(i)->objectName()) == 0)
        {
          m_manager->setActivePart(m_manager->parts().at(i));
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
  // Send custom event to the dock widgets, so that they know
  // that they have to reload their settings.
  if (m_browser_part)
  {
    Smb4KEvent *customBrowserEvent = new Smb4KEvent(Smb4KEvent::LoadSettings);
    QApplication::postEvent(m_browser_part, customBrowserEvent);
  }
  else
  {
    // Do nothing
  }

  if (m_search_part)
  {
    Smb4KEvent *customSearchEvent = new Smb4KEvent(Smb4KEvent::LoadSettings);
    QApplication::postEvent(m_search_part, customSearchEvent);
  }
  else
  {
    // Do nothing
  }

  if (m_shares_part)
  {
    Smb4KEvent *customSharesEvent = new Smb4KEvent(Smb4KEvent::LoadSettings);
    QApplication::postEvent(m_shares_part, customSharesEvent);
  }
  else
  {
    // Do nothing
  }

  // Reload the list of bookmarks.
  Smb4KBookmarkMenu *bookmark_menu = findChild<Smb4KBookmarkMenu *>();

  if (bookmark_menu)
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
  // Save the active part.
  KConfigGroup config_group(Smb4KSettings::self()->config(), "MainWindow");
  
  if (m_manager->activePart())
  {
    config_group.writeEntry("ActivePart", m_manager->activePart()->objectName());
  }
  else
  {
    // Do nothing
  }

  // Save if the main window should be started docked.
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

  m_feedback_icon->setPixmap(KIconLoader::global()->loadIcon("folder-remote", KIconLoader::Small, 0, KIconLoader::DefaultState, overlays));
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


void Smb4KMainWindow::slotAddBookmark()
{
  if (m_active_part)
  {
    Smb4KEvent *customEvent = new Smb4KEvent(Smb4KEvent::AddBookmark);
    QApplication::postEvent(m_active_part, customEvent);
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


void Smb4KMainWindow::slotScannerAboutToStart(Smb4KBasicNetworkItem *item, int process)
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
      Smb4KWorkgroup *workgroup = static_cast<Smb4KWorkgroup *>(item);
      statusBar()->showMessage(i18n("Looking for hosts in domain %1...", workgroup->workgroupName()), 0);
      break;
    }
    case LookupShares:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>(item);
      statusBar()->showMessage(i18n("Looking for shares provided by host %1...", host->hostName()), 0);
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


void Smb4KMainWindow::slotScannerFinished(Smb4KBasicNetworkItem */*item*/, int /*process*/)
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


void Smb4KMainWindow::slotVisualMountFeedback(Smb4KShare *share)
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


void Smb4KMainWindow::slotVisualUnmountFeedback(Smb4KShare *share)
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


void Smb4KMainWindow::slotPrintingAboutToStart(Smb4KShare *printer)
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


void Smb4KMainWindow::slotPrintingFinished(Smb4KShare */*printer*/)
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


void Smb4KMainWindow::slotPreviewerAboutToStart(Smb4KShare *share, const QUrl &/*url*/)
{
  Q_ASSERT(share);

  statusBar()->showMessage(i18n("Retrieving preview from %1...", share->unc()), 0);

  if (!m_progress_bar->isVisible())
  {
    m_progress_bar->setVisible(true);
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotPreviewerFinished(Smb4KShare */*share*/, const QUrl &/*url*/)
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


void Smb4KMainWindow::slotActivePartChanged(KParts::Part *part)
{
  Q_ASSERT(part);

  // First break the connections and disable the actions
  if (m_active_part)
  {
    // Bookmark action
    QAction *bookmark_action = m_active_part->actionCollection()->action("bookmark_action");

    if (bookmark_action)
    {
      disconnect(bookmark_action, SIGNAL(changed()), this, SLOT(slotEnableBookmarkAction()));
      actionCollection()->action("bookmark_action")->setEnabled(false);
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

  // Let m_active_part point to the new active part.
  m_active_part = part;
  
  // Setup actions
  QList<QAction *> dynamic_list;

  for (int i = 0; i < m_active_part->actionCollection()->actions().size(); ++i)
  {
    QAction *action = m_active_part->actionCollection()->action(i);

    if (QString::compare(action->objectName(), "bookmark_action") == 0)
    {
      actionCollection()->action("bookmark_action")->setEnabled(action->isEnabled());
      connect(action, SIGNAL(changed()), this, SLOT(slotEnableBookmarkAction()));
      continue;
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

    dynamic_list << action;
  }

  unplugActionList("dynamic_list");
  plugActionList("dynamic_list", dynamic_list);
}


void Smb4KMainWindow::slotEnableBookmarkAction()
{
  QAction *action = m_active_part->actionCollection()->action("bookmark_action");

  if (action)
  {
    actionCollection()->action("bookmark_action")->setEnabled(action->isEnabled());
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMainWindow::slotNetworkBrowserVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<QDockWidget *>("NetworkBrowserDockWidget");
  
  if (dock)
  {
    if (visible && m_browser_part != m_active_part)
    {
      m_manager->setActivePart(m_browser_part);
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


void Smb4KMainWindow::slotSharesViewVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<QDockWidget *>("SharesViewDockWidget");
  
  if (dock)
  {
    if (visible && m_shares_part != m_active_part)
    {
      m_manager->setActivePart(m_shares_part);
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


void Smb4KMainWindow::slotSearchDialogVisibilityChanged(bool visible)
{
  QDockWidget *dock = findChild<QDockWidget *>("NetworkSearchDockWidget");
  
  if (dock)
  {
    if (visible && m_search_part != m_active_part)
    {
      m_manager->setActivePart(m_search_part);
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

