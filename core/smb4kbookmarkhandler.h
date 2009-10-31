/***************************************************************************
    smb4kbookmarkhandler  -  This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2008 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KBOOKMARKHANDLER_H
#define SMB4KBOOKMARKHANDLER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QObject>
#include <QList>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KBookmark;
class Smb4KShare;


/**
 * This class belongs the to core classes of Smb4K and manages the
 * bookmarks.
 *
 * @author         Alexander Reinholdt <dustpuppy@mail.berlios.de>
 */

class KDE_EXPORT Smb4KBookmarkHandler : public QObject
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent        The parent of this object
     */
    Smb4KBookmarkHandler( QObject *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KBookmarkHandler();

    /**
     * This function adds a new bookmark.
     *
     * @param share         The share that is to be bookmarked.
     *
     * @param overwrite     Set this to FALSE if you do not want to overwrite
     *                      (i.e. update) an existing bookmark with the data of
     *                      this share. By default, @p overwrite is set to TRUE.
     */
    void addBookmark( Smb4KShare *share,
                      bool overwrite = true );

    /**
     * This function adds a new bookmark. In contrast to the above function, it
     * takes a Smb4KBookmark object as argument.
     *
     * @param list          The bookmark that is to be stored.
     *
     * @param overwrite     Set this to FALSE if you do not want to overwrite
     *                      (i.e. update) an existing bookmark with the data of
     *                      this share. By default, @p overwrite is set to TRUE.
     */
    void addBookmark( Smb4KBookmark *bookmark,
                      bool overwrite = true );

    /**
     * This function writes a new list of bookmarks. The old list will be
     * deleted. It should be used, if you manipulated the list of bookmarks
     * i. e. by a bookmark editor. When this function finishes, the
     * bookmarksUpdated() signal will be emitted.
     *
     * @param list          The (new) list of bookmarks that is to be written
     *                      to the bookmark file
     */
    void writeBookmarkList( const QList<Smb4KBookmark *> &list );

    /**
     * Get the list of bookmarks.
     *
     * @returns             The current list of bookmarks stored in the
     *                      bookmark file.
     */
    const QList<Smb4KBookmark *> &getBookmarks();

    /**
     * This function searches for a bookmark using its UNC and returns a pointer
     * to it if it is present or NULL.
     *
     * @param UNC           The UNC of the bookmark that is searched.
     *
     * @returns the bookmark object that was searched for or NULL if it was not
     * found.
     */
    Smb4KBookmark *findBookmarkByUNC( const QString &unc );

    /**
     * This function searches for a bookmark using its label and returns a pointer
     * to it if it is present or NULL.
     *
     * @param label         The label that is searched.
     *
     * @returns             The bookmark object that was searched for or NULL if it
     *                      wasn't found.
     */
    Smb4KBookmark *findBookmarkByLabel( const QString &label );

  signals:
    /**
     * Signal emitted when the list of bookmarks has been updated.
     */
    void updated();

  private:
    /**
     * The list of bookmarks.
     */
    QList<Smb4KBookmark *> m_bookmarks;

    /**
     * This function loads the list of bookmarks from the bookmarks file.
     * When it finishes, the bookmarksUpdated() signal is emitted. So, if you
     * want to access the list of bookmarks immediately after they were read,
     * connect a slot to that signal.
     */
    void loadBookmarks();

    /**
     * This function updates the data of the bookmarks, i.e. is searches for
     * the host provided by m_hosts and sets the appropriate data, if
     * necessary.
     */
    void update();
};
#endif
