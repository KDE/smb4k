/*
    The network neighborhood browser dock widget

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4knetworkbrowserdockwidget.h"
#include "core/smb4kclient.h"
#include "core/smb4khost.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "core/smb4kwalletmanager.h"
#include "core/smb4kworkgroup.h"
#include "smb4kbookmarkdialog.h"
#include "smb4kcustomsettingseditor.h"
#include "smb4kmountdialog.h"
#include "smb4knetworkbrowser.h"
#include "smb4knetworkbrowseritem.h"
#include "smb4knetworksearchtoolbar.h"
#include "smb4kpassworddialog.h"
#include "smb4kpreviewdialog.h"
#include "smb4kprintdialog.h"
#include "smb4ktooltip.h"

// Qt includes
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QPointer>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>

// KDE includes
#include <KDualAction>
#include <KGuiItem>
#include <KIconLoader>
#include <KLocalizedString>

using namespace Smb4KGlobal;

Smb4KNetworkBrowserDockWidget::Smb4KNetworkBrowserDockWidget(const QString &title, QWidget *parent)
    : QDockWidget(title, parent)
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainWidgetLayout = new QVBoxLayout(mainWidget);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_networkBrowser = new Smb4KNetworkBrowser(mainWidget);
    m_searchToolBar = new Smb4KNetworkSearchToolBar(mainWidget);
    m_searchToolBar->setVisible(false);

    mainWidgetLayout->addWidget(m_networkBrowser);
    mainWidgetLayout->addWidget(m_searchToolBar);

    setWidget(mainWidget);

    m_actionCollection = new KActionCollection(this);
    m_contextMenu = new KActionMenu(this);
    m_searchRunning = false;

    setupActions();
    loadSettings();

    connect(m_networkBrowser, &Smb4KNetworkBrowser::customContextMenuRequested, this, &Smb4KNetworkBrowserDockWidget::slotContextMenuRequested);
    connect(m_networkBrowser, &Smb4KNetworkBrowser::itemActivated, this, &Smb4KNetworkBrowserDockWidget::slotItemActivated);
    connect(m_networkBrowser, &Smb4KNetworkBrowser::itemSelectionChanged, this, &Smb4KNetworkBrowserDockWidget::slotItemSelectionChanged);

    connect(m_searchToolBar, &Smb4KNetworkSearchToolBar::closeSearchBar, this, &Smb4KNetworkBrowserDockWidget::slotHideSearchToolBar);
    connect(m_searchToolBar, &Smb4KNetworkSearchToolBar::search, this, &Smb4KNetworkBrowserDockWidget::slotPerformSearch);
    connect(m_searchToolBar, &Smb4KNetworkSearchToolBar::abort, this, &Smb4KNetworkBrowserDockWidget::slotStopSearch);
    connect(m_searchToolBar, &Smb4KNetworkSearchToolBar::jumpToResult, this, &Smb4KNetworkBrowserDockWidget::slotJumpToResult);
    connect(m_searchToolBar, &Smb4KNetworkSearchToolBar::clearSearchResults, this, &Smb4KNetworkBrowserDockWidget::slotClearSearchResults);

    connect(Smb4KClient::self(), &Smb4KClient::aboutToStart, this, &Smb4KNetworkBrowserDockWidget::slotClientAboutToStart);
    connect(Smb4KClient::self(), &Smb4KClient::finished, this, &Smb4KNetworkBrowserDockWidget::slotClientFinished);
    connect(Smb4KClient::self(), &Smb4KClient::workgroups, this, &Smb4KNetworkBrowserDockWidget::slotWorkgroups);
    connect(Smb4KClient::self(), &Smb4KClient::hosts, this, &Smb4KNetworkBrowserDockWidget::slotWorkgroupMembers);
    connect(Smb4KClient::self(), &Smb4KClient::shares, this, &Smb4KNetworkBrowserDockWidget::slotShares);
    connect(Smb4KClient::self(), &Smb4KClient::searchResults, this, &Smb4KNetworkBrowserDockWidget::slotSearchResults);

    connect(Smb4KMounter::self(), &Smb4KMounter::mounted, this, &Smb4KNetworkBrowserDockWidget::slotShareMounted);
    connect(Smb4KMounter::self(), &Smb4KMounter::unmounted, this, &Smb4KNetworkBrowserDockWidget::slotShareUnmounted);
    connect(Smb4KMounter::self(), &Smb4KMounter::aboutToStart, this, &Smb4KNetworkBrowserDockWidget::slotMounterAboutToStart);
    connect(Smb4KMounter::self(), &Smb4KMounter::finished, this, &Smb4KNetworkBrowserDockWidget::slotMounterFinished);
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
    rescanAbortAction->setInactiveIcon(KDE::icon(QStringLiteral("view-refresh")));
    rescanAbortAction->setInactiveText(i18n("Scan Netwo&rk"));
    rescanAbortAction->setActiveIcon(KDE::icon(QStringLiteral("process-stop")));
    rescanAbortAction->setActiveText(i18n("&Abort"));
    rescanAbortAction->setAutoToggle(false);
    rescanAbortAction->setEnabled(true);

    connect(rescanAbortAction, &KDualAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotRescanAbortActionTriggered);

    m_actionCollection->addAction(QStringLiteral("rescan_abort_action"), rescanAbortAction);
    m_actionCollection->setDefaultShortcut(rescanAbortAction, QKeySequence::Refresh);

    //
    // Search action
    //
    QAction *searchAction = new QAction(KDE::icon(QStringLiteral("search")), i18n("&Search"), this);

    connect(searchAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotShowSearchToolBar);

    m_actionCollection->addAction(QStringLiteral("search_action"), searchAction);
    m_actionCollection->setDefaultShortcut(searchAction, QKeySequence::Find);

    //
    // Separator
    //
    QAction *separator1 = new QAction(this);
    separator1->setSeparator(true);

    m_actionCollection->addAction(QStringLiteral("network_separator1"), separator1);

    //
    // Bookmark action
    //
    QAction *bookmarkAction = new QAction(KDE::icon(QStringLiteral("bookmark-new")), i18n("Add &Bookmark"), this);
    bookmarkAction->setEnabled(false);

    connect(bookmarkAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotAddBookmark);

    m_actionCollection->addAction(QStringLiteral("bookmark_action"), bookmarkAction);
    m_actionCollection->setDefaultShortcut(bookmarkAction, QKeySequence(i18n("Ctrl+B")));

    //
    // Add custom options action
    //
    QAction *customAction = new QAction(KDE::icon(QStringLiteral("settings-configure")), i18n("Add &Custom Settings"), this);
    customAction->setEnabled(false);

    connect(customAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotAddCustomSettings);

    m_actionCollection->addAction(QStringLiteral("custom_action"), customAction);
    m_actionCollection->setDefaultShortcut(customAction, QKeySequence(i18n("Ctrl+C")));

    //
    // Mount dialog action
    //
    QAction *manualAction =
        new QAction(KDE::icon(QStringLiteral("view-form"), QStringList(QStringLiteral("emblem-mounted"))), i18n("&Open Mount Dialog"), this);
    manualAction->setEnabled(true);

    connect(manualAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotMountManually);

    m_actionCollection->addAction(QStringLiteral("mount_manually_action"), manualAction);
    m_actionCollection->setDefaultShortcut(manualAction, QKeySequence(i18n("Ctrl+O")));

    //
    // Separator
    //
    QAction *separator2 = new QAction(this);
    separator2->setSeparator(true);

    m_actionCollection->addAction(QStringLiteral("network_separator2"), separator2);

    //
    // Authentication action
    //
    QAction *authAction = new QAction(KDE::icon(QStringLiteral("dialog-password")), i18n("Au&thentication"), this);
    authAction->setEnabled(false);

    connect(authAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotAuthentication);

    m_actionCollection->addAction(QStringLiteral("authentication_action"), authAction);
    m_actionCollection->setDefaultShortcut(authAction, QKeySequence(i18n("Ctrl+T")));

    //
    // Preview action
    //
    QAction *previewAction = new QAction(KDE::icon(QStringLiteral("view-list-icons")), i18n("Pre&view"), this);
    previewAction->setEnabled(false);

    connect(previewAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotPreview);

    m_actionCollection->addAction(QStringLiteral("preview_action"), previewAction);
    m_actionCollection->setDefaultShortcut(previewAction, QKeySequence(i18n("Ctrl+V")));

    //
    // Print action
    //
    QAction *printAction = new QAction(KDE::icon(QStringLiteral("printer")), i18n("&Print File"), this);
    printAction->setEnabled(false);

    connect(printAction, &QAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotPrint);

    m_actionCollection->addAction(QStringLiteral("print_action"), printAction);
    m_actionCollection->setDefaultShortcut(printAction, QKeySequence(i18n("Ctrl+P")));

    //
    // Mount/unmount action
    //
    KDualAction *mountAction = new KDualAction(this);
    KGuiItem mountItem(i18n("&Mount"), KDE::icon(QStringLiteral("media-mount")));
    KGuiItem unmountItem(i18n("&Unmount"), KDE::icon(QStringLiteral("media-eject")));
    mountAction->setActiveGuiItem(mountItem);
    mountAction->setInactiveGuiItem(unmountItem);
    mountAction->setActive(true);
    mountAction->setAutoToggle(false);
    mountAction->setEnabled(false);

    connect(mountAction, &KDualAction::triggered, this, &Smb4KNetworkBrowserDockWidget::slotMountActionTriggered);
    connect(mountAction, &KDualAction::activeChanged, this, &Smb4KNetworkBrowserDockWidget::slotMountActionChanged);

    m_actionCollection->addAction(QStringLiteral("mount_action"), mountAction);
    m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(i18n("Ctrl+M")));

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

    KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");

    if (configGroup.exists()) {
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
    }

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");

    if (completionGroup.exists()) {
        m_searchToolBar->setCompletionItems(completionGroup.readEntry("SearchItemCompletion", QStringList()));
    }

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
    KConfigGroup configGroup(Smb4KSettings::self()->config(), "NetworkBrowserPart");
    configGroup.writeEntry("ColumnPositionNetwork", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Network));
    configGroup.writeEntry("ColumnPositionType", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Type));
    configGroup.writeEntry("ColumnPositionIP", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::IP));
    configGroup.writeEntry("ColumnPositionComment", m_networkBrowser->header()->visualIndex(Smb4KNetworkBrowser::Comment));
    configGroup.sync();

    KConfigGroup completionGroup(Smb4KSettings::self()->config(), "CompletionItems");
    completionGroup.writeEntry("SearchItemCompletion", m_searchToolBar->completionItems());
    completionGroup.sync();
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
    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    if (selectedItems.size() > 1) {
        //
        // In this case there are only shares selected, because all other items
        // are automatically deselected in extended selection mode.
        //
        // For deciding which function the mount action should have, we use
        // the number of unmounted shares. If that is identical with the items.size(),
        // it will mount the items, otherwise it will unmount them.
        //
        // The print action will be enabled when at least one printer was marked.
        //
        int unmountedShares = selectedItems.size();
        int printerShares = 0;

        for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

            if (item) {
                if (item->shareItem()->isMounted() && !item->shareItem()->isForeign()) {
                    unmountedShares--;
                }

                if (item->shareItem()->isPrinter()) {
                    printerShares++;
                }
            }
        }

        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")))->setInactiveText(i18n("Scan Netwo&rk"));
        m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("authentication_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("preview_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("print_action"))->setEnabled(printerShares != 0);
        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(unmountedShares == selectedItems.size());
        m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(true);
    } else if (selectedItems.size() == 1) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItems.first());

        if (item) {
            switch (item->type()) {
            case Host: {
                qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")))->setInactiveText(i18n("Scan Compute&r"));
                m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("authentication_action"))->setEnabled(true);
                m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(true);
                m_actionCollection->action(QStringLiteral("preview_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("print_action"))->setEnabled(false);
                qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
                m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(false);
                break;
            }
            case Share: {
                qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")))->setInactiveText(i18n("Scan Compute&r"));
                m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(!item->shareItem()->isPrinter());
                m_actionCollection->action(QStringLiteral("authentication_action"))->setEnabled(true);
                m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(!item->shareItem()->isPrinter());
                m_actionCollection->action(QStringLiteral("preview_action"))->setEnabled(!item->shareItem()->isPrinter());
                m_actionCollection->action(QStringLiteral("print_action"))->setEnabled(item->shareItem()->isPrinter());

                if (!item->shareItem()->isPrinter()) {
                    if (!item->shareItem()->isMounted() || item->shareItem()->isForeign()) {
                        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
                        m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(true);
                    } else if (item->shareItem()->isMounted() && !item->shareItem()->isForeign()) {
                        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(false);
                        m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(true);
                    } else {
                        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
                        m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(false);
                    }
                } else {
                    qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
                    m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(false);
                }
                break;
            }
            default: {
                qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")))->setInactiveText(i18n("Scan Wo&rkgroup"));
                m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("authentication_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("preview_action"))->setEnabled(false);
                m_actionCollection->action(QStringLiteral("print_action"))->setEnabled(false);
                qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
                m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(false);
                break;
            }
            }
        }
    } else {
        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")))->setInactiveText(i18n("Scan Netwo&rk"));
        m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("authentication_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("preview_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("print_action"))->setEnabled(false);
        qobject_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")))->setActive(true);
        m_actionCollection->action(QStringLiteral("mount_action"))->setEnabled(false);
    }
}

void Smb4KNetworkBrowserDockWidget::slotClientAboutToStart(const NetworkItemPtr & /*item*/, int process)
{
    //
    // Get the rescan/abort action
    //
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")));

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
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")));

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
    }
}

void Smb4KNetworkBrowserDockWidget::slotRescanAbortActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    //
    // Get the Rescan/Abort action
    //
    KDualAction *rescanAbortAction = static_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("rescan_abort_action")));

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

void Smb4KNetworkBrowserDockWidget::slotAddBookmark(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();
    QList<SharePtr> shares;

    if (!selectedItems.isEmpty()) {
        for (QTreeWidgetItem *selectedItem : selectedItems) {
            Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

            if (item && item->type() == Share && !item->shareItem()->isPrinter()) {
                shares << item->shareItem();
            }
        }
    } else {
        return;
    }

    if (!shares.isEmpty()) {
        QPointer<Smb4KBookmarkDialog> bookmarkDialog = new Smb4KBookmarkDialog();

        if (bookmarkDialog->setShares(shares)) {
            bookmarkDialog->open();
        } else {
            delete bookmarkDialog;
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountManually(bool checked)
{
    Q_UNUSED(checked);

    QPointer<Smb4KMountDialog> mountDialog = new Smb4KMountDialog();
    mountDialog->open();
}

void Smb4KNetworkBrowserDockWidget::slotAuthentication(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

        if (item) {
            QPointer<Smb4KPasswordDialog> passwordDialog = new Smb4KPasswordDialog();

            if (passwordDialog->setNetworkItem(item->networkItem())) {
                passwordDialog->open();
            } else {
                delete passwordDialog;
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotAddCustomSettings(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

        QPointer<Smb4KCustomSettingsEditor> customSettingsEditor = new Smb4KCustomSettingsEditor();
        if (customSettingsEditor->setNetworkItem(item->networkItem())) {
            customSettingsEditor->open();
        } else {
            delete customSettingsEditor;
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotPreview(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

        if (item && item->type() == Share && !item->shareItem()->isPrinter()) {
            QPointer<Smb4KPreviewDialog> previewDialog = new Smb4KPreviewDialog();

            if (previewDialog->setShare(item->shareItem())) {
                previewDialog->open();
            } else {
                delete previewDialog;
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotPrint(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();

    for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

        if (item && item->shareItem()->isPrinter()) {
            QPointer<Smb4KPrintDialog> printDialog = new Smb4KPrintDialog();

            if (printDialog->setPrinterShare(item->shareItem())) {
                printDialog->open();
            } else {
                delete printDialog;
            }
        }
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QTreeWidgetItem *> selectedItems = m_networkBrowser->selectedItems();
    QList<SharePtr> unmountedShares, mountedShares;

    for (QTreeWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KNetworkBrowserItem *item = static_cast<Smb4KNetworkBrowserItem *>(selectedItem);

        if (item && item->type() == Share && !item->shareItem()->isPrinter()) {
            if (item->shareItem()->isMounted()) {
                mountedShares << item->shareItem();
            } else {
                unmountedShares << item->shareItem();
            }
        }
    }

    if (!mountedShares.isEmpty()) {
        Smb4KMounter::self()->unmountShares(mountedShares);
    } else {
        Smb4KMounter::self()->mountShares(unmountedShares);
    }
}

void Smb4KNetworkBrowserDockWidget::slotMountActionChanged(bool active)
{
    //
    // Get the mount action
    //
    KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")));

    //
    // Change the shortcuts depending on the value of the 'active' argument
    //
    if (mountAction) {
        if (active) {
            m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(i18n("Ctrl+M")));
        } else {
            m_actionCollection->setDefaultShortcut(mountAction, QKeySequence(i18n("Ctrl+U")));
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
    KDualAction *mountAction = static_cast<KDualAction *>(m_actionCollection->action(QStringLiteral("mount_action")));

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
