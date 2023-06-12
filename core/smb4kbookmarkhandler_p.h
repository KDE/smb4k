/*
    Private classes for the bookmark handler

    SPDX-FileCopyrightText: 2011-2022 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARKHANDLER_P_H
#define SMB4KBOOKMARKHANDLER_P_H

// application specific includes
#include "smb4kbookmarkhandler.h"
#include "smb4kglobal.h"

// Qt includes
#include <QDialog>
#include <QListWidget>

class Q_DECL_EXPORT Smb4KBookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * The constructor
     *
     * @param bookmarks       The list of bookmarks that are to be saved
     *
     * @param categories      The list of available bookmark categories
     *
     * @param parent          The parent widget
     */
    Smb4KBookmarkDialog(const QList<BookmarkPtr> &bookmarks, const QStringList &categories, QWidget *parent);

    /**
     * The destructor
     */
    ~Smb4KBookmarkDialog();

    /**
     * Returns the list of bookmarks including all changes that could
     * be made in the bookmark dialog.
     *
     * @returns the list of bookmarks.
     */
    const QList<BookmarkPtr> &bookmarks();

protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked in the list widget.
     */
    void slotBookmarkClicked(QListWidgetItem *bookmarkItem);

    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the category is edited by the user
     */
    void slotCategoryEdited();

    /**
     * Called when the OK button was clicked
     */
    void slotDialogAccepted();

    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);

private:
    /**
     * Sets up the view
     */
    void setupView();

    /**
     * Load the list of bookmarks and the one of the categories
     */
    void loadLists(const QList<BookmarkPtr> &bookmarks, const QStringList &categories);

    /**
     * Finds the bookmark in the list
     */
    BookmarkPtr findBookmark(const QUrl &url);

    /**
     * The list of bookmarks
     */
    QList<BookmarkPtr> m_bookmarks;

    /**
     * The list of groups
     */
    QStringList m_categories;
};

class Smb4KBookmarkHandlerPrivate
{
public:
    QList<BookmarkPtr> bookmarks;
};

class Smb4KBookmarkHandlerStatic
{
public:
    Smb4KBookmarkHandler instance;
};

#endif
