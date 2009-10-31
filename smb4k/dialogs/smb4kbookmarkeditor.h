/***************************************************************************
    smb4kbookmarkeditor  -  The bookmark editor of Smb4K
                             -------------------
    begin                : Di Okt 5 2004
    copyright            : (C) 2004-2007 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKEDITOR_H
#define SMB4KBOOKMARKEDITOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidget>

// KDE includes
#include <kdialog.h>
#include <kactioncollection.h>
#include <kdemacros.h>

/**
 * This is the bookmark editor of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class KDE_EXPORT Smb4KBookmarkEditor : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent  The parent of this dialog.
     */
    Smb4KBookmarkEditor( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KBookmarkEditor();

  protected slots:
    /**
     * This slot is activated whenever the context menu is requested.
     *
     * @param pos     The position where the mouse is.
     */
    void slotContextMenuRequested( const QPoint &pos );

    /**
     * This slot is activated whenever the "Edit" action is triggered.
     * It edits the selected bookmark.
     *
     * @param checked Is TRUE if the action is checkable and checked
     *                and otherwise FALSE.
     */
    void slotEditActionTriggered( bool checked );

    /**
     * This slot is activated whenever the "Delete" action is triggered.
     * It removes all selected items from the bookmark editor.
     *
     * @param checked Is TRUE if the action is checkable and checked
     *                and otherwise FALSE.
     */
    void slotDeleteActionTriggered( bool checked );

    /**
     * This slot initiates the editing of an item, if the user double
     * clicked the right column.
     *
     * @param item    The item the user double clicked.
     *
     * @param column  The column the user double clicked.
     */
    void slotItemDoubleClicked( QTreeWidgetItem *item,
                                int column );

    /**
     * This slot is activated if the OK button has been clicked.
     */
    void slotOkClicked();

    /**
     * This slot is activated if the Cancel button has been clicked.
     */
    void slotCancelClicked();

    /**
     * This slot is invoked, if the bookmark handler updated the bookmarks.
     */
    void slotLoadBookmarks();

  private:
    /**
     * The tree view.
     */
    QTreeWidget *m_widget;

    /**
     * The action collection of this class.
     */
    KActionCollection *m_collection;

    /**
     * Enumeration for the columns
     */
    enum Columns { Bookmark = 0,
                   Workgroup = 1,
                   Login = 2,
                   IPAddress = 3,
                   Label = 4 };
};

#endif
