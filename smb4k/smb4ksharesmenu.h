/*
    smb4ksharesmenu  -  Shares menu

    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>

// forward declarations
class Smb4KShare;

class Smb4KSharesMenu : public KActionMenu
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    explicit Smb4KSharesMenu(QObject *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KSharesMenu();

    /**
     * Refresh the shares menu
     */
    void refreshMenu();

protected slots:
    /**
     * This slot is connected to the Smb4KMounter::mountedSharesListChanged()
     * signal. It refreshes the menu.
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
     * Setup the menu
     */
    void setupMenu();

    /**
     * Share menus
     */
    QActionGroup *m_menus;

    /**
     * Share actions
     */
    QActionGroup *m_actions;
};

#endif
