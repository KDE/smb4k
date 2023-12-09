/*
    smb4ksharesmenu  -  Shares menu

    SPDX-FileCopyrightText: 2011-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSHARESMENU_H
#define SMB4KSHARESMENU_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QAction>
#include <QActionGroup>

// KDE includes
#include <KActionMenu>

// forward declarations
class Smb4KShare;

class Smb4KSharesMenu : public KActionMenu
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KSharesMenu(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~Smb4KSharesMenu();

    /**
     * Refresh the shares menu
     */
    void refreshMenu();

protected Q_SLOTS:
    /**
     * This slot is invokes when a share was mounted
     */
    void slotShareMounted(const SharePtr &share);

    /**
     * This slot is invoked when a share was unmounted
     */
    void slotShareUnmounted(const SharePtr &share);

    /**
     * This slot is invoked when the modifictions of the
     * mounted shares list are finished.
     */
    void slotMountedSharesListChanged();

    /**
     * This slot unmounts all shares at once.
     */
    void slotUnmountAllShares();

    /**
     * This slot is called when an action is triggered.
     *
     * @param action        The action that was triggered
     */
    void slotShareAction(QAction *action);

private:
    /**
     * Add a share to the menu
     */
    void addShareToMenu(const SharePtr &share);

    /**
     * Remove a share from the menu
     */
    void removeShareFromMenu(const SharePtr &share);

    /**
     * Share menus
     */
    QActionGroup *m_menus;

    /**
     * Share actions
     */
    QActionGroup *m_actions;

    /**
     * The Unmount All action
     */
    QAction *m_unmountAll;

    /**
     * The separator action
     */
    QAction *m_separator;
};

#endif
