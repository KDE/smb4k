/***************************************************************************
    smb4kshareslistview  -  This is the shares list view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSHARESLISTVIEW_H
#define SMB4KSHARESLISTVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidget>
#include <QTimer>

// KDE includes

// forward declarations
class Smb4KSharesListViewItem;
class Smb4KToolTip;

/**
 * This widget class provides the shares list view of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesListView : public QTreeWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent        The parent widget
     */
    Smb4KSharesListView( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSharesListView();

    /**
     * Enumeration for the columns.
     */
#ifndef __FreeBSD__
    enum Columns { Item = 0,
                   Login = 1,
                   FileSystem = 2,
                   Owner = 3,
                   Free = 4,
                   Used = 5,
                   Total = 6,
                   Usage = 7 };
#else
    enum Columns { Item = 0,
                   FileSystem = 1,
                   Owner = 2,
                   Free = 3,
                   Used = 4,
                   Total = 5,
                   Usage = 6 };
#endif

  /**
   * Returns a pointer to the tooltip.
   * 
   * @returns a pointer to the tooltip.
   */
  Smb4KToolTip *tooltip() { return m_tooltip; }
  
  signals:
    /**
     * This signal is emitted when an item has been executed.
     *
     * @param item          The item that was executed.
     *
     * @param column        The column where the item was executed.
     */
    void itemExecuted( QTreeWidgetItem *item,
                       int column );

    /**
     * This signal is emitted when something has been dropped onto
     * @p item and the drop event was accepted.
     *
     * @param item          The item on which something has been dropped.
     *
     * @param e             The drop event.
     */
    void acceptedDropEvent( Smb4KSharesListViewItem *item,
                            QDropEvent *e );

  protected:
    /**
     * Reimplemented from QTreeWidget.
     */
    bool event( QEvent *e );

    /**
     * Reimplemented from QTreeWidget. This function is used to
     * show the tooltips.
     */
    void mouseMoveEvent( QMouseEvent *e );

    /**
     * Reimplemented from QWidget. This function emits the signal
     * mouseOutside().
     *
     * @param e             The event object
     */
    void leaveEvent( QEvent *e );

    /**
     * Reimplemented from QWidget. This function emits the signal
     * mouseInside().
     *
     * @param e             The event object
     */
    void enterEvent( QEvent *e );

    /**
     * Reimplemented from QAbstractItemView. This function handles
     * mouse press events.
     *
     * @param e             The mouse event object
     */
    void mousePressEvent( QMouseEvent *e );

    /**
     * Reimplemented from QAbstractItemView. This function is used
     * to stop the auto selection.
     *
     * @param e             The focus event
     */
    void focusOutEvent( QFocusEvent *e );

    /**
     * Reimplemented from QWidget. This function is used to handle
     * the tooltip.
     *
     * @param e             The wheel event
     */
    void wheelEvent( QWheelEvent *e );

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drag event
     */
    void dragEnterEvent( QDragEnterEvent *e );

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drag move event
     */
    void dragMoveEvent( QDragMoveEvent *e );

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drop event
     */
    void dropEvent( QDropEvent *e );

    /**
     * Reimplemented to allow only the copy drop action.
     */
    Qt::DropActions supportedDropActions() const;

    /**
     * Reimplemented to allow dragging.
     */
    QMimeData *mimeData( const QList<QTreeWidgetItem *> list ) const;

    /**
     * Reimplemented to allow dragging.
     */
    void startDrag( Qt::DropActions supported );

  protected slots:
    /**
     * This slot is invoked when an item in the shares view has been entered.
     * It is used to display the tool tips.
     *
     * @param item            The item that is under the mouse.
     *
     * @param column          The column where the mouse entered the item.
     */
    void slotItemEntered( QTreeWidgetItem *item,
                          int column );

    /**
     * This slot is invoked when the viewport is entered. It is used
     * to hide the tool tip if needed.
     */
    void slotViewportEntered();

    /**
     * This slot is used to adjust to KDE's settings.
     *
     * @param category      The category where the settings changed.
     */
    void slotKDESettingsChanged( int category );

    /**
     * This slot is used to auto select the item under the mouse.
     */
    void slotAutoSelectItem();

  private:
    /**
     * The tool tip
     */
    Smb4KToolTip *m_tooltip;

    /**
     * Auto-selection timer
     */
    QTimer *m_auto_select_timer;

    /**
     * Is the mouse inside the widget?
     */
    bool m_mouse_inside;

    /**
     * Determines if single or double click is used.
     */
    bool m_use_single_click;

    /**
     * Determines if the cursor is changed over an
     * item in single click mode.
     */
    bool m_change_cursor_over_icon;

    /**
     * Carries the auto select delay.
     */
    int m_auto_select_delay;

    /**
     * The item for auto selection.
     */
    QTreeWidgetItem *m_auto_select_item;
};

#endif

