/***************************************************************************
    smb4kbookmarkmenu  -  Bookmark menu
                             -------------------
    begin                : Sat Apr 02 2011
    copyright            : (C) 2011-2018 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KBOOKMARKMENU_H
#define SMB4KBOOKMARKMENU_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QPointer>

// KDE includes
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>

// forward declarations
class Smb4KBookmark;


class Smb4KBookmarkMenu : public KActionMenu
{
  Q_OBJECT

  public:
    /**
     * Enumeration
     */
    enum Type { MainWindow,
                SystemTray };
    
    /**
     * Constructor
     */
    explicit Smb4KBookmarkMenu(int type,
                                QWidget *parentWidget = 0,
                                QObject *parent = 0);

    /**
     * Destructor
     */
    ~Smb4KBookmarkMenu();

    /**
     * Force the menu to be set up again. This should be called if 
     * the settings changed and the handling of bookmarks might be
     * affected.
     */
    void refreshMenu();
    
    /**
     * Enable/disable the 'Add Bookmark' action
     */
    void setBookmarkActionEnabled(bool enable);
    
  Q_SIGNALS:
    /**
     * This signal is emitted when the 'Add Bookmark' is triggered.
     */
    void addBookmark();

  protected Q_SLOTS:
    /**
     * Called when the edit action is triggered
     */
    void slotEditActionTriggered(bool checked);
    
    /**
     * Called when the add action is triggered
     */
    void slotAddActionTriggered(bool checked);
    
    /**
     * Called when the toplevel mount action is triggered
     */
    void slotToplevelMountActionTriggered(bool checked);
    
    /**
     * Called when a group action is triggered
     */
    void slotGroupActionTriggered(QAction *action);
    
    /**
     * Called when a bookmark action is triggered
     */
    void slotBookmarkActionTriggered(QAction *action);

    /**
     * Called when the list bookmarks has been updated
     */
    void slotBookmarksUpdated();
    
    /**
     * Called when a bookmark was unmounted
     */
    void slotEnableBookmark(const SharePtr &share);

  private:
    /**
     * Set up the menu
     */
    void setupMenu();
    
    /**
     * Type
     */
    int m_type;

    /**
     * Widget that should be used as parent
     */
    QWidget *m_parent_widget;
    
    /**
     * The actions
     */
    QActionGroup *m_actions;

    /**
     * The bookmarks
     */
    QActionGroup *m_bookmarks;
};

#endif
