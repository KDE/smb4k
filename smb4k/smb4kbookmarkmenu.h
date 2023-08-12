/*
    smb4kbookmarkmenu  -  Bookmark menu

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARKMENU_H
#define SMB4KBOOKMARKMENU_H

// application specific includes
#include "smb4kbookmarkeditor.h"
#include "smb4kglobal.h"

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QPointer>

// KDE includes
#include <KActionMenu>

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
     * Refresh the menu
     */
    void refreshMenu();

    /**
     * Enable/disable the 'Add Bookmark' action
     */
    void setBookmarkActionEnabled(bool enable);

public Q_SLOTS:
    /**
     * Load the bookmarks into the menu
     */
    void loadBookmarks();

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
     * Called when a bookmark was unmounted
     */
    void slotEnableBookmark(const SharePtr &share);

private:
    /**
     * Enables or disables the mount actions according to the status
     * of the bookmarks in their respective category.
     */
    void adjustMountActions();

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

    /**
     * The bookmark editor
     */
    QPointer<Smb4KBookmarkEditor> m_bookmarkEditor;
};

#endif
