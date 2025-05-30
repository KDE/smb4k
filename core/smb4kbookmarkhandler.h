/*
    This class handles the bookmarks.

    SPDX-FileCopyrightText: 2004-2025 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KBOOKMARKHANDLER_H
#define SMB4KBOOKMARKHANDLER_H

// application specific includes
#include "smb4kcore_export.h"
#include "smb4kglobal.h"

// Qt includes
#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QUrl>

// forward declarations
class Smb4KBookmarkHandlerPrivate;

/**
 * This class belongs the to core classes of Smb4K and manages the
 * bookmarks.
 *
 * @author         Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class SMB4KCORE_EXPORT Smb4KBookmarkHandler : public QObject
{
    Q_OBJECT

    friend class Smb4KBookmarkHandlerPrivate;

public:
    /**
     * The constructor.
     */
    explicit Smb4KBookmarkHandler(QObject *parent = nullptr);

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
     * This function adds a new bookmark. The bookmark will be copied
     * internally, so it is save to clear the bookmark pointer after
     * it was passed to this function.
     *
     * @param bookmark      The bookmark that is to be added.
     */
    void addBookmark(const BookmarkPtr &bookmark);

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
    QList<BookmarkPtr> bookmarkList() const;

    /**
     * Get the list of bookmarks belonging to a certain category.
     *
     * @param categoryName  The name of the category the bookmarks are organized in
     *
     * @returns a list of bookmarks belonging to a certain category
     */
    QList<BookmarkPtr> bookmarkList(const QString &categoryName) const;

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
     * Returns the sorted list of all bookmark categories.
     *
     * @returns the list of categories
     */
    QStringList categoryList() const;

    /**
     * This function checks if the @p share is already bookmarked or not.
     * @param share         The share item
     * @returns TRUE if the share is bookmarked and FALSE otherwise.
     */
    bool isBookmarked(const SharePtr &share);

Q_SIGNALS:
    /**
     * Signal emitted when the list of bookmarks has been updated.
     */
    void updated();

protected Q_SLOTS:
    /**
     * Called when a profile was removed
     *
     * @param name          The name of the profile
     */
    void slotProfileRemoved(const QString &name);

    /**
     * Called when a profile was migrated
     *
     * @param oldName       The old profile name
     * @param newName       The new profile name
     */
    void slotProfileMigrated(const QString &oldName, const QString &newName);

    /**
     * Called when the active profile changed
     *
     * @param name          Profile name
     */
    void slotActiveProfileChanged(const QString &name);

private:
    /**
     * Add a @p bookmark to the list. Return TRUE on success.
     */
    bool add(const BookmarkPtr &bookmark);

    /**
     * Remove the @p bookmark from the list. Return TRUE on success.
     *
     * @param bookmark      The bookmark to be removed
     *
     * @returns TRUE if successful.
     */
    bool remove(const BookmarkPtr &bookmark);

    /**
     * Remove the category with @p name from the list. Return TRUE
     * on success.
     *
     * @parem name          The category to be removed
     *
     * @returns TRUE if successful.
     */
    bool remove(const QString &name);

    /**
     * Read the bookmarks from the file
     */
    void read();

    /**
     * Write the bookmarks to the file
     */
    void write();

    /**
     * Pointer to Smb4KBookmarkHandlerPrivate class
     */
    const QScopedPointer<Smb4KBookmarkHandlerPrivate> d;
};

#endif
