/*
    The main window of Smb4K

    SPDX-FileCopyrightText: 2008-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4kmainwindow.h"
#include "core/smb4kclient.h"
#include "core/smb4kfile.h"
#include "core/smb4kglobal.h"
#include "core/smb4khost.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "core/smb4ksynchronizer.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kworkgroup.h"
#include "smb4kbookmarkmenu.h"
#include "smb4knetworkbrowserdockwidget.h"
#include "smb4kpassworddialog.h"
#include "smb4kprofilesmenu.h"
#include "smb4ksharesviewdockwidget.h"
#include "smb4ksystemtray.h"

// Qt includes
#include <QActionGroup>
#include <QApplication>
#include <QDockWidget>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSize>
#include <QStatusBar>
#include <QString>
#include <QTabBar>
#include <QTimer>
#include <QVariantList>

// KDE includes
#include <KConfigDialog>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KStandardAction>
#include <KXMLGUIFactory>

using namespace Smb4KGlobal;
using namespace KParts;

Smb4KMainWindow::Smb4KMainWindow()
    : KXmlGuiWindow()
    , m_systemTrayWidget(nullptr)
{
    m_focusWidget = nullptr;

    m_passwordDialog = new Smb4KPasswordDialog(this);
    m_timerId = 0;

    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();
    setDockNestingEnabled(true);
    setupActions();
    setupGUI(Default, QStringLiteral("smb4k_shell.rc"));
    setupView();
    setupMenuBar();
    setupStatusBar();
    setupSystemTrayWidget();

    switch (Smb4KSettings::mainWindowTabOrientation()) {
    case Smb4KSettings::EnumMainWindowTabOrientation::Top: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Bottom: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::South);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Left: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Right: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::East);
        break;
    }
    default: {
        break;
    }
    }

    //
    // Apply the main window settings
    //
    setAutoSaveSettings(KConfigGroup(Smb4KSettings::self()->config(), "MainWindow"), true);

    //
    // Save the setting no matter how the application is closed
    //
    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        saveSettings();
    });

    connect(Smb4KClient::self(), &Smb4KClient::requestCredentials, this, &Smb4KMainWindow::slotCredentialsRequested);
    connect(Smb4KMounter::self(), &Smb4KMounter::requestCredentials, this, &Smb4KMainWindow::slotCredentialsRequested);
}

Smb4KMainWindow::~Smb4KMainWindow()
{
}

void Smb4KMainWindow::setupActions()
{
    //
    // Quit action
    //
    QAction *quitAction = KStandardAction::quit(this, &QCoreApplication::quit, actionCollection());
    actionCollection()->addAction(QStringLiteral("quit_action"), quitAction);

    //
    // Configure action
    //
    QAction *configure_action = KStandardAction::preferences(this, SLOT(slotConfigDialog()), actionCollection());
    actionCollection()->addAction(QStringLiteral("configure_action"), configure_action);

    //
    // Dock widgets action menu
    //
    KActionMenu *dock_widgets_menu = new KActionMenu(KDE::icon(QStringLiteral("tab-duplicate")), i18n("Dock Widgets"), actionCollection());
    actionCollection()->addAction(QStringLiteral("dock_widgets_menu"), dock_widgets_menu);

    m_dockWidgets = new QActionGroup(actionCollection());
    m_dockWidgets->setExclusive(false);

    //
    // Bookmarks menu and action
    //
    m_bookmarkMenu = new Smb4KBookmarkMenu(Smb4KBookmarkMenu::MainWindow, this);
    QAction *addBookmarkAction = new QAction(KDE::icon(QStringLiteral("bookmark-new")), i18n("Add &Bookmark"), actionCollection());
    addBookmarkAction->setEnabled(false);
    actionCollection()->addAction(QStringLiteral("bookmarks_menu"), m_bookmarkMenu);
    actionCollection()->addAction(QStringLiteral("bookmark_action"), addBookmarkAction);
    connect(addBookmarkAction, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmarks()));
    connect(m_bookmarkMenu, SIGNAL(addBookmark()), this, SLOT(slotAddBookmarks()));

    //
    // Profiles menu
    //
    Smb4KProfilesMenu *profiles = new Smb4KProfilesMenu(this);
    actionCollection()->addAction(QStringLiteral("profiles_menu"), profiles);
}

void Smb4KMainWindow::setupStatusBar()
{
    // Set up the progress bar.
    m_progressBar = new QProgressBar(statusBar());
    m_progressBar->setFixedWidth(100);
    m_progressBar->setMaximum(0);
    m_progressBar->setMinimum(0);
    m_progressBar->setFixedHeight(statusBar()->fontMetrics().height());
    m_progressBar->setAlignment(Qt::AlignCenter);
    m_progressBar->setVisible(false);

    // Set the icon on the right side that represents the initial
    // state of the wallet manager.
    m_passwordIcon = new QLabel(statusBar());
    m_passwordIcon->setContentsMargins(0, 0, 0, 0);
    m_passwordIcon->setAlignment(Qt::AlignCenter);

    // The feedback icon.
    m_feedbackIcon = new QLabel(statusBar());
    m_feedbackIcon->setContentsMargins(0, 0, 0, 0);
    m_feedbackIcon->setAlignment(Qt::AlignCenter);

    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_feedbackIcon);
    statusBar()->addPermanentWidget(m_passwordIcon);

    slotWalletManagerInitialized();
    setupMountIndicator();

    //
    // Connections
    //
    connect(Smb4KClient::self(), SIGNAL(aboutToStart(NetworkItemPtr, int)), this, SLOT(slotClientAboutToStart(NetworkItemPtr, int)));
    connect(Smb4KClient::self(), SIGNAL(finished(NetworkItemPtr, int)), this, SLOT(slotClientFinished(NetworkItemPtr, int)));

    connect(Smb4KWalletManager::self(), SIGNAL(initialized()), this, SLOT(slotWalletManagerInitialized()));

    connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), this, SLOT(slotVisualMountFeedback(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), this, SLOT(slotVisualUnmountFeedback(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)), this, SLOT(slotMounterAboutToStart(int)));
    connect(Smb4KMounter::self(), SIGNAL(finished(int)), this, SLOT(slotMounterFinished(int)));

    connect(Smb4KSynchronizer::self(), SIGNAL(aboutToStart(QString)), this, SLOT(slotSynchronizerAboutToStart(QString)));
    connect(Smb4KSynchronizer::self(), SIGNAL(finished(QString)), this, SLOT(slotSynchronizerFinished(QString)));
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
    m_networkBrowserDockWidget = new Smb4KNetworkBrowserDockWidget(i18n("Network Neighborhood"), this);
    m_networkBrowserDockWidget->setObjectName(QStringLiteral("NetworkBrowserDockWidget"));
    m_networkBrowserDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_networkBrowserDockWidget->widget()->installEventFilter(this);

    // Connections
    connect(m_networkBrowserDockWidget, SIGNAL(visibilityChanged(bool)), SLOT(slotNetworkBrowserVisibilityChanged(bool)));

    // Add dock widget
    addDockWidget(Qt::LeftDockWidgetArea, m_networkBrowserDockWidget);

    // Insert the toggle view mode action to the action group.
    m_dockWidgets->addAction(m_networkBrowserDockWidget->toggleViewAction());
    static_cast<KActionMenu *>(actionCollection()->action(QStringLiteral("dock_widgets_menu")))->addAction(m_networkBrowserDockWidget->toggleViewAction());

    // Insert the Network menu
    plugActionList(QStringLiteral("network_menu"), m_networkBrowserDockWidget->actionCollection()->actions());

    //
    // Shares view dock widget
    //
    m_sharesViewDockWidget = new Smb4KSharesViewDockWidget(i18n("Mounted Shares"), this);
    m_sharesViewDockWidget->setObjectName(QStringLiteral("SharesViewDockWidget"));
    m_sharesViewDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_sharesViewDockWidget->widget()->installEventFilter(this);

    // Connections
    connect(m_sharesViewDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(slotSharesViewVisibilityChanged(bool)));

    // Add dock widget
    addDockWidget(Qt::LeftDockWidgetArea, m_sharesViewDockWidget);

    // Insert the toggle view mode action to the action group.
    m_dockWidgets->addAction(m_sharesViewDockWidget->toggleViewAction());
    static_cast<KActionMenu *>(actionCollection()->action(QStringLiteral("dock_widgets_menu")))->addAction(m_sharesViewDockWidget->toggleViewAction());

    // Insert the Shares menu
    plugActionList(QStringLiteral("shares_menu"), m_sharesViewDockWidget->actionCollection()->actions());

    // KMessageWidget *messageWidget = new KMessageWidget();
    // messageWidget->setMessageType(KMessageWidget::Positive);
    // messageWidget->setText(i18n("Test ..."));
    //
    // QDockWidget *messageDockWidget = new QDockWidget(this);
    // messageDockWidget->setObjectName(QStringLiteral("MessageDockWidget"));
    // messageDockWidget->setWidget(messageWidget);
    // messageDockWidget->setTitleBarWidget(new QWidget());
    // messageDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // messageDockWidget->setContentsMargins(0, 0, 0, 0);
    //
    // addDockWidget(Qt::TopDockWidgetArea, messageDockWidget);

    //
    // Initial main window look
    //
    KConfigGroup configGroup(Smb4KSettings::self()->config(), "MainWindow");

    if (!configGroup.hasKey(QStringLiteral("FirstStartup"))) {
        QList<QDockWidget *> docks = findChildren<QDockWidget *>();

        for (int i = 1; i < docks.size(); ++i) {
            tabifyDockWidget(docks.at(i - 1), docks.at(i));
        }

        configGroup.writeEntry(QStringLiteral("FirstStartup"), true);
    }
}

void Smb4KMainWindow::setupMenuBar()
{
    // Get the "Bookmarks" menu
    QList<QAction *> actions = menuBar()->actions();
    QListIterator<QAction *> it(actions);

    while (it.hasNext()) {
        QAction *action = it.next();

        if (action->objectName() == QStringLiteral("bookmarks")) {
            action->setMenu(m_bookmarkMenu->menu());
            break;
        } else {
            continue;
        }
    }
}

void Smb4KMainWindow::setupSystemTrayWidget()
{
    if (!m_systemTrayWidget) {
        m_systemTrayWidget = new Smb4KSystemTray(this);
    }

    connect(m_systemTrayWidget, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged(QString)));
}

void Smb4KMainWindow::loadSettings()
{
    //
    // Main window
    //
    switch (Smb4KSettings::mainWindowTabOrientation()) {
    case Smb4KSettings::EnumMainWindowTabOrientation::Top: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Bottom: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::South);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Left: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::West);
        break;
    }
    case Smb4KSettings::EnumMainWindowTabOrientation::Right: {
        setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::East);
        break;
    }
    default: {
        break;
    }
    }

    m_networkBrowserDockWidget->loadSettings();
    m_sharesViewDockWidget->loadSettings();

    m_bookmarkMenu->refreshMenu();

    // Check the state of the password handler and the wallet settings and
    // set the pixmap in the status bar accordingly.
    slotWalletManagerInitialized();

    // Set up the mount indicator icon.
    setupMountIndicator();
}

void Smb4KMainWindow::saveSettings()
{
    m_networkBrowserDockWidget->saveSettings();
    m_sharesViewDockWidget->saveSettings();

    //
    // Save if the main window should be started docked.
    //
    Smb4KSettings::setStartMainWindowDocked(!isVisible());

    //
    // Save the settings
    //
    Smb4KSettings::self()->save();
}

bool Smb4KMainWindow::queryClose()
{
    if (!QApplication::closingDown() && !qApp->isSavingSession() && isVisible()) {
        // This part has been copied from JuK application.
        KMessageBox::information(this,
                                 i18n("<qt>Closing the main window will keep Smb4K running in the system tray.<br>"
                                      "Use <i>Quit</i> from the <i>File</i> menu to quit the application.</qt>"),
                                 i18n("Docking"),
                                 QStringLiteral("DockToSystemTrayInfo"));
        setVisible(false);
        return false;
    } else {
        saveSettings();
        return true;
    }
}

bool Smb4KMainWindow::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type()) {
    case QEvent::FocusIn: {
        //
        // Check if the widget that has the focus belongs to the network
        // browser widget
        //
        QObjectList networkBrowserDockWidgetChildren = m_networkBrowserDockWidget->children();

        for (QObject *object : networkBrowserDockWidgetChildren) {
            if (object == obj) {
                m_focusWidget = m_networkBrowserDockWidget;
                setupDynamicActionList(m_networkBrowserDockWidget);
                break;
            }
        }

        //
        // Check if the widget that has the focus belongs to the shares
        // view
        //
        QObjectList sharesViewDockWidgetChildren = m_sharesViewDockWidget->children();

        for (QObject *object : sharesViewDockWidgetChildren) {
            if (object == obj) {
                m_focusWidget = m_sharesViewDockWidget;
                setupDynamicActionList(m_sharesViewDockWidget);
                break;
            }
        }
        break;
    }
    default: {
        break;
    }
    }

    return KXmlGuiWindow::eventFilter(obj, e);
}

void Smb4KMainWindow::setupMountIndicator()
{
    QStringList overlays;

    if (mountedSharesList().size() == 0) {
        m_feedbackIcon->setToolTip(i18n("There are currently no shares mounted."));
    } else {
        overlays << QStringLiteral("emblem-mounted");
        m_feedbackIcon->setToolTip(i18np("There is currently %1 share mounted.", "There are currently %1 shares mounted.", mountedSharesList().size()));
    }

    m_feedbackIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("folder-network"), KIconLoader::Small, 0, KIconLoader::DefaultState, overlays));
}

void Smb4KMainWindow::setupDynamicActionList(QDockWidget *dock)
{
    if (dock) {
        //
        // Remove all connections to Smb4KMainWindow::slotEnableBookmarkAction() and
        // disable the bookmark action.
        //
        disconnect(this, SLOT(slotEnableBookmarkAction()));
        actionCollection()->action(QStringLiteral("bookmark_action"))->setEnabled(false);

        //
        // Disable the bookmark action
        //
        m_bookmarkMenu->setBookmarkActionEnabled(false);

        //
        // Prepare the dynamic action list for the main window
        //
        QList<QAction *> dynamicList;
        QList<QAction *> actionsList;

        if (dock == m_networkBrowserDockWidget) {
            actionsList = m_networkBrowserDockWidget->actionCollection()->actions();
        } else if (dock == m_sharesViewDockWidget) {
            actionsList = m_sharesViewDockWidget->actionCollection()->actions();
        }

        for (QAction *action : qAsConst(actionsList)) {
            if (action->objectName() == QStringLiteral("bookmark_action")) {
                m_bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
                connect(action, SIGNAL(changed()), this, SLOT(slotEnableBookmarkAction()));
                continue;
            } else if (action->objectName() == QStringLiteral("filemanager_action")) {
                continue;
            } else if (action->objectName() == QStringLiteral("konsole_action")) {
                continue;
            } else if (action->objectName() == QStringLiteral("icon_view_action")) {
                continue;
            } else if (action->objectName() == QStringLiteral("list_view_action")) {
                continue;
            }

            dynamicList << action;
        }

        //
        // Remove old and insert new dynamic action list
        //
        unplugActionList(QStringLiteral("dynamic_list"));
        plugActionList(QStringLiteral("dynamic_list"), dynamicList);
    }
}

void Smb4KMainWindow::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (!m_requestQueue.isEmpty()) {
        if (!m_passwordDialog->isVisible()) {
            NetworkItemPtr networkItem = m_requestQueue.takeFirst();

            if (networkItem && m_passwordDialog->setNetworkItem(networkItem)) {
                m_passwordDialog->show();
            }
        }
    } else {
        killTimer(m_timerId);
        m_timerId = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KMainWindow::slotConfigDialog()
{
    if (KConfigDialog::showDialog(QStringLiteral("ConfigDialog"))) {
        return;
    }

    KPluginMetaData metaData(QStringLiteral("smb4kconfigdialog"));
    KPluginFactory::Result<KPluginFactory> result = KPluginFactory::loadFactory(metaData);

    if (result.errorReason == KPluginFactory::NO_PLUGIN_ERROR) {
        QPointer<KConfigDialog> dlg = result.plugin->create<KConfigDialog>(this);

        if (dlg) {
            connect(dlg, SIGNAL(settingsChanged(QString)), this, SLOT(slotSettingsChanged(QString)), Qt::UniqueConnection);
            connect(dlg, SIGNAL(settingsChanged(QString)), m_systemTrayWidget, SLOT(slotSettingsChanged(QString)), Qt::UniqueConnection);
            dlg->show();
        }
    } else {
        KMessageBox::error(nullptr, result.errorString);
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
    if (m_focusWidget) {
        if (m_focusWidget == m_networkBrowserDockWidget) {
            QAction *action = m_networkBrowserDockWidget->actionCollection()->action(QStringLiteral("bookmark_action"));

            // Only trigger the action if it is enabled
            if (action && action->isEnabled()) {
                action->trigger();
            }
        } else if (m_focusWidget == m_sharesViewDockWidget) {
            QAction *action = m_sharesViewDockWidget->actionCollection()->action(QStringLiteral("bookmark_action"));

            // Only trigger the action if it is enabled
            if (action && action->isEnabled()) {
                action->trigger();
            }
        }
    }
}

void Smb4KMainWindow::slotWalletManagerInitialized()
{
    if (Smb4KWalletManager::self()->useWalletSystem()) {
        if (KIconLoader::global()->hasIcon(QStringLiteral("kwalletmanager"))) {
            m_passwordIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("kwalletmanager"), KIconLoader::Small, 0, KIconLoader::DefaultState));
        } else {
            m_passwordIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("security-high"), KIconLoader::Small, 0, KIconLoader::DefaultState));
        }

        m_passwordIcon->setToolTip(i18n("The wallet is used."));
    } else {
        m_passwordIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-password"), KIconLoader::Small, 0, KIconLoader::DefaultState));
        m_passwordIcon->setToolTip(i18n("The password dialog is used."));
    }
}

void Smb4KMainWindow::slotClientAboutToStart(const NetworkItemPtr &item, int process)
{
    Q_ASSERT(item);

    switch (process) {
    case LookupDomains: {
        statusBar()->showMessage(i18n("Looking for workgroups and domains..."), 0);
        break;
    }
    case LookupDomainMembers: {
        WorkgroupPtr workgroup = item.staticCast<Smb4KWorkgroup>();
        statusBar()->showMessage(i18n("Looking for hosts in domain %1...", workgroup->workgroupName()), 0);
        break;
    }
    case LookupShares: {
        HostPtr host = item.staticCast<Smb4KHost>();
        statusBar()->showMessage(i18n("Looking for shares provided by host %1...", host->hostName()), 0);
        break;
    }
    case LookupFiles: {
        QString message;

        switch (item->type()) {
        case Share: {
            message = i18n("Looking for files and directories in %1...", item.staticCast<Smb4KShare>()->displayString());
            break;
        }
        case FileOrDirectory: {
            FilePtr file = item.staticCast<Smb4KFile>();

            for (const SharePtr &s : sharesList()) {
                if (s->workgroupName() == file->workgroupName() && s->hostName() == file->hostName() && s->shareName() == file->shareName()) {
                    message = i18n("Looking for files and directories in %1...", s->displayString());
                    break;
                }
            }

            break;
        }
        default: {
            break;
        }
        }

        statusBar()->showMessage(message, 0);

        break;
    }
    case WakeUp: {
        statusBar()->showMessage(i18n("Waking up remote hosts..."), 0);
        break;
    }
    case PrintFile: {
        SharePtr share = item.staticCast<Smb4KShare>();
        statusBar()->showMessage(i18n("Sending file to printer %1...", share->displayString()), 0);
        break;
    }
    case NetworkSearch: {
        statusBar()->showMessage(i18n("Searching..."), 0);
        break;
    }
    default: {
        break;
    }
    }

    if (!m_progressBar->isVisible()) {
        m_progressBar->setVisible(true);
    }
}

void Smb4KMainWindow::slotClientFinished(const NetworkItemPtr & /*item*/, int /*process*/)
{
    if (!coreIsRunning()) {
        m_progressBar->setVisible(false);
        m_progressBar->reset();
        statusBar()->showMessage(i18n("Done."), 2000);
    }
}

void Smb4KMainWindow::slotMounterAboutToStart(int process)
{
    // Tell the user which action is performed by the mounter:
    // mounting, unmounting or waking up.
    switch (process) {
    case MountShare: {
        statusBar()->showMessage(i18n("Mounting..."), 0);
        break;
    }
    case UnmountShare: {
        statusBar()->showMessage(i18n("Unmounting..."), 0);
        break;
    }
    case WakeUp: {
        statusBar()->showMessage(i18n("Waking up host..."), 0);
        break;
    }
    default: {
        break;
    }
    }

    if (!m_progressBar->isVisible()) {
        m_progressBar->setVisible(true);
    }
}

void Smb4KMainWindow::slotMounterFinished(int /*process*/)
{
    QTimer::singleShot(250, this, [this]() {
        if (!coreIsRunning()) {
            m_progressBar->setVisible(false);
            m_progressBar->reset();
            statusBar()->showMessage(i18n("Done."), 2000);
        }
    });
}

void Smb4KMainWindow::slotVisualMountFeedback(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        m_feedbackIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-ok"), KIconLoader::Small, 0, KIconLoader::DefaultState));
        m_feedbackIcon->setToolTip(i18n("%1 has been mounted successfully.", share->displayString()));

        QList<QTabBar *> list = findChildren<QTabBar *>();

        for (int i = 0; i < list.size(); ++i) {
            if (list.at(i)->count() != 0) {
                for (int j = 0; j < list.at(i)->count(); ++j) {
                    if (QString::compare(m_sharesViewDockWidget->windowTitle(), list.at(i)->tabText(j)) == 0 && list.at(i)->currentIndex() != j) {
                        list.at(i)->setTabTextColor(j, palette().highlightedText().color());
                        break;
                    }
                }
            }
        }

        QTimer::singleShot(2000, this, SLOT(slotEndVisualFeedback()));
    }
}

void Smb4KMainWindow::slotVisualUnmountFeedback(const SharePtr &share)
{
    Q_ASSERT(share);

    if (share) {
        m_feedbackIcon->setPixmap(KIconLoader::global()->loadIcon(QStringLiteral("dialog-ok"), KIconLoader::Small, 0, KIconLoader::DefaultState));
        m_feedbackIcon->setToolTip(i18n("%1 has been unmounted successfully.", share->displayString()));

        QList<QTabBar *> list = findChildren<QTabBar *>();

        for (int i = 0; i < list.size(); ++i) {
            if (list.at(i)->count() != 0) {
                for (int j = 0; j < list.at(i)->count(); ++j) {
                    if (QString::compare(m_sharesViewDockWidget->windowTitle(), list.at(i)->tabText(j)) == 0 && list.at(i)->currentIndex() != j) {
                        list.at(i)->setTabTextColor(j, palette().highlightedText().color());
                        break;
                    }
                }
            }
        }

        QTimer::singleShot(2000, this, SLOT(slotEndVisualFeedback()));
    }
}

void Smb4KMainWindow::slotEndVisualFeedback()
{
    QList<QTabBar *> list = findChildren<QTabBar *>();

    for (int i = 0; i < list.size(); ++i) {
        if (list.at(i)->count() != 0) {
            for (int j = 0; j < list.at(i)->count(); ++j) {
                if (QString::compare(m_sharesViewDockWidget->windowTitle(), list.at(i)->tabText(j)) == 0) {
                    list.at(i)->setTabTextColor(j, palette().text().color());
                    break;
                }
            }
        }
    }

    setupMountIndicator();
}

void Smb4KMainWindow::slotSynchronizerAboutToStart(const QString &dest)
{
    statusBar()->showMessage(i18n("Synchronizing %1", dest), 0);

    if (!m_progressBar->isVisible()) {
        m_progressBar->setVisible(true);
    }
}

void Smb4KMainWindow::slotSynchronizerFinished(const QString & /*dest*/)
{
    if (!coreIsRunning()) {
        m_progressBar->setVisible(false);
        m_progressBar->reset();
        statusBar()->showMessage(i18n("Done."), 2000);
    }
}

void Smb4KMainWindow::slotEnableBookmarkAction()
{
    //
    // Get the focused widget's 'Add Bookmark' action and read its
    // isEnabled() property. Set the action of the main window and the
    // bookmark menu respectively.
    //
    if (m_focusWidget) {
        if (m_focusWidget == m_networkBrowserDockWidget) {
            QAction *action = m_networkBrowserDockWidget->actionCollection()->action(QStringLiteral("bookmark_action"));

            if (action) {
                // Bookmark action of the main window
                actionCollection()->action(QStringLiteral("bookmark_action"))->setEnabled(action->isEnabled());

                // Bookmark action of the bookmark menu
                m_bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
            }
        } else if (m_focusWidget == m_sharesViewDockWidget) {
            QAction *action = m_sharesViewDockWidget->actionCollection()->action(QStringLiteral("bookmark_action"));

            if (action) {
                // Bookmark action of the main window
                actionCollection()->action(QStringLiteral("bookmark_action"))->setEnabled(action->isEnabled());

                // Bookmark action of the bookmark menu
                m_bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
            }
        }
    }
}

void Smb4KMainWindow::slotNetworkBrowserVisibilityChanged(bool visible)
{
    if (visible) {
        m_networkBrowserDockWidget->widget()->setFocus();
    } else {
        m_networkBrowserDockWidget->widget()->clearFocus();
    }
}

void Smb4KMainWindow::slotSharesViewVisibilityChanged(bool visible)
{
    if (visible) {
        m_sharesViewDockWidget->widget()->setFocus();
    } else {
        m_sharesViewDockWidget->widget()->clearFocus();
    }
}

void Smb4KMainWindow::slotCredentialsRequested(const NetworkItemPtr &networkItem)
{
    m_requestQueue.append(networkItem);

    if (m_timerId == 0) {
        m_timerId = startTimer(500);
    }
}
