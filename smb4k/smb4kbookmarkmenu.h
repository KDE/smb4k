/*
    smb4kbookmarkmenu  -  Bookmark menu

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    enum Type { MainWindow, SystemTray };

    /**
     * Constructor
     */
    explicit Smb4KBookmarkMenu(int type, QObject *parent = nullptr);

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
     * Called when a mount action was triggered
     */
    void slotMountActionTriggered(QAction *action);

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
     * Add a bookmark to the menu
     */
    void addBookmarkToMenu(const BookmarkPtr &bookmark);

    /**
     * Enables or disables the mount actions according to the status
     * of the bookmarks in their respective category.
     */
    void adjustMountActions();

    /**
     * Type
     */
    int m_type;

    /**
     * The actions
     */
    QActionGroup *m_mountActions;

    /**
     * The categories
     */
    QActionGroup *m_categories;

    /**
     * The bookmarks
     */
    QActionGroup *m_bookmarks;

    /**
     * The 'Edit Bookmarks' action
     */
    QAction *m_editBookmarks;

    /**
     * The "Add Bookmark" action
     */
    QAction *m_addBookmark;

    /**
     * The toplevel "Mount All Bookmarks" action
     */
    QAction *m_toplevelMount;

    /**
     * The separator between the actions and the bookmarks
     */
    QAction *m_separator;
};

#endif
