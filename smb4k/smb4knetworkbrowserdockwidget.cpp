/*
    The network neighborhood browser dock widget

    SPDX-FileCopyrightText: 2018-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworkbrowserdockwidget.h"
#include "core/smb4kbookmarkhandler.h"
#include "core/smb4kclient.h"
#include "core/smb4kcustomoptionsmanager.h"
#include "core/smb4khost.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kworkgroup.h"
#include "smb4knetworkbrowseritem.h"
#include "smb4ktooltip.h"

// Qt includes
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// KDE includes
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KDualAction>
#include <KWidgetsAddons/KGuiItem>

using namespace Smb4KGlobal;

Smb4KNetworkBrowserDockWidget::Smb4KNetworkBrowserDockWidget(const QString &title, QWidget *parent)
    : QDockWidget(title, parent)
{
    //
    // The network browser widget
    //
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(mainWidget);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_networkBrowser = new Smb4KNetworkBrowser(mainWidget);
    m_searchToolBar = new Smb4KNetworkSearchToolBar(mainWidget);
    m_searchToolBar->setVisible(false);

    mainWidgetLayout->addWidget(m_networkBrowser);
    mainWidgetLayout->addWidget(m_searchToolBar);

    setWidget(mainWidget);

    //
    // The action collection
    //
    m_actionCollection = new KActionCollection(this);

    //
    // The context menu
    //
    m_contextMenu = new KActionMenu(this);

    //
    // Search underway?
    //
    m_searchRunning = false;

    //
    // Set up the actions
    //
    setupActions();

    //
    // Load the settings
    //
    loadSettings();

    //
    // Connections
    //
    connect(m_networkBrowser, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenuRequested(QPoint)));
    connect(m_networkBrowser, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(slotItemActivated(QTreeWidgetItem *, int)));
    connect(m_networkBrowser, SIGNAL(itemSelectionChanged()), this, SLOT(slotItemSelectionChanged()));

    connect(m_searchToolBar, SIGNAL(closeSearchBar()), this, SLOT(slotHideSearchToolBar()));
    connect(m_searchToolBar, SIGNAL(search(QString)), this, SLOT(slotPerformSearch(QString)));
    connect(m_searchToolBar, SIGNAL(abort()), this, SLOT(slotStopSearch()));
    connect(m_searchToolBar, SIGNAL(jumpToResult(QString)), this, SLOT(slotJumpToResult(QString)));
    connect(m_searchToolBar, SIGNAL(clearSearchResults()), this, SLOT(slotClearSearchResults()));

    connect(Smb4KClient::self(), SIGNAL(aboutToStart(NetworkItemPtr, int)), this, SLOT(slotClientAboutToStart(NetworkItemPtr, int)));
    connect(Smb4KClient::self(), SIGNAL(finished(NetworkItemPtr, int)), this, SLOT(slotClientFinished(NetworkItemPtr, int)));
    connect(Smb4KClient::self(), SIGNAL(workgroups()), this, SLOT(slotWorkgroups()));
    connect(Smb4KClient::self(), SIGNAL(hosts(WorkgroupPtr)), this, SLOT(slotWorkgroupMembers(WorkgroupPtr)));
    connect(Smb4KClient::self(), SIGNAL(shares(HostPtr)), this, SLOT(slotShares(HostPtr)));
    connect(Smb4KClient::self(), SIGNAL(searchResults(QList<SharePtr>)), this, SLOT(slotSearchResults(QList<SharePtr>)));

    connect(Smb4KMounter::self(), SIGNAL(mounted(SharePtr)), this, SLOT(slotShareMounted(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(unmounted(SharePtr)), this, SLOT(slotShareUnmounted(SharePtr)));
    connect(Smb4KMounter::self(), SIGNAL(aboutToStart(int)), this, SLOT(slotMounterAboutToStart(int)));
    connect(Smb4KMounter::self(), SIGNAL(finished(int)), this, SLOT(slotMounterFinished(int)));
}

Smb4KNetworkBrowserDockWidget::~Smb4KNetworkBrowserDockWidget()
{
}

void Smb4KNetworkBrowserDockWidget::setupActions()
{
    //
    // Rescan and abort dual action
    //
    KDualAction *rescanAbortAction = new KDualAction(this);
    rescanAbortAction->setInactiveIcon(KDE::icon("view-refresh"));
    rescanAbortAction->setInactiveText(i18n("Scan Netwo&rk"));
    rescanAbortAction->setActiveIcon(KDE::icon("process-stop"));
    rescanAbortAction->setActiveText(i18n("&Abort"));
    rescanAbortAction->setAutoToggle(false);
    rescanAbortAction->setEnabled(true);

    connect(rescanAbortAction, SIGNAL(triggered(bool)), this, SLOT(slotRescanAbortActionTriggered(bool)));

    m_actionCollection->addAction("rescan_abort_action", rescanAbortAction);
    m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);

    //
    // Search action
    //
    QAction *searchAction = new QAction(KDE::icon("search"), i18n("&Search"), this);

    connect(searchAction, SIGNAL(triggered(bool)), this, SLOT(slotShowSearchToolBar()));

    m_actionCollection->addAction("search_action", searchAction);
    m_actionCollection->setDefaultShortcut(searchAction, QKeySequence::Find);

    //
    // Separator
    //
    QAction *separator1 = new QAction(this);
    separator1->setSeparator(true);

    m_actionCollection->addAction("network_separator1", separator1);

    //
    // Bookmark action
    //
    QAction *bookmarkAction = new QAction(KDE::icon("bookmark-new"), i18n("Add &Bookmark"), this);
    bookmarkAction->setEnabled(false);

    connect(bookmarkAction, SIGNAL(triggered(bool)), this, SLOT(slotAddBookmark(bool)));

    m_actionCollection->addAction("bookmark_action", bookmarkAction);
    m_actionCollection->setDefaultShortcut(bookmarkAction, QKeySequence(Qt::CTRL + Qt::Key_B));

    //
    // Mount dialog action
    //
    QAction *manualAction = new QAction(KDE::icon("view-form", QStringList("emblem-mounted")), i18n("&Open Mount Dialog"), this);
    manualAction->setEnabled(true);

    connect(manualAction, SIGNAL(triggered(bool)), this, SLOT(slotMountManually(bool)));

    m_actionCollection->addAction("mount_manually_action", manualAction);
    m_actionCollection->setDefaultShortcut(manualAction, QKeySequence(Qt::CTRL + Qt::Key_O));

    //
    // Separator
    //
    QAction *separator2 = new QAction(this);
    separator2->setSeparator(true);

    m_actionCollection->addAction("network_separator2", separator2);

    //
    // Authentication action
    //
    QAction *authAction = new QAction(KDE::icon("dialog-password"), i18n("Au&thentication"), this);
    authAction->setEnabled(false);

    connect(authAction, SIGNAL(triggered(bool)), this, SLOT(slotAuthentication(bool)));

    m_actionCollection->addAction("authentication_action", authAction);
    m_actionCollection->setDefaultShortcut(authAction, QKeySequence(Qt::CTRL + Qt::Key_T));

    //
    // Custom options action
    //
    QAction *customAction = new QAction(KDE::icon("preferences-system-network"), i18n("&Custom Options"), this);
    customAction->setEnabled(false);

    connect(customAction, SIGNAL(triggered(bool)), this, SLOT(slotCustomOptions(bool)));

    m_actionCollection->addAction("custom_action", customAction);
    m_actionCollection->setDefaultShortcut(customAction, QKeySequence(Qt::CTRL + Qt::Key_C));

    //
    // Preview action
    //
    QAction *previewAction = new QAction(KDE::icon("view-list-icons"), i18n("Pre&view"), this);
    previewAction->setEnabled(false);

    connect(previewAction, SIGNAL(triggered(bool)), this, SLOT(slotPreview(bool)));

    m_actionCollection->addAction("preview_action", previewAction);
    m_actionCollection->setDefaultShortcut(previewAction, QKeySequence(Qt::CTRL + Qt::Key_V));

    //
    // Print action
    //
    QAction *printAction = new QAction(KDE::icon("printer"), i18n("&Print File"), this);
    printAction->setEnabled(false);

    connect(printAction, SIGNAL(triggered(bool)), this, SLOT(slotPrint(bool)));

    m_actionCollection->addAction("print_action", printAction);
    m_actionCollection->setDefaultShortcut(printAction, QKeySequence(Qt::CTRL + Qt::Key_P));

    //
    // Mount/unmount action
    //
    KDualAction *mountAction = new KDualAction(this);
    KGuiItem mountItem(i18n("&Mount"), KDE::icon("media-mount"));
    KGuiItem unmountItem(i18n("&Unmount"), KDE::icon("media-eject"));
    mountAction->setActiveGuiItem(mountItem);
    mountAction->setInactiveGuiItem(unmountItem);
    mountAction->setActive(true);
    mountAction->setAutoToggle(false);
    mountAction->setEnabled(false);

    connect(mountAction, SIGNAL(triggered(bool)), this, SLOT(slotMountActionTriggered(bool)));
    connect(mountAction, SIGNAL(activeChanged(bool)), this, SLOT(slotMountActionChanged(bool)));

    m_actionCollection->addAction("mount_action", mountAction);
    m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL + Qt::Key_M));

    //
    // Plug the actions into the context menu
    //
    QList<QAction *> actionsList = m_actionCollection->actions();

    for (QAction *action : qAsConst(actionsList)) {
        m_contextMenu->addAction(action);
    }
}

void Smb4KNetworkBrowserDockWidget::loadSettings()
{
    //
    // Load icon size
    //
    int iconSize = Smb4KSettings::networkBrowserIconSize();
    m_networkBrowser->setIconSize(QSize(iconSize, iconSize));

    //
    // Show/hide columns
    //
    m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::IP, !Smb4KSettings::showIPAddress());
    m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::Type, !Smb4KSettings::showType());
    m_networkBrowser->setColumnHidden(Smb4KNetworkBrowser::Comment, !Smb4KSettings::showComment());

    //
    // Load and apply the positions of the columns
    //
    KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");

    QMap<int, int> map;
    map.insert(configGroup.readEntry("ColumnPositionNetwork", (int)Smb4KNetworkBrowser::Network), Smb4KNetworkBrowser::Network);
    map.insert(configGroup.readEntry("ColumnPositionType", (int)Smb4KNetworkBrowser::Type), Smb4KNetworkBrowser::Type);
    map.insert(configGroup.readEntry("ColumnPositionIP", (int)Smb4KNetworkBrowser::IP), Smb4KNetworkBrowser::IP);
    map.insert(configGroup.readEntry("ColumnPositionComment", (int)Smb4KNetworkBrowser::Comment), Smb4KNetworkBrowser::Comment);

    QMap<int, int>::const_iterator it = map.constBegin();

    while (it != map.constEnd()) {
        if (it.key() != m_networkBrowser->header()->visualIndex(it.value())) {
            m_networkBrowser->header()->moveSection(m_networkBrowser->header()->visualIndex(it.value()), it.key());
        }

        ++it;
    }

    //
    // Apply the completion strings to the search toolbar
    //
    m_searchToolBar->setCompletionStrings(configGroup.readEntry("SearchItemCompletion", QStringList()));

    //
    // Does anything has to be changed with the marked shares?
    //
    for (const SharePtr &share : mountedSharesList()) {
        // We do not need to use slotShareUnmounted() here, too,
        // because slotShareMounted() will take care of everything
        // we need here.
        slotShareMounted(share);
    }

    //
    // Adjust the actions, if needed
    //
    slotItemSelectionChanged();
}

void Smb4KNetworkBrowserDockWidget::saveSettings()
{
    //
    // Save the position of the columns
    //
    KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");
    configGroup.writeEntry("ColumnPositionNetwork", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Network));
    configGroup.writeEntry("ColumnPositionType", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Type));
    configGroup.writeEntry("ColumnPositionIP", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::IP));
    configGroup.writeEntry("ColumnPositionComment", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Comment));

    //
    // Save the completion strings
    //
    configGroup.writeEntry("SearchItemCompletion", m_searchToolBar->completionStrings());

    configGroup.sync();
}

KActionCollection *Smb4KNetworkBrowserDockWidget::actionCollection()
{
    return m_actionCollection;
}

void Smb4KNetworkBrowserDockWidget::slotContextMenuRequested(const QPoint &pos)
{
    m_contextMenu->menu()->popup(m_networkBrowser->viewport()->mapToGlobal(pos));
}

void Smb4KNetworkBrowserDockWidget::slotItemActivated(QTreeWidgetItem *item, int /*column*/)
{
    //
    // Process the activated item
    //
    if (QApplication::keyboardModifiers() == Qt::NoModifier && m_networkBrowser->selectedItems().size() == 1) {
        Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);

        if (browserItem) {
            switch (browserItem->type()) {
            case Workgroup: {
                if (browserItem->isExpanded()) {
                    Smb4KClient::self()->lookupDomainMembers(browserItem->workgroupItem());
                }
                break;
            }
            case Host: {
                if (browserItem->isExpanded()) {
                    Smb4KClient::self()->lookupShares(browserItem->hostItem());
                }
                break;
            }
            case Share: {
                if (!browserItem->shareItem()->isPrinter()) {
                    slotMountActionTriggered(false); // boolean is ignored
                } else {
                    slotPrint(false); // boolean is ignored
                }
                break;
            }
            default: {
                break;
            }
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotItemSelectionChanged()
{
    //
    // Get the selected item
    //
    QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();

    //
    // Enable/disable and/or adjust the actions depending of the number
    // of selected items and their type
    //
    if (items.size() == 1) {
        Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(items.first());

        if (browserItem) {
            switch (browserItem->type()) {
            case Host: {
                //
                // Adjust the actions
                //
                qobject_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setInactiveText(i18n("Scan Compute&r"));
                m_actionCollection->action("bookmark_action")->setEnabled(false);
                m_actionCollection->action("authentication_action")->setEnabled(true);
                m_actionCollection->action("custom_action")->setEnabled(true);
                m_actionCollection->action("preview_action")->setEnabled(false);
                m_actionCollection->action("print_action")->setEnabled(false);
                static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
                m_actionCollection->action("mount_action")->setEnabled(false);
                break;
            }
            case Share: {
                //
                // Adjust the actions
                //
                qobject_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setInactiveText(i18n("Scan Compute&r"));
                m_actionCollection->action("bookmark_action")->setEnabled(!browserItem->shareItem()->isPrinter());
                m_actionCollection->action("authentication_action")->setEnabled(true);
                m_actionCollection->action("custom_action")->setEnabled(!browserItem->shareItem()->isPrinter());
                m_actionCollection->action("preview_action")->setEnabled(!browserItem->shareItem()->isPrinter());
                m_actionCollection->action("print_action")->setEnabled(browserItem->shareItem()->isPrinter());

                if (!browserItem->shareItem()->isPrinter()) {
                    if (!browserItem->shareItem()->isMounted() || (browserItem->shareItem()->isMounted() && browserItem->shareItem()->isForeign())) {
                        static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
                        m_actionCollection->action("mount_action")->setEnabled(true);
                    } else if (browserItem->shareItem()->isMounted() && !browserItem->shareItem()->isForeign()) {
                        static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(false);
                        m_actionCollection->action("mount_action")->setEnabled(true);
                    } else {
                        static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
                        m_actionCollection->action("mount_action")->setEnabled(false);
                    }
                } else {
                    static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
                    m_actionCollection->action("mount_action")->setEnabled(true);
                }
                break;
            }
            default: {
                //
                // Adjust the actions
                //
                qobject_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setInactiveText(i18n("Scan Wo&rkgroup"));
                m_actionCollection->action("bookmark_action")->setEnabled(false);
                m_actionCollection->action("authentication_action")->setEnabled(false);
                m_actionCollection->action("custom_action")->setEnabled(false);
                m_actionCollection->action("preview_action")->setEnabled(false);
                m_actionCollection->action("print_action")->setEnabled(false);
                static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
                m_actionCollection->action("mount_action")->setEnabled(false);
                break;
            }
            }
        }
    } else if (items.size() > 1) {
        //
        // In this case there are only shares selected, because all other items
        // are automatically deselected in extended selection mode.
        //
        // For deciding which function the mount action should have, we use
        // the number of unmounted shares. If that is identical with the items.size(),
        // it will mount the items, otherwise it will unmount them.
        //
        int unmountedShares = items.size();

        for (QTreeWidgetItem *item : qAsConst(items)) {
            Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);

            if (browserItem && browserItem->shareItem()->isMounted() && !browserItem->shareItem()->isForeign()) {
                //
                // Subtract shares mounted by the user
                //
                unmountedShares--;
            }
        }

        //
        // Adjust the actions
        //
        qobject_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setInactiveText(i18n("Scan Netwo&rk"));
        m_actionCollection->action("bookmark_action")->setEnabled(true);
        m_actionCollection->action("authentication_action")->setEnabled(false);
        m_actionCollection->action("custom_action")->setEnabled(false);
        m_actionCollection->action("preview_action")->setEnabled(true);
        m_actionCollection->action("print_action")->setEnabled(false);
        static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(unmountedShares == items.size());
        m_actionCollection->action("mount_action")->setEnabled(true);
    } else {
        //
        // Adjust the actions
        //
        qobject_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"))->setInactiveText(i18n("Scan Netwo&rk"));
        m_actionCollection->action("bookmark_action")->setEnabled(false);
        m_actionCollection->action("authentication_action")->setEnabled(false);
        m_actionCollection->action("custom_action")->setEnabled(false);
        m_actionCollection->action("preview_action")->setEnabled(false);
        m_actionCollection->action("print_action")->setEnabled(false);
        static_cast<KDualAction *>(m_actionCollection->action("mount_action"))->setActive(true);
        m_actionCollection->action("mount_action")->setEnabled(false);
    }
}

void Smb4KNetworkBrowserDockWidget::slotClientAboutToStart(const NetworkItemPtr & /*item*/, int process)
{
    //
    // Get the rescan/abort action
    //
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));

    //
    // Make adjustments
    //
    if (rescanAbortAction) {
        rescanAbortAction->setActive(true);
        m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Cancel);
    }

    //
    // Set the active status of the search tool bar
    //
    if (process == NetworkSearch) {
        m_searchToolBar->setActiveState(true);
    }
}

void Smb4KNetworkBrowserDockWidget::slotClientFinished(const NetworkItemPtr & /*item*/, int process)
{
    //
    // Get the rescan/abort action
    //
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));

    //
    // Make adjustments
    //
    if (rescanAbortAction) {
        rescanAbortAction->setActive(false);
        m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);
    }

    //
    // Set the active status of the search tool bar
    //
    if (process == NetworkSearch) {
        m_searchToolBar->setActiveState(false);
    }
}

void Smb4KNetworkBrowserDockWidget::slotWorkgroups()
{
    if (!workgroupsList().isEmpty()) {
        //
        // Remove obsolete workgroups and update existing ones
        //
        QTreeWidgetItemIterator itemIt(m_networkBrowser, QTreeWidgetItemIterator::All);

        while (*itemIt) {
            Smb4KNetworkBrowserItem *networkItem = static_cast<Smb4KNetworkBrowserItem *>(*itemIt);

            if (networkItem->type() == Workgroup) {
                WorkgroupPtr workgroup = findWorkgroup(networkItem->workgroupItem()->workgroupName());

                if (workgroup) {
                    networkItem->update();

                    // Update the master browser
                    for (int i = 0; i < networkItem->childCount(); ++i) {
                        Smb4KNetworkBrowserItem *host = static_cast<Smb4KNetworkBrowserItem *>(networkItem->child(i));
                        host->update();
                    }
                } else {
                    delete networkItem;
                }
            }

            ++itemIt;
        }

        //
        // Add new workgroups to the tree widget
        //
        for (const WorkgroupPtr &workgroup : workgroupsList()) {
            QList<QTreeWidgetItem *> items = m_networkBrowser->findItems(workgroup->workgroupName(), Qt::MatchFixedString, Smb4KNetworkBrowser::Network);

            if (items.isEmpty()) {
                (void)new Smb4KNetworkBrowserItem(m_networkBrowser, workgroup);
            }
        }

        //
        // Sort the items
        //
        m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);

        //
        // Update the tooltip
        //
        m_networkBrowser->toolTip()->update();
    } else {
        //
        // Clear the tree widget
        //
        m_networkBrowser->clear();
    }
}

void Smb4KNetworkBrowserDockWidget::slotWorkgroupMembers(const WorkgroupPtr &workgroup)
{
    if (workgroup) {
        //
        // Find the right workgroup
        //
        QList<QTreeWidgetItem *> workgroups =
            m_networkBrowser->findItems(workgroup->workgroupName(), Qt::MatchFixedString | Qt::MatchRecursive, Smb4KNetworkBrowser::Network);
        Smb4KNetworkBrowserItem *workgroupItem = nullptr;

        for (QTreeWidgetItem *item : qAsConst(workgroups)) {
            Smb4KNetworkBrowserItem *tempWorkgroup = static_cast<Smb4KNetworkBrowserItem *>(item);

            if (tempWorkgroup->type() == Workgroup && tempWorkgroup->workgroupItem()->workgroupName() == workgroup->workgroupName()) {
                workgroupItem = tempWorkgroup;
                break;
            }
        }

        //
        // Process the hosts
        //
        if (workgroupItem) {
            //
            // Remove obsolete hosts and update existing ones
            //
            QTreeWidgetItemIterator hostIt(workgroupItem);

            while (*hostIt) {
                Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>(*hostIt);

                if (hostItem->type() == Host) {
                    HostPtr host = findHost(hostItem->hostItem()->hostName(), hostItem->hostItem()->workgroupName());

                    if (host) {
                        hostItem->update();
                    } else {
                        delete hostItem;
                    }
                }

                ++hostIt;
            }

            //
            // Add new hosts to the workgroup item and remove obsolete workgroups if
            // necessary. Honor the auto-expand feature.
            //
            QList<HostPtr> members = workgroupMembers(workgroup);

            if (!members.isEmpty()) {
                for (const HostPtr &host : qAsConst(members)) {
                    bool foundHost = false;

                    for (int i = 0; i < workgroupItem->childCount(); ++i) {
                        Smb4KNetworkBrowserItem *hostItem = static_cast<Smb4KNetworkBrowserItem *>(workgroupItem->child(i));

                        if (hostItem->hostItem()->hostName() == host->hostName()) {
                            foundHost = true;
                            break;
                        }
                    }

                    if (!foundHost) {
                        (void)new Smb4KNetworkBrowserItem(workgroupItem, host);
                    }
                }

                //
                // Auto-expand the workgroup item, if applicable
                //
                if (Smb4KSettings::autoExpandNetworkItems() && !workgroupItem->isExpanded() && !m_searchRunning) {
                    m_networkBrowser->expandItem(workgroupItem);
                }
            } else {
                //
                // Remove empty workgroup.
                //
                delete workgroupItem;
            }

            //
            // Sort the items
            //
            m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);

            //
            // Update the tooltip
            //
            m_networkBrowser->toolTip()->update();
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotShares(const HostPtr &host)
{
    if (host) {
        //
        // Find the right host
        //
        QList<QTreeWidgetItem *> hosts = m_networkBrowser->findItems(host->hostName(), Qt::MatchFixedString | Qt::MatchRecursive, Smb4KNetworkBrowser::Network);
        Smb4KNetworkBrowserItem *hostItem = nullptr;

        for (QTreeWidgetItem *item : qAsConst(hosts)) {
            Smb4KNetworkBrowserItem *tempHost = static_cast<Smb4KNetworkBrowserItem *>(item);

            if (tempHost->type() == Host && tempHost->hostItem()->workgroupName() == host->workgroupName()) {
                hostItem = tempHost;
                break;
            }
        }

        //
        // Process the shares
        //
        if (hostItem) {
            //
            // Remove obsolete shares and update existing ones
            //
            QTreeWidgetItemIterator shareIt(hostItem);

            while (*shareIt) {
                Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(*shareIt);

                if (shareItem->type() == Share) {
                    SharePtr share = findShare(shareItem->shareItem()->url(), shareItem->shareItem()->workgroupName());

                    if (share) {
                        shareItem->update();
                    } else {
                        delete shareItem;
                    }
                }

                ++shareIt;
            }

            //
            // Add new shares to the host item. The host will not be removed from the
            // view when it has no shares. Honor the auto-expand feature.
            //
            QList<SharePtr> shares = sharedResources(host);

            if (!shares.isEmpty()) {
                for (const SharePtr &share : qAsConst(shares)) {
                    bool foundShare = false;

                    for (int i = 0; i < hostItem->childCount(); ++i) {
                        Smb4KNetworkBrowserItem *shareItem = static_cast<Smb4KNetworkBrowserItem *>(hostItem->child(i));

                        if (shareItem->shareItem()->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort)
                            == share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort)) {
                            foundShare = true;
                            break;
                        }
                    }

                    if (!foundShare) {
                        (void)new Smb4KNetworkBrowserItem(hostItem, share);
                    }
                }

                //
                // Auto-expand the host item, if applicable
                //
                if (Smb4KSettings::autoExpandNetworkItems() && !hostItem->isExpanded() && !m_searchRunning) {
                    m_networkBrowser->expandItem(hostItem);
                }
            }
        }

        //
        // Sort the items
        //
        m_networkBrowser->sortItems(Smb4KNetworkBrowser::Network, Qt::AscendingOrder);

        //
        // Update the tooltip
        //
        m_networkBrowser->toolTip()->update();
    }
}

void Smb4KNetworkBrowserDockWidget::slotRescanAbortActionTriggered(bool /*checked*/)
{
    //
    // Get the Rescan/Abort action
    //
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action("rescan_abort_action"));

    //
    // Get the selected items
    //
    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    //
    // Perform actions according to the state of the action and the number of
    // selected items.
    //
    if (!rescanAbortAction->isActive()) {
        if (selectedItems.size() == 1) {
            Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(selectedItems.first());

            if (browserItem) {
                switch (browserItem->type()) {
                case Workgroup: {
                    Smb4KClient::self()->lookupDomainMembers(browserItem->workgroupItem());
                    break;
                }
                case Host: {
                    Smb4KClient::self()->lookupShares(browserItem->hostItem());
                    break;
                }
                case Share: {
                    Smb4KNetworkBrowserItem *parentItem = static_cast<Smb4KNetworkBrowserItem *>(browserItem->parent());
                    Smb4KClient::self()->lookupShares(parentItem->hostItem());
                    break;
                }
                default: {
                    break;
                }
                }
            }
        } else {
            //
            // If several items are selected or no selected items,
            // only the network can be scanned.
            //
            Smb4KClient::self()->lookupDomains();
        }
    } else {
        //
        // Stop all actions performed by the client
        //
        if (Smb4KClient::self()->isRunning()) {
            Smb4KClient::self()->abort();
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotAddBookmark(bool /*checked*/)
{
    QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();
    QList<SharePtr> shares;

    if (!items.isEmpty()) {
        for (int i = 0; i < items.size(); ++i) {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

            if (item && item->type() == Share && !item->shareItem()->isPrinter()) {
                shares << item->shareItem();
            }
        }
    } else {
        // No selected items. Just return.
        return;
    }

    if (!shares.isEmpty()) {
        Smb4KBookmarkHandler::self()->addBookmarks(shares);
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountManually(bool /*checked*/)
{
    Smb4KMounter::self()->openMountDialog();
}

void Smb4KNetworkBrowserDockWidget::slotAuthentication(bool /*checked*/)
{
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());

    if (item) {
        switch (item->type()) {
        case Host: {
            Smb4KWalletManager::self()->showPasswordDialog(item->hostItem());
            break;
        }
        case Share: {
            Smb4KWalletManager::self()->showPasswordDialog(item->shareItem());
            break;
        }
        default: {
            break;
        }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotCustomOptions(bool /*checked*/)
{
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());

    if (item) {
        switch (item->type()) {
        case Host: {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->hostItem());
            break;
        }
        case Share: {
            Smb4KCustomOptionsManager::self()->openCustomOptionsDialog(item->shareItem());
            break;
        }
        default: {
            break;
        }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotPreview(bool /*checked*/)
{
    QList<QTreeWidgetItem *> items = m_networkBrowser->selectedItems();

    if (!items.isEmpty()) {
        for (int i = 0; i < items.size(); ++i) {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(items.at(i));

            if (item && item->type() == Share && !item->shareItem()->isPrinter()) {
                Smb4KClient::self()->openPreviewDialog(item->shareItem());
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotPrint(bool /*checked*/)
{
    Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(m_networkBrowser->currentItem());

    if (item && item->shareItem()->isPrinter()) {
        Smb4KClient::self()->openPrintDialog(item->shareItem());
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountActionTriggered(bool /*checked*/)
{
    //
    // Get the selected items
    //
    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    if (selectedItems.size() > 1) {
        //
        // In the case of multiple selected network items, selectedItems()
        // only contains shares. Thus, we do not need to test for the type.
        // For deciding what the mount action is supposed to do, i.e. mount
        // the (remaining) selected unmounted shares or unmounting all selected
        // mounted shares, we use the number of unmounted shares. If that is
        // greater than 0, we mount all shares that need to be mounted, otherwise
        // we unmount all selected shares.
        //
        QList<SharePtr> unmounted, mounted;

        for (QTreeWidgetItem *item : qAsConst(selectedItems)) {
            Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(item);

            if (browserItem && browserItem->shareItem()->isMounted()) {
                mounted << browserItem->shareItem();
            } else if (browserItem && !browserItem->shareItem()->isMounted()) {
                unmounted << browserItem->shareItem();
            }
        }

        if (!unmounted.empty()) {
            // Mount the (remaining) unmounted shares.
            Smb4KMounter::self()->mountShares(unmounted);
        } else {
            // Unmount all shares.
            Smb4KMounter::self()->unmountShares(mounted, m_networkBrowser);
        }
    } else {
        //
        // If only one network item is selected, we need to test for the type
        // of the item. Only in case of a share we need to do something.
        //
        Smb4KNetworkBrowserItem *browserItem = static_cast<Smb4KNetworkBrowserItem *>(selectedItems.first());

        if (browserItem) {
            switch (browserItem->type()) {
            case Share: {
                if (!browserItem->shareItem()->isMounted()) {
                    Smb4KMounter::self()->mountShare(browserItem->shareItem());
                } else {
                    Smb4KMounter::self()->unmountShare(browserItem->shareItem(), false);
                }
                break;
            }
            default: {
                break;
            }
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountActionChanged(bool active)
{
    //
    // Get the mount action
    //
    KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));

    //
    // Change the shortcuts depending on the value of the 'active' argument
    //
    if (mountAction) {
        if (active) {
            m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL + Qt::Key_M));
        } else {
            m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(Qt::CTRL + Qt::Key_U));
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotShareMounted(const SharePtr &share)
{
    QTreeWidgetItemIterator it(m_networkBrowser);

    while (*it) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);

        if (item->type() == Share) {
            if (QString::compare(item->shareItem()->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                == 0) {
                item->update();
                break;
            }
        }

        ++it;
    }
}

void Smb4KNetworkBrowserDockWidget::slotShareUnmounted(const SharePtr &share)
{
    QTreeWidgetItemIterator it(m_networkBrowser);

    while (*it) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(*it);

        if (item->type() == Share) {
            if (QString::compare(item->shareItem()->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 share->url().toString(QUrl::RemoveUserInfo | QUrl::RemovePort),
                                 Qt::CaseInsensitive)
                == 0) {
                item->update();
                break;
            }
        }

        ++it;
    }
}

void Smb4KNetworkBrowserDockWidget::slotMounterAboutToStart(int /*process*/)
{
    //
    // Unused at the moment
    //
}

void Smb4KNetworkBrowserDockWidget::slotMounterFinished(int process)
{
    //
    // Get the mount/unmount action
    //
    KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action("mount_action"));

    //
    // Make adjustments
    //
    if (mountAction) {
        switch (process) {
        case MountShare: {
            mountAction->setActive(false);
            break;
        }
        case UnmountShare: {
            mountAction->setActive(true);
            break;
        }
        default: {
            break;
        }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotShowSearchToolBar()
{
    //
    // Show the search toolbar
    //
    m_searchToolBar->setVisible(true);

    //
    // Set the focus to the search item input
    //
    m_searchToolBar->prepareInput();
}

void Smb4KNetworkBrowserDockWidget::slotHideSearchToolBar()
{
    //
    // Prevent another dock widget from stealing the focus when
    // the search tool bar is hidden
    //
    m_networkBrowser->setFocus();

    //
    // Hide the search toolbar
    //
    m_searchToolBar->setVisible(false);
}

void Smb4KNetworkBrowserDockWidget::slotPerformSearch(const QString &item)
{
    //
    // Prevent another dock widget from stealing the focus when
    // the search item input is disabled
    //
    m_networkBrowser->setFocus();

    //
    // Clear the selections in the network browser
    //
    m_networkBrowser->clearSelection();

    //
    // A global search is underway
    //
    m_searchRunning = true;

    //
    // Start the search
    //
    Smb4KClient::self()->search(item);
}

void Smb4KNetworkBrowserDockWidget::slotStopSearch()
{
    //
    // Stop the network search
    //
    Smb4KClient::self()->abort();

    //
    // A global search finished
    //
    m_searchRunning = false;
}

void Smb4KNetworkBrowserDockWidget::slotSearchResults(const QList<SharePtr> &shares)
{
    //
    // A global search finished
    //
    m_searchRunning = false;

    //
    // Process the search results
    //
    QTreeWidgetItemIterator it(m_networkBrowser);

    while (*it) {
        Smb4KNetworkBrowserItem *networkItem = static_cast<Smb4KNetworkBrowserItem *>(*it);

        if (networkItem->type() == Share) {
            for (const SharePtr &share : shares) {
                if (networkItem->shareItem() == share) {
                    //
                    // Select the search result
                    //
                    networkItem->setSelected(true);

                    //
                    // Expand the branch of the network tree where a search result
                    // was retrieved
                    //
                    if (!networkItem->parent()->isExpanded()) {
                        m_networkBrowser->expandItem(networkItem->parent());
                    }

                    if (!networkItem->parent()->parent()->isExpanded()) {
                        m_networkBrowser->expandItem(networkItem->parent()->parent());
                    }
                }
            }
        }

        it++;
    }

    //
    // Pass the search results to the search toolbar
    //
    m_searchToolBar->setSearchResults(shares);
}

void Smb4KNetworkBrowserDockWidget::slotJumpToResult(const QString &url)
{
    //
    // Find the share item with URL url
    //
    QTreeWidgetItemIterator it(m_networkBrowser);

    while (*it) {
        Smb4KNetworkBrowserItem *networkItem = static_cast<Smb4KNetworkBrowserItem *>(*it);

        if (networkItem->type() == Share && networkItem->shareItem()->url().toString() == url) {
            m_networkBrowser->setCurrentItem(networkItem);
            break;
        }

        it++;
    }
}

void Smb4KNetworkBrowserDockWidget::slotClearSearchResults()
{
    m_networkBrowser->clearSelection();
}
