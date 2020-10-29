/***************************************************************************
    Private classes for the bookmark handler
                             -------------------
    begin                : Sun Mar 20 2011
    copyright            : (C) 2011-2020 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKHANDLER_P_H
#define SMB4KBOOKMARKHANDLER_P_H

// application specific includes
#include "smb4kbookmarkhandler.h"
#include "smb4kglobal.h"

// Qt includes
#include <QString>
#include <QUrl>
#include <QTreeWidget>
#include <QDialog>
#include <QListWidget>
#include <QAction>
#include <QPushButton>
#include <QPointer>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>
#include <KWidgetsAddons/KActionMenu>


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


class Smb4KBookmarkEditor : public QDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param bookmarks   The list of all bookmarks
     *
     * @param parent      The parent of this dialog.
     */
    explicit Smb4KBookmarkEditor(const QList<BookmarkPtr> &bookmarks, QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KBookmarkEditor();
    
    /**
     * Load the bookmarks into the view
     */
    void loadBookmarks();
    
    /**
     * Return the list of edited bookmarks
     */
    QList<BookmarkPtr> editedBookmarks();

  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

  protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked
     */
    void slotItemClicked(QTreeWidgetItem *item, int col);

    /**
     * Called when the context menu was requested
     */
    void slotContextMenuRequested(const QPoint &pos);
    
    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the category is edited by the user
     */
    void slotCategoryEdited();

    /**
     * Called when the IP address is edited by the user
     */
    void slotIpEdited();
    
    /**
     * Called when the workgroup name is edited by the user
     */
    void slotWorkgroupNameEdited();

    /**
     * Called when the login is edited by the user
     */
    void slotLoginEdited();

    /**
     * Called when the add action was triggered
     */
    void slotAddCategoryTriggered(bool checked);

    /**
     * Called when the delete action was triggered
     */
    void slotDeleteTriggered(bool checked);

    /**
     * Called when the clear action was triggered
     */
    void slotClearTriggered(bool checked);
    
    /**
     * Called when the Ok button was clicked
     */
    void slotDialogAccepted();
    
    /**
     * Called when the Cancel button was clicked
     */
    void slotDialogRejected();
    
    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);
    
    /**
     * Do adjustments in the list view
     */
    void slotAdjust();

  private:
    /**
     * Set up the view
     */
    void setupView();

    /**
     * Finds the bookmark in the list
     */
    BookmarkPtr findBookmark(const QUrl &url);

    /**
     * List of the bookmarks that are being processed
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
    QPointer<Smb4KBookmarkEditor> editor;
};


class Smb4KBookmarkHandlerStatic
{
  public:
    Smb4KBookmarkHandler instance;
};

#endif
