/***************************************************************************
    Bookmark editor for Smb4K
                             -------------------
    begin                : Sun Apr 14 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKEDITOR_H
#define SMB4KBOOKMARKEDITOR_H

// application spcific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QDialog>
#include <QTreeWidget>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>
#include <KWidgetsAddons/KActionMenu>

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
    explicit Smb4KBookmarkEditor(QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KBookmarkEditor();
    
    /**
     * Load the bookmarks into the view
     */
    void loadBookmarks();

  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj, QEvent *e);

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
     * Called when the group is edited by the user
     */
    void slotGroupEdited();

    /**
     * Called when the IP address is edited by the user
     */
    void slotIPEdited();

    /**
     * Called when the login is edited by the user
     */
    void slotLoginEdited();

    /**
     * Called when the add action was triggered
     */
    void slotAddGroupTriggered(bool checked);

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
     * Ok push button
     */
    QPushButton *m_ok_button;
    
    /**
     * Cancel push button
     */
    QPushButton *m_cancel_button;

    /**
     * List of the bookmarks that are being processed
     */
    QList<BookmarkPtr> m_bookmarks;

    /**
     * Tree widget
     */
    QTreeWidget *m_tree_widget;

    /**
     * The widget containing the editors
     */
    QWidget *m_editors;

    /**
     * The label
     */
    KLineEdit *m_label_edit;

    /**
     * The IP address
     */
    KLineEdit *m_ip_edit;

    /**
     * The login
     */
    KLineEdit *m_login_edit;

    /**
     * The groups
     */
    KComboBox *m_group_combo;

    /**
     * The list of groups
     */
    QStringList m_groups;

    /**
     * Action menu
     */
    KActionMenu *m_menu;

    /**
     * Add group action
     */
    QAction *m_add_group;

    /**
     * Delete action
     */
    QAction *m_delete;

    /**
     * Clear action
     */
    QAction *m_clear;
};

#endif
