/*
    The main window of Smb4K

    SPDX-FileCopyrightText: 2008-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include "core/smb4kworkgroup.h"
#include "smb4kbookmarkmenu.h"
#include "smb4knetworkbrowserdockwidget.h"
#include "smb4kpassworddialog.h"
#include "smb4kprofilesmenu.h"
#include "smb4ksharesviewdockwidget.h"
#include "smb4ksystemtray.h"

// Qt includes
#include <QApplication>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>

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

Smb4KMainWindow::Smb4KMainWindow()
    : KXmlGuiWindow()
{
    setStandardToolBarMenuEnabled(true);
    createStandardStatusBarAction();
    setDockNestingEnabled(true);
    setupActions();
    setupGUI(QSize(800, 500), Default, QStringLiteral("smb4k_shell.rc"));
    setupView();
    setupMenuBar();
    setupStatusBar();

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

    m_systemTrayWidget = new Smb4KSystemTray(this);
    m_passwordDialog = new Smb4KPasswordDialog(this);

    m_focusWidget = nullptr;
    m_timerId = 0;
    m_quitting = false;

    KConfigGroup configGroup(Smb4KSettings::self()->config(), QStringLiteral("MainWindow"));
    setAutoSaveSettings(configGroup, true);

    connect(Smb4KClient::self(), &Smb4KClient::requestCredentials, this, &Smb4KMainWindow::slotCredentialsRequested);
    connect(Smb4KMounter::self(), &Smb4KMounter::requestCredentials, this, &Smb4KMainWindow::slotCredentialsRequested);
}

Smb4KMainWindow::~Smb4KMainWindow()
{
}

void Smb4KMainWindow::setupActions()
{
    QAction *quitAction = KStandardAction::quit(this, &Smb4KMainWindow::slotQuit, actionCollection());
    actionCollection()->addAction(QStringLiteral("quit_action"), quitAction);

    QAction *configureAction = KStandardAction::preferences(this, SLOT(slotConfigDialog()), actionCollection());
    actionCollection()->addAction(QStringLiteral("configure_action"), configureAction);

    KActionMenu *dockWidgetsMenu = new KActionMenu(KDE::icon(QStringLiteral("tab-duplicate")), i18n("Dock Widgets"), actionCollection());
    actionCollection()->addAction(QStringLiteral("dock_widgets_menu"), dockWidgetsMenu);

    m_bookmarkMenu = new Smb4KBookmarkMenu(Smb4KBookmarkMenu::MainWindow, this);
    QAction *addBookmarkAction = new QAction(KDE::icon(QStringLiteral("bookmark-new")), i18n("Add &Bookmark"), actionCollection());
    addBookmarkAction->setEnabled(false);
    actionCollection()->addAction(QStringLiteral("bookmarks_menu"), m_bookmarkMenu);
    actionCollection()->addAction(QStringLiteral("bookmark_action"), addBookmarkAction);
    connect(addBookmarkAction, &QAction::triggered, this, &Smb4KMainWindow::slotAddBookmarks);
    connect(m_bookmarkMenu, &Smb4KBookmarkMenu::addBookmark, this, &Smb4KMainWindow::slotAddBookmarks);

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

    // The feedback icon.
    m_feedbackIcon = new QLabel(statusBar());
    m_feedbackIcon->setContentsMargins(0, 0, 0, 0);
    m_feedbackIcon->setAlignment(Qt::AlignCenter);

    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_feedbackIcon);

    setupMountIndicator();

    //
    // Connections
    //
    connect(Smb4KClient::self(), &Smb4KClient::aboutToStart, this, &Smb4KMainWindow::slotClientAboutToStart);
    connect(Smb4KClient::self(), &Smb4KClient::finished, this, &Smb4KMainWindow::slotClientFinished);

    connect(Smb4KMounter::self(), &Smb4KMounter::mounted, this, &Smb4KMainWindow::slotVisualMountFeedback);
    connect(Smb4KMounter::self(), &Smb4KMounter::unmounted, this, &Smb4KMainWindow::slotVisualUnmountFeedback);
    connect(Smb4KMounter::self(), &Smb4KMounter::aboutToStart, this, &Smb4KMainWindow::slotMounterAboutToStart);
    connect(Smb4KMounter::self(), &Smb4KMounter::finished, this, &Smb4KMainWindow::slotMounterFinished);

    connect(Smb4KSynchronizer::self(), &Smb4KSynchronizer::aboutToStart, this, &Smb4KMainWindow::slotSynchronizerAboutToStart);
    connect(Smb4KSynchronizer::self(), &Smb4KSynchronizer::finished, this, &Smb4KMainWindow::slotSynchronizerFinished);
}

void Smb4KMainWindow::setupView()
{
    //
    // We do not set a central widget, because it causes "problems"
    // with the dock widgets. We have the nested dock widget property
    // set to true, so we can arrange the dock widgets as we like,
    // nonetheless.
    //

    KActionMenu *dockWidgetsMenu = qobject_cast<KActionMenu *>(actionCollection()->action(QStringLiteral("dock_widgets_menu")));

    m_networkBrowserDockWidget = new Smb4KNetworkBrowserDockWidget(i18n("Network Neighborhood"), this);
    m_networkBrowserDockWidget->setObjectName(QStringLiteral("NetworkBrowserDockWidget"));
    m_networkBrowserDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_networkBrowserDockWidget->widget()->installEventFilter(this);

    connect(m_networkBrowserDockWidget, &Smb4KNetworkBrowserDockWidget::visibilityChanged, this, &Smb4KMainWindow::slotNetworkBrowserVisibilityChanged);

    addDockWidget(Qt::LeftDockWidgetArea, m_networkBrowserDockWidget);
    dockWidgetsMenu->addAction(m_networkBrowserDockWidget->toggleViewAction());
    plugActionList(QStringLiteral("network_menu"), m_networkBrowserDockWidget->actionCollection()->actions());

    m_sharesViewDockWidget = new Smb4KSharesViewDockWidget(i18n("Mounted Shares"), this);
    m_sharesViewDockWidget->setObjectName(QStringLiteral("SharesViewDockWidget"));
    m_sharesViewDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_sharesViewDockWidget->widget()->installEventFilter(this);

    connect(m_sharesViewDockWidget, &Smb4KSharesViewDockWidget::visibilityChanged, this, &Smb4KMainWindow::slotSharesViewVisibilityChanged);

    addDockWidget(Qt::LeftDockWidgetArea, m_sharesViewDockWidget);
    dockWidgetsMenu->addAction(m_sharesViewDockWidget->toggleViewAction());
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

    KConfigGroup configGroup(Smb4KSettings::self()->config(), QStringLiteral("MainWindow"));

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

void Smb4KMainWindow::loadSettings()
{
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

    // Set up the mount indicator icon.
    setupMountIndicator();
}

void Smb4KMainWindow::saveSettings()
{
    m_networkBrowserDockWidget->saveSettings();
    m_sharesViewDockWidget->saveSettings();

    Smb4KSettings::setStartMainWindowDocked(!isVisible());
    Smb4KSettings::self()->save();
}

bool Smb4KMainWindow::queryClose()
{
    if (!(m_quitting || QCoreApplication::closingDown()) && !qApp->isSavingSession() && isVisible()) {
        // This part has been copied from JuK application.
        KMessageBox::information(this,
                                 i18n("<qt>Closing the main window will keep Smb4K running in the system tray.<br>"
                                      "Use <i>Quit</i> from the <i>File</i> menu to quit the application.</qt>"),
                                 i18n("Docking"),
                                 QStringLiteral("DockToSystemTrayInfo"));
        setVisible(false);
        return false;
    }

    return true;
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

        for (QAction *action : std::as_const(actionsList)) {
            if (action->objectName() == QStringLiteral("bookmark_action")) {
                m_bookmarkMenu->setBookmarkActionEnabled(action->isEnabled());
                connect(action, &QAction::changed, this, &Smb4KMainWindow::slotEnableBookmarkAction);
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

void Smb4KMainWindow::slotQuit()
{
    m_quitting = true;
    saveSettings();
    QCoreApplication::quit();
}

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
            connect(dlg, &KConfigDialog::settingsChanged, this, &Smb4KMainWindow::loadSettings, Qt::UniqueConnection);
            connect(dlg, &KConfigDialog::settingsChanged, m_systemTrayWidget, &Smb4KSystemTray::loadSettings, Qt::UniqueConnection);
            dlg->show();
        }
    } else {
        KMessageBox::error(nullptr, result.errorString);
        return;
    }
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

void Smb4KMainWindow::slotClientAboutToStart(const NetworkItemPtr &item, int process)
{
    Q_ASSERT(item);

    if (Smb4KClient::self()->isRunning()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

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

            for (const SharePtr &share : sharesList()) {
                // FIXME: Use QUrl::matches() here. Additionally, we do not really need the workgroup.
                if (share->workgroupName() == file->workgroupName() && share->hostName() == file->hostName() && share->shareName() == file->shareName()) {
                    message = i18n("Looking for files and directories in %1...", share->displayString());
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

void Smb4KMainWindow::slotClientFinished(const NetworkItemPtr &item, int process)
{
    Q_UNUSED(item);
    Q_UNUSED(process);

    if (!Smb4KClient::self()->isRunning() && !Smb4KMounter::self()->isRunning() && !Smb4KSynchronizer::self()->isRunning()) {
        m_progressBar->setVisible(false);
        m_progressBar->reset();
        statusBar()->showMessage(i18n("Done."), 2000);
    }

    if (!Smb4KClient::self()->isRunning()) {
        QApplication::restoreOverrideCursor();
    }
}

void Smb4KMainWindow::slotMounterAboutToStart(int process)
{
    if (Smb4KMounter::self()->isRunning()) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
    }

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

void Smb4KMainWindow::slotMounterFinished(int process)
{
    Q_UNUSED(process);

    QTimer::singleShot(250, this, [&]() {
        if (!Smb4KClient::self()->isRunning() && !Smb4KMounter::self()->isRunning() && !Smb4KSynchronizer::self()->isRunning()) {
            m_progressBar->setVisible(false);
            m_progressBar->reset();
            statusBar()->showMessage(i18n("Done."), 2000);
        }
    });

    if (!Smb4KMounter::self()->isRunning()) {
        QApplication::restoreOverrideCursor();
    }
}

void Smb4KMainWindow::slotVisualMountFeedback(const SharePtr &share)
{
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

        QTimer::singleShot(2000, this, &Smb4KMainWindow::slotEndVisualFeedback);
    }
}

void Smb4KMainWindow::slotVisualUnmountFeedback(const SharePtr &share)
{
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

        QTimer::singleShot(2000, this, &Smb4KMainWindow::slotEndVisualFeedback);
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

void Smb4KMainWindow::slotSynchronizerAboutToStart(const QString &destination)
{
    statusBar()->showMessage(i18n("Synchronizing %1", destination), 0);

    if (!m_progressBar->isVisible()) {
        m_progressBar->setVisible(true);
    }
}

void Smb4KMainWindow::slotSynchronizerFinished(const QString &destination)
{
    Q_UNUSED(destination);

    if (!Smb4KClient::self()->isRunning() && !Smb4KMounter::self()->isRunning() && !Smb4KSynchronizer::self()->isRunning()) {
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
