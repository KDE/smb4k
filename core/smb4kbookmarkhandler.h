/***************************************************************************
    This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2021 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKHANDLER_H
#define SMB4KBOOKMARKHANDLER_H

// application specific includes
#include "smb4kglobal.h"

// Qt includes
#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QUrl>

// forward declarations
class Smb4KBookmark;
class Smb4KBookmarkHandlerPrivate;
class Smb4KBookmarkDialog;
class Smb4KBookmarkEditor;
class Smb4KProfileManager;

/**
 * This class belongs the to core classes of Smb4K and manages the
 * bookmarks.
 *
 * @author         Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KBookmarkHandler : public QObject
{
    Q_OBJECT

    friend class Smb4KBookmarkHandlerPrivate;
    friend class Smb4KProfileManager;

public:
    /**
     * The constructor.
     */
    explicit Smb4KBookmarkHandler(QObject *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KBookmarkHandler();

    /**
     * This function returns a static pointer to this class.
     *
     * @returns a static pointer to the Smb4KSynchronizer class.
     */
    static Smb4KBookmarkHandler *self();

    /**
     * This function adds a new bookmark.
     *
     * @param share         The share that is to be bookmarked.
     */
    void addBookmark(const SharePtr &share);

    /**
     * This function adds a new bookmark. The bookmark will be copied
     * internally, so it is save to clear the bookmark pointer after
     * it was passed to this function.
     *
     * @param bookmark      The bookmark that is to be added.
     */
    void addBookmark(const BookmarkPtr &bookmark);

    /**
     * This function adds several bookmarks at once. It takes a list of
     * shares and converts them internally to bookmark items.
     *
     * @param list          The list of shares that are to be bookmarked
     */
    void addBookmarks(const QList<SharePtr> &list);

    /**
     * This function adds several bookmarks at once. It takes a list of
     * Smb4KBookmark items.
     *
     * @param list          The list of bookmarks that are to be bookmarked
     *
     * @param replace       If TRUE the old list of bookmarks is replaced by
     *                      @p list.
     */
    void addBookmarks(const QList<BookmarkPtr> &list, bool replace = false);

    /**
     * Remove a bookmark.
     *
     * @param bookmark      The bookmark that is to be removed
     */
    void removeBookmark(const BookmarkPtr &bookmark);

    /**
     * This function removes a category and all the bookmarks it contains.
     *
     * @param name          The group name
     */
    void removeCategory(const QString &name);

    /**
     * Get the list of bookmarks.
     *
     * @returns             The current list of bookmarks stored in the
     *                      bookmark file.
     */
    QList<BookmarkPtr> bookmarksList() const;

    /**
     * Get the list of bookmarks belonging to a certain category.
     *
     * @param categoryName  The name of the category the bookmarks are organized in
     *
     * @returns a list of bookmarks belonging to a certain category
     */
    QList<BookmarkPtr> bookmarksList(const QString &categoryName) const;

    /**
     * This function searches for a bookmark using its URL and returns a pointer
     * to it if it is present or NULL.
     *
     * @param url           The URL of the bookmark that is searched.
     *
     * @returns the bookmark object that was searched for or NULL if it was not
     * found.
     */
    BookmarkPtr findBookmarkByUrl(const QUrl &url);

    /**
     * This function searches for a bookmark using its label and returns a pointer
     * to it if it is present or NULL.
     *
     * @param label         The label that is searched.
     *
     * @returns             The bookmark object that was searched for or NULL if it
     *                      wasn't found.
     */
    BookmarkPtr findBookmarkByLabel(const QString &label);

    /**
     * Returns the sorted list of bookmark categories.
     *
     * @returns the list of categories
     */
    QStringList categoryList() const;

    /**
     * Reset the bookmarks by reloading them from the file.
     */
    void resetBookmarks();

    /**
     * This function checks if the @p share is already bookmarked or not.
     * @param share         The share item
     * @returns TRUE if the share is bookmarked and FALSE otherwise.
     */
    bool isBookmarked(const SharePtr &share);

    /**
     * This function opens the bookmark editor.
     */
    void editBookmarks();

Q_SIGNALS:
    /**
     * Signal emitted when the list of bookmarks has been updated.
     */
    void updated();

private:
    /**
     * This function reads the list of bookmarks from the bookmarks file.
     */
    void readBookmarkList();

    /**
     * This function updates the data of the bookmarks.
     */
    void update() const;

    /**
     * This function writes the bookmarks to the disk.
     */
    void writeBookmarkList();

    /**
     * Migrates one profile to another.
     *
     * This function is meant to be used by the profile manager.
     *
     * @param from        The name of the old profile.
     * @param to          The name of the new profile.
     */
    void migrateProfile(const QString &from, const QString &to);

    /**
     * Removes a profile from the list of profiles.
     *
     * This function is meant to be used by the profile manager.
     *
     * @param name        The name of the profile.
     */
    void removeProfile(const QString &name);

    /**
     * Pointer to Smb4KBookmarkHandlerPrivate class
     */
    const QScopedPointer<Smb4KBookmarkHandlerPrivate> d;
};

#endif
