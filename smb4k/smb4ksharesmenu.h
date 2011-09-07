/***************************************************************************
    smb4ksharesmenu  -  Shares menu
                             -------------------
    begin                : Mon Sep 05 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSHARESMENU_H
#define SMB4KSHARESMENU_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QAction>
#include <QActionGroup>

// KDE includes
#include <kactionmenu.h>
#include <kactioncollection.h>

// forward declarations
class Smb4KShare;

class Smb4KSharesMenu : public KActionMenu
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    Smb4KSharesMenu( QWidget *parentWidget = 0,
                     QObject *parent = 0 );

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
     * Connected to Smb4KMounter::mounted() signal. Adds a
     * share to the menu.
     *
     * @param share         The share that has been mounted
     */
    void slotShareMounted( Smb4KShare *share );

    /**
     * Connected to Smb4KMounter::unmounted() signal. Removes
     * a share from the menu.
     *
     * @param share         The share that has been unmounted
     */
    void slotShareUnmounted( Smb4KShare *share );

    /**
     * This slot unmounts all shares at once.
     */
    void slotUnmountAllShares();

    /**
     * This slot is called when an action is triggered.
     * 
     * @param action        The action that was triggered
     */
    void slotShareAction( QAction *action );

  private:
    /**
     * Setup the menu
     */
    void setupMenu();
    
    /**
     * Parent widget
     */
    QWidget *m_parent_widget;

    /**
     * The action collection
     */
    KActionCollection *m_action_collection;

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