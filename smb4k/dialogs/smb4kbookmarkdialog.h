/***************************************************************************
    smb4kbookmarkdialog  -  This class provides a dialog to add a bookmark.
                             -------------------
    begin                : Di Jan 13 2009
    copyright            : (C) 2009 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKDIALOG_H
#define SMB4KBOOKMARKDIALOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QList>
#include <QWidget>
#include <QTreeWidget>

// KDE includes
#include <kdialog.h>
#include <kdemacros.h>

// application specific includes
#include <core/smb4kbookmark.h>


/**
 * This class provides a dialog to add a bookmark. You can define its label here
 * as well.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */


class KDE_EXPORT Smb4KBookmarkDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent          The parent widget
     */
    Smb4KBookmarkDialog( QWidget *parent );

   /**
    * The destructor
    */
   ~Smb4KBookmarkDialog();

   /**
    * Set the list of bookmarks. This function will exchange the current list
    * of potential bookmarks if there is one and display the new one.
    *
    * @param list             The new list of potential bookmarks
    */
   void setBookmarks( const QList<Smb4KBookmark *> &list );

  protected slots:
    /**
     * This slot is called when the user clicked a button in the dialog. It
     * is connected to the KDialog::buttonClicked() signal.
     *
     * @param botton_code     The button code
     */
    void slotUserClickedButton( KDialog::ButtonCode button_code );

    /**
     * This slot is called when the user clicked an item. It is used to determine
     * if the clicked item was checked and if the Ok button has to be enabled.
     *
     * @param item            The tree widget item
     *
     * @param col             The column where the user clicked
     */
    void slotItemClicked( QTreeWidgetItem *item,
                          int col );

    /**
     * This slot is called when the user double clicked an item. It is used to
     * edit the Label field.
     *
     * @param item            The tree widget item
     *
     * @param col             The column where the user clicked
     */
    void slotItemDoubleClicked( QTreeWidgetItem *item,
                                int col );

  private:
    /**
     * Sets up the view
     */
    void setupView();

    /**
     * The list of bookmarks
     */
    QList<Smb4KBookmark *> m_bookmarks;

    /**
     * The tree widget for the potential bookmarks
     */
    QTreeWidget *m_widget;
};

#endif
