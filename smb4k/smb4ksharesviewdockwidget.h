/*
    The network search widget dock widget

    SPDX-FileCopyrightText: 2018-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSHARESVIEWDOCKWIDGET_H
#define SMB4KSHARESVIEWDOCKWIDGET_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4ksharesview.h"

// Qt includes
#include <QDockWidget>

// KDE includes
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>

// forward declarations
class Smb4KSharesViewItem;

using namespace Smb4KGlobal;

class Smb4KSharesViewDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    Smb4KSharesViewDockWidget(const QString &title, QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KSharesViewDockWidget();

    /**
     * Load settings
     */
    void loadSettings();

    /**
     * Save settings
     */
    void saveSettings();

    /**
     * Returns the action collection of this dock widget
     * @returns the action collection
     */
    KActionCollection *actionCollection();

protected Q_SLOTS:
    /**
     * This slot is called if the user requests the context menu. It shows
     * the menu with the actions defined for the widget.
     * @param pos                 The position where user clicked.
     */
    void slotContextMenuRequested(const QPoint &pos);

    /**
     * This slot is invoked when the user activated an item. It is used to mount
     * shares.
     * @param item                The item that was executed.
     */
    void slotItemActivated(QListWidgetItem *item);

    /**
     * This slot is called when the selection changed. It takes care of the
     * actions being enabled or disabled accordingly. All widget specific
     * stuff has to be done in the shares view itself.
     */
    void slotItemSelectionChanged();

    /**
     * This slot is used to process an accepted drop event.
     * @param item                The item where the drop event occurred.
     * @param e                   The drop event that encapsulates the necessary data.
     */
    void slotDropEvent(Smb4KSharesViewItem *item, QDropEvent *e);

    /**
     * This slot is invoked when the view mode was changed in the View Modes
     * context menu.
     * @param action              The action that was checked
     */
    void slotViewModeChanged(QAction *action);

    /**
     * This slot is connected to the Smb4KMounter::mounted() signal and adds the
     * mounted share @p share to the shares view.
     * @param share               The share item
     */
    void slotShareMounted(const SharePtr &share);

    /**
     * This slot is connected to the Smb4KMounter::unmounted() signal and removes
     * the share @p share from the shares view.
     * @param share               The share item
     */
    void slotShareUnmounted(const SharePtr &share);

    /**
     * This slot is connected to the Smb4KMounter::updated() signal and updates
     * the item in the shares view corresponding to @p share.
     *
     * This slot does not remove or add any share, it only updates the present
     * items.
     * @param share               The Smb4KShare item
     */
    void slotShareUpdated(const SharePtr &share);

    /**
     * This slot is connected to the 'Unmount action'.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountActionTriggered(bool checked);

    /**
     * This slot is connected to the 'Unmount All' action. All shares - either of
     * the user or that are present on the system (depending on the settings the
     * user chose) - will be unmounted.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountAllActionTriggered(bool checked);

    /**
     * This slot is connected to the 'Add Bookmark' action. It lets you add a share
     * to the bookmarks.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotBookmarkActionTriggered(bool checked);

    /**
     * This slot is connected to the 'Synchronize' action. The selected items will be
     * synchronized with a local copy (or vice versa) if you activate it.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotSynchronizeActionTriggered(bool checked);

    /**
     * This slot is connected to the 'Open with Konsole' action. The mount point of
     * the selected share items will be opened in Konsole.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotKonsoleActionTriggered(bool checked);

    /**
     * This slot is connected to the 'Open with Filemanager' action. The contents of
     * the selected share items will be opened in the file manager.
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotFileManagerActionTriggered(bool checked);

private:
    /**
     * Set up the actions
     */
    void setupActions();

    /**
     * The shares shares view
     */
    Smb4KSharesView *m_sharesView;

    /**
     * Action collection
     */
    KActionCollection *m_actionCollection;

    /**
     * Context menu
     */
    KActionMenu *m_contextMenu;
};

#endif
