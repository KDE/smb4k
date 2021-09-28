/*
    smb4ksharesmenu  -  Shares menu
    -------------------
    begin                : Mon Sep 05 2011
    SPDX-FileCopyrightText: 2011-2021 Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
*/

/***************************************************************************
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
