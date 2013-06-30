/***************************************************************************
    smb4kbookmarkhandler  -  This class handles the bookmarks.
                             -------------------
    begin                : Fr Jan 9 2004
    copyright            : (C) 2004-2012 by Alexander Reinholdt
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

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QScopedPointer>
#include <QtCore/QUrl>
#include <QtGui/QWidget>
#include <QtDeclarative/QDeclarativeListProperty>

// KDE includes
#include <kdemacros.h>

// forward declarations
class Smb4KBookmark;
class Smb4KBookmarkObject;
class Smb4KShare;
class Smb4KBookmarkHandlerPrivate;
class Smb4KBookmarkDialog;
class Smb4KBookmarkEditor;


/**
 * This class belongs the to core classes of Smb4K and manages the
 * bookmarks.
 *
 * @author         Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class KDE_EXPORT Smb4KBookmarkHandler : public QObject
{
  Q_OBJECT
  
  Q_PROPERTY( QDeclarativeListProperty<Smb4KBookmarkObject> bookmarks READ bookmarks NOTIFY updated )
  Q_PROPERTY( QDeclarativeListProperty<Smb4KBookmarkObject> groups READ groups )
  
  
  friend class Smb4KBookmarkHandlerPrivate;

  public:
    /**
     * The constructor.
     */
    explicit Smb4KBookmarkHandler( QObject *parent = 0 );

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
     * 
     * @param parent        The parent widget
     */
    void addBookmark( Smb4KShare *share,
                      QWidget *parent = 0 );
    
    /**
     * This function adds a new bookmark. In contrast to the functions above,
     * it takes an URL @p url and searches the global list of shares for the 
     * share with this URL. If the share cannot be found, nothing will be done.
     * 
     * @param url           The URL of the share that is to be bookmarked
     */
    Q_INVOKABLE void addBookmark( const QUrl &url );

    /**
     * This function adds several bookmarks at once. It takes a list of 
     * Smb4KShare items and converts them internally to bookmark items.
     *
     * @param list          The list of shares that are to be bookmarked
     *
     * @param parent        The parent widget
     */
    void addBookmarks( const QList<Smb4KShare *> &list,
                       QWidget *parent = 0 );
    
    /**
     * Remove a bookmark.
     * 
     * @param bookmark      The bookmark that is to be removed
     */
    void removeBookmark( Smb4KBookmark *bookmark );
    
    /**
     * This function removes a bookmark with URL @p url. 
     * 
     * @param url           The URL of the bookmark that is to be removed
     */
    Q_INVOKABLE void removeBookmark( const QUrl &url );
    
    /**
     * This function removes a group and all the bookmarks it contains. Please
     * note that the 
     * 
     * @param name          The group name
     */
    Q_INVOKABLE void removeGroup( const QString &name );

    /**
     * Get the list of bookmarks.
     *
     * @returns             The current list of bookmarks stored in the
     *                      bookmark file.
     */
    QList<Smb4KBookmark *> bookmarksList() const;
    
    /**
     * This function returns the list of bookmarks. Basically, this is the 
     * the list returned by the above bookmarksList() function converted into a 
     * list of Smb4KBookmarkObject objects.
     * 
     * @returns the list of bookmarks
     */
    QDeclarativeListProperty<Smb4KBookmarkObject> bookmarks();

    /**
     * Get the list of bookmarks belonging to a certain group.
     *
     * @param group         The name of the group the bookmarks are organized in
     *
     * @returns a list of bookmarks belonging to a certain group
     */
    QList<Smb4KBookmark *> bookmarksList( const QString &group ) const;
    
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

    /**
     * Returns the sorted list of bookmark groups.
     *
     * @returns the list of groups
     */
    QStringList groupsList() const;
    
    /**
     * This function returns the list of bookmark groups. Basically, this is the 
     * the list returned by the above groupsList() function converted into a 
     * list of Smb4KBookmarkObject objects.
     * 
     * @returns the list of bookmarks
     */
    QDeclarativeListProperty<Smb4KBookmarkObject> groups();    

    /**
     * Opens the bookmark editor
     * 
     * @param parent        The parent widget
     */
    Q_INVOKABLE void editBookmarks( QWidget *parent = 0 );

  Q_SIGNALS:
    /**
     * Signal emitted when the list of bookmarks has been updated.
     */
    void updated();

  private:
    /**
     * This function loads the list of bookmarks from the bookmarks file.
     * When it finishes, the updated() signal is emitted. So, if you
     * want to access the list of bookmarks immediately after they were read,
     * connect a slot to that signal.
     */
    void loadBookmarks();

    /**
     * This function updates the data of the bookmarks, i.e. is searches for
     * the host provided by m_hosts and sets the appropriate data, if
     * necessary.
     */
    void update() const;

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
     * This function adds several bookmarks at once. It takes a list of
     * Smb4KBookmark items.
     * 
     * @param list          The list of bookmarks that are to be bookmarked
     * 
     * @param replace       If TRUE the old list of bookmarks is replaced by
     *                      @p list.
     */
    void addBookmarks( const QList<Smb4KBookmark *> &list,
                       bool replace = false );
    
    /**
     * Pointer to Smb4KBookmarkHandlerPrivate class
     */
    const QScopedPointer<Smb4KBookmarkHandlerPrivate> d;
};

#endif
