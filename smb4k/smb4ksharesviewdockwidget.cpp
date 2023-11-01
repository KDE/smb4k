/*
    The network search widget dock widget

    SPDX-FileCopyrightText: 2018-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// application specific includes
#include "smb4ksharesviewdockwidget.h"
#include "core/smb4khardwareinterface.h"
#include "core/smb4kmounter.h"
#include "core/smb4ksettings.h"
#include "core/smb4kshare.h"
#include "core/smb4ksynchronizer.h"
#include "smb4kbookmarkdialog.h"
#include "smb4kcustomsettingseditor.h"
#include "smb4kpassworddialog.h"
#include "smb4ksharesview.h"
#include "smb4ksharesviewitem.h"
#include "smb4ksynchronizationdialog.h"
#include "smb4ktooltip.h"

#if defined(Q_OS_LINUX)
#include "smb4kmountsettings_linux.h"
#elif defined(Q_OS_FREEBSD) || defined(Q_OS_NETBSD)
#include "smb4kmountsettings_bsd.h"
#endif

// Qt includes
#include <QActionGroup>
#include <QApplication>
#include <QDropEvent>
#include <QMenu>
#include <QPointer>

// KDE includes
#include <KIO/DropJob>
#include <KIconLoader>
#include <KJobUiDelegate>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

Smb4KSharesViewDockWidget::Smb4KSharesViewDockWidget(const QString &title, QWidget *parent)
    : QDockWidget(title, parent)
{
    m_sharesView = new Smb4KSharesView(this);
    setWidget(m_sharesView);

    m_actionCollection = new KActionCollection(this);
    m_contextMenu = new KActionMenu(this);

    setupActions();
    loadSettings();

    connect(m_sharesView, &Smb4KSharesView::customContextMenuRequested, this, &Smb4KSharesViewDockWidget::slotContextMenuRequested);
    connect(m_sharesView, &Smb4KSharesView::itemActivated, this, &Smb4KSharesViewDockWidget::slotItemActivated);
    connect(m_sharesView, &Smb4KSharesView::itemSelectionChanged, this, &Smb4KSharesViewDockWidget::slotItemSelectionChanged);
    connect(m_sharesView, &Smb4KSharesView::acceptedDropEvent, this, &Smb4KSharesViewDockWidget::slotDropEvent);

    connect(Smb4KMounter::self(), &Smb4KMounter::mounted, this, &Smb4KSharesViewDockWidget::slotShareMounted);
    connect(Smb4KMounter::self(), &Smb4KMounter::unmounted, this, &Smb4KSharesViewDockWidget::slotShareUnmounted);
    connect(Smb4KMounter::self(), &Smb4KMounter::updated, this, &Smb4KSharesViewDockWidget::slotShareUpdated);
}

Smb4KSharesViewDockWidget::~Smb4KSharesViewDockWidget()
{
}

void Smb4KSharesViewDockWidget::loadSettings()
{
    //
    // Adjust the view according to the setting chosen
    //
    switch (Smb4KSettings::sharesViewMode()) {
    case Smb4KSettings::EnumSharesViewMode::IconView: {
        m_sharesView->setViewMode(Smb4KSharesView::IconMode, Smb4KSettings::sharesViewIconSizeIconView());
        break;
    }
    case Smb4KSettings::EnumSharesViewMode::ListView: {
        m_sharesView->setViewMode(Smb4KSharesView::ListMode, Smb4KSettings::sharesViewIconSizeListView());
        break;
    }
    default: {
        break;
    }
    }

    //
    // Adjust the unmount actions if needed
    //
    if (!m_sharesView->selectedItems().isEmpty()) {
        QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

        if (selectedItems.size() == 1) {
            Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItems.first());
            m_actionCollection->action(QStringLiteral("unmount_action"))
                ->setEnabled((!item->shareItem()->isForeign() || Smb4KMountSettings::unmountForeignShares()));
        } else if (selectedItems.size() > 1) {
            int foreign = 0;

            for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
                Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

                if (item && item->shareItem()->isForeign()) {
                    foreign++;
                }
            }

            m_actionCollection->action(QStringLiteral("unmount_action"))
                ->setEnabled(((selectedItems.size() > foreign) || Smb4KMountSettings::unmountForeignShares()));
        }
    }

    actionCollection()
        ->action(QStringLiteral("unmount_all_action"))
        ->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && m_sharesView->count() != 0));
}

void Smb4KSharesViewDockWidget::saveSettings()
{
    //
    // Not used at the moment
    //
}

KActionCollection *Smb4KSharesViewDockWidget::actionCollection()
{
    return m_actionCollection;
}

void Smb4KSharesViewDockWidget::setupActions()
{
    //
    // The 'View Modes' submenu and the respective actions
    //
    KActionMenu *viewModesMenu = new KActionMenu(KDE::icon(QStringLiteral("view-choose")), i18n("View Modes"), this);

    QActionGroup *viewModesGroup = new QActionGroup(this);
    viewModesGroup->setExclusive(true);
    connect(viewModesGroup, &QActionGroup::triggered, this, &Smb4KSharesViewDockWidget::slotViewModeChanged);

    QAction *iconViewAction = new QAction(KDE::icon(QStringLiteral("view-list-icons")), i18n("Icon View"), this);
    iconViewAction->setObjectName(QStringLiteral("icon_view_action"));
    iconViewAction->setCheckable(true);
    viewModesGroup->addAction(iconViewAction);
    viewModesMenu->addAction(iconViewAction);

    QAction *listViewAction = new QAction(KDE::icon(QStringLiteral("view-list-details")), i18n("List View"), this);
    listViewAction->setObjectName(QStringLiteral("list_view_action"));
    listViewAction->setCheckable(true);
    viewModesGroup->addAction(listViewAction);
    viewModesMenu->addAction(listViewAction);

    switch (Smb4KSettings::sharesViewMode()) {
    case Smb4KSettings::EnumSharesViewMode::IconView: {
        iconViewAction->setChecked(true);
        break;
    }
    case Smb4KSettings::EnumSharesViewMode::ListView: {
        listViewAction->setChecked(true);
        break;
    }
    default: {
        break;
    }
    }

    m_actionCollection->addAction(QStringLiteral("shares_view_modes"), viewModesMenu);

    //
    // First separator
    //
    QAction *separator1 = new QAction(this);
    separator1->setSeparator(true);

    m_actionCollection->addAction(QStringLiteral("shares_separator1"), separator1);

    //
    // The Unmount action
    //
    QAction *unmountAction = new QAction(KDE::icon(QStringLiteral("media-eject")), i18n("&Unmount"), this);
    unmountAction->setEnabled(false);
    connect(unmountAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotUnmountActionTriggered);

    m_actionCollection->addAction(QStringLiteral("unmount_action"), unmountAction);
    m_actionCollection->setDefaultShortcut(unmountAction, QKeySequence(i18n("Ctrl+U")));

    //
    // The Unmount All action
    //
    QAction *unmountAllAction = new QAction(KDE::icon(QStringLiteral("system-run")), i18n("U&nmount All"), this);
    unmountAllAction->setEnabled(false);
    connect(unmountAllAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotUnmountAllActionTriggered);

    m_actionCollection->addAction(QStringLiteral("unmount_all_action"), unmountAllAction);
    m_actionCollection->setDefaultShortcut(unmountAllAction, QKeySequence(i18n("Ctrl+N")));

    //
    // Second separator
    //
    QAction *separator2 = new QAction(this);
    separator2->setSeparator(true);

    m_actionCollection->addAction(QStringLiteral("shares_separator2"), separator2);

    //
    // The Add Bookmark action
    //
    QAction *bookmarkAction = new QAction(KDE::icon(QStringLiteral("bookmark-new")), i18n("Add &Bookmark"), this);
    bookmarkAction->setEnabled(false);
    connect(bookmarkAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotBookmarkActionTriggered);

    m_actionCollection->addAction(QStringLiteral("bookmark_action"), bookmarkAction);
    m_actionCollection->setDefaultShortcut(bookmarkAction, QKeySequence(i18n("Ctrl+B")));

    //
    // Add custom options action
    //
    QAction *customAction = new QAction(KDE::icon(QStringLiteral("settings-configure")), i18n("Add &Custom Settings"), this);
    customAction->setEnabled(false);
    connect(customAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotAddCustomSettingsTriggered);

    m_actionCollection->addAction(QStringLiteral("custom_action"), customAction);
    m_actionCollection->setDefaultShortcut(customAction, QKeySequence(i18n("Ctrl+C")));

    //
    // The Synchronize action
    //
    QAction *synchronizeAction = new QAction(KDE::icon(QStringLiteral("folder-sync")), i18n("S&ynchronize"), this);
    synchronizeAction->setEnabled(false);
    connect(synchronizeAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotSynchronizeActionTriggered);

    m_actionCollection->addAction(QStringLiteral("synchronize_action"), synchronizeAction);
    m_actionCollection->setDefaultShortcut(synchronizeAction, QKeySequence(i18n("Ctrl+Y")));

    //
    // Third separator
    //
    QAction *separator3 = new QAction(this);
    separator3->setSeparator(true);

    m_actionCollection->addAction(QStringLiteral("shares_separator3"), separator3);

    //
    // The Open with Konsole action
    //
    QAction *konsoleAction = new QAction(KDE::icon(QStringLiteral("utilities-terminal")), i18n("Open with Konso&le"), this);
    konsoleAction->setEnabled(false);
    connect(konsoleAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotKonsoleActionTriggered);

    m_actionCollection->addAction(QStringLiteral("konsole_action"), konsoleAction);
    m_actionCollection->setDefaultShortcut(konsoleAction, QKeySequence(i18n("Ctrl+L")));

    QAction *filemanagerAction = new QAction(KDE::icon(QStringLiteral("system-file-manager")), i18n("Open with F&ile Manager"), this);
    filemanagerAction->setEnabled(false);
    connect(filemanagerAction, &QAction::triggered, this, &Smb4KSharesViewDockWidget::slotFileManagerActionTriggered);

    m_actionCollection->addAction(QStringLiteral("filemanager_action"), filemanagerAction);
    m_actionCollection->setDefaultShortcut(filemanagerAction, QKeySequence(i18n("Ctrl+I")));

    //
    // Plug the actions into the context menu
    //
    QList<QAction *> actionsList = m_actionCollection->actions();

    for (QAction *action : qAsConst(actionsList)) {
        m_contextMenu->addAction(action);
    }
}

void Smb4KSharesViewDockWidget::slotContextMenuRequested(const QPoint &pos)
{
    m_contextMenu->menu()->popup(m_sharesView->viewport()->mapToGlobal(pos));
}

void Smb4KSharesViewDockWidget::slotItemActivated(QListWidgetItem * /*item*/)
{
    //
    // Do not execute the item when keyboard modifiers were pressed
    // or the mouse button is not the left one.
    //
    if (QApplication::keyboardModifiers() == Qt::NoModifier) {
        slotFileManagerActionTriggered(false);
    }
}

void Smb4KSharesViewDockWidget::slotItemSelectionChanged()
{
    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

    if (selectedItems.size() == 1) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItems.first());
        bool syncRunning = Smb4KSynchronizer::self()->isRunning(QUrl::fromLocalFile(item->shareItem()->path()));

        m_actionCollection->action(QStringLiteral("unmount_action"))
            ->setEnabled((!item->shareItem()->isForeign() || Smb4KMountSettings::unmountForeignShares()));
        m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(true);

        if (!item->shareItem()->isInaccessible()) {
            m_actionCollection->action(QStringLiteral("synchronize_action"))
                ->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("rsync")).isEmpty() && !syncRunning);
            m_actionCollection->action(QStringLiteral("konsole_action"))->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("konsole")).isEmpty());
            m_actionCollection->action(QStringLiteral("filemanager_action"))->setEnabled(true);
        } else {
            m_actionCollection->action(QStringLiteral("synchronize_action"))->setEnabled(false);
            m_actionCollection->action(QStringLiteral("konsole_action"))->setEnabled(false);
            m_actionCollection->action(QStringLiteral("filemanager_action"))->setEnabled(false);
        }
    } else if (selectedItems.size() > 1) {
        int syncsRunning = 0;
        int inaccessible = 0;
        int foreign = 0;

        for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
            Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

            if (item) {
                // Is the share synchronized at the moment?
                if (Smb4KSynchronizer::self()->isRunning(QUrl::fromLocalFile(item->shareItem()->path()))) {
                    syncsRunning += 1;
                }

                // Is the share inaccessible at the moment?
                if (item->shareItem()->isInaccessible()) {
                    inaccessible += 1;
                }

                // Was the share being mounted by another user?
                if (item->shareItem()->isForeign()) {
                    foreign += 1;
                }
            }
        }

        m_actionCollection->action(QStringLiteral("unmount_action"))
            ->setEnabled(((selectedItems.size() > foreign) || Smb4KMountSettings::unmountForeignShares()));
        m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(true);
        m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(true);

        if (selectedItems.size() > inaccessible) {
            m_actionCollection->action(QStringLiteral("synchronize_action"))
                ->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("rsync")).isEmpty() && (selectedItems.size() > syncsRunning));
            m_actionCollection->action(QStringLiteral("konsole_action"))->setEnabled(!QStandardPaths::findExecutable(QStringLiteral("konsole")).isEmpty());
            m_actionCollection->action(QStringLiteral("filemanager_action"))->setEnabled(true);
        } else {
            m_actionCollection->action(QStringLiteral("synchronize_action"))->setEnabled(false);
            m_actionCollection->action(QStringLiteral("konsole_action"))->setEnabled(false);
            m_actionCollection->action(QStringLiteral("filemanager_action"))->setEnabled(false);
        }
    } else {
        m_actionCollection->action(QStringLiteral("unmount_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("bookmark_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("custom_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("synchronize_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("konsole_action"))->setEnabled(false);
        m_actionCollection->action(QStringLiteral("filemanager_action"))->setEnabled(false);
    }
}

void Smb4KSharesViewDockWidget::slotDropEvent(Smb4KSharesViewItem *item, QDropEvent *e)
{
    if (item && e) {
        if (e->mimeData()->hasUrls()) {
            if (Smb4KHardwareInterface::self()->isOnline()) {
                QUrl dest = QUrl::fromLocalFile(item->shareItem()->path());
                KIO::DropJob *job = KIO::drop(e, dest, KIO::DefaultFlags);
                KJobWidgets::setWindow(job, m_sharesView->viewport());
                job->uiDelegate()->setAutoErrorHandlingEnabled(true);
                job->uiDelegate()->setAutoWarningHandlingEnabled(true);
            } else {
                KMessageBox::error(
                    m_sharesView,
                    i18n("<qt>There is no active connection to the share <b>%1</b>! You cannot drop any files here.</qt>", item->shareItem()->displayString()));
            }
        }
    }
}

void Smb4KSharesViewDockWidget::slotViewModeChanged(QAction *action)
{
    //
    // Set the new view mode
    //
    if (action->objectName() == QStringLiteral("icon_view_action")) {
        Smb4KSettings::setSharesViewMode(Smb4KSettings::EnumSharesViewMode::IconView);
    } else if (action->objectName() == QStringLiteral("list_view_action")) {
        Smb4KSettings::setSharesViewMode(Smb4KSettings::EnumSharesViewMode::ListView);
    }

    //
    // Save settings
    //
    Smb4KSettings::self()->save();

    //
    // Load the settings
    //
    loadSettings();
}

void Smb4KSharesViewDockWidget::slotShareMounted(const SharePtr &share)
{
    //
    // Add the share to the shares view
    //
    if (share) {
        // Add the item
        (void)new Smb4KSharesViewItem(m_sharesView, share);

        // Sort the view
        m_sharesView->sortItems(Qt::AscendingOrder);

        // Enable/disable the 'Unmount All' action
        actionCollection()
            ->action(QStringLiteral("unmount_all_action"))
            ->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && m_sharesView->count() != 0));
    }
}

void Smb4KSharesViewDockWidget::slotShareUnmounted(const SharePtr &share)
{
    //
    // Remove the share from the shares view
    //
    if (share) {
        // Get the item and delete it. Take care of the current item, if necessary.
        for (int i = 0; i < m_sharesView->count(); ++i) {
            Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(m_sharesView->item(i));

            if (item && (item->shareItem()->path() == share->path() || item->shareItem()->canonicalPath() == share->canonicalPath())) {
                if (item == m_sharesView->currentItem()) {
                    m_sharesView->setCurrentItem(nullptr);
                }

                delete m_sharesView->takeItem(i);
                break;
            } else {
                continue;
            }
        }

        // Enable/disable the 'Unmount All' action
        actionCollection()
            ->action(QStringLiteral("unmount_all_action"))
            ->setEnabled(((!onlyForeignMountedShares() || Smb4KMountSettings::unmountForeignShares()) && m_sharesView->count() != 0));
    }
}

void Smb4KSharesViewDockWidget::slotShareUpdated(const SharePtr &share)
{
    if (share) {
        m_sharesView->toolTip()->update();

        for (int i = 0; i < m_sharesView->count(); ++i) {
            Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(m_sharesView->item(i));

            if (item && (item->shareItem()->path() == share->path() || item->shareItem()->canonicalPath() == share->canonicalPath())) {
                item->update();
                break;
            } else {
                continue;
            }
        }
    }
}

void Smb4KSharesViewDockWidget::slotUnmountActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();
    QList<SharePtr> shares;

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

        if (item) {
            shares << item->shareItem();
        }
    }

    Smb4KMounter::self()->unmountShares(shares, false);
}

void Smb4KSharesViewDockWidget::slotUnmountAllActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    Smb4KMounter::self()->unmountAllShares(false);
}

void Smb4KSharesViewDockWidget::slotBookmarkActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();
    QList<SharePtr> shares;

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);
        shares << item->shareItem();
    }

    QPointer<Smb4KBookmarkDialog> bookmarkDialog = new Smb4KBookmarkDialog();

    if (bookmarkDialog->setShares(shares)) {
        bookmarkDialog->open();
    } else {
        delete bookmarkDialog;
    }
}

void Smb4KSharesViewDockWidget::slotAddCustomSettingsTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

        QPointer<Smb4KCustomSettingsEditor> customSettingsEditor = new Smb4KCustomSettingsEditor();
        if (customSettingsEditor->setNetworkItem(item->shareItem())) {
            customSettingsEditor->open();
        } else {
            delete customSettingsEditor;
        }
    }
}

void Smb4KSharesViewDockWidget::slotSynchronizeActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

        if (item && !item->shareItem()->isInaccessible() && !Smb4KSynchronizer::self()->isRunning(QUrl::fromLocalFile(item->shareItem()->path()))) {
            QPointer<Smb4KSynchronizationDialog> synchronizationDialog = new Smb4KSynchronizationDialog();
            if (synchronizationDialog->setShare(item->shareItem())) {
                synchronizationDialog->open();
            } else {
                delete synchronizationDialog;
            }
        }
    }
}

void Smb4KSharesViewDockWidget::slotKonsoleActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

        if (item && !item->shareItem()->isInaccessible()) {
            openShare(item->shareItem(), Konsole);
        }
    }
}

void Smb4KSharesViewDockWidget::slotFileManagerActionTriggered(bool checked)
{
    Q_UNUSED(checked);

    QList<QListWidgetItem *> selectedItems = m_sharesView->selectedItems();

    for (QListWidgetItem *selectedItem : qAsConst(selectedItems)) {
        Smb4KSharesViewItem *item = static_cast<Smb4KSharesViewItem *>(selectedItem);

        if (item && !item->shareItem()->isInaccessible()) {
            openShare(item->shareItem(), FileManager);
        }
    }
}
