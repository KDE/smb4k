/***************************************************************************
    smb4ksharesiconview  -  This is the shares icon view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
    copyright            : (C) 2006-2012 by Alexander Reinholdt
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

#ifndef SMB4KSHARESICONVIEW_H
#define SMB4KSHARESICONVIEW_H

// Qt includes
#include <QListWidget>
#include <QTimer>

// forward declarations
class Smb4KSharesIconViewItem;
class Smb4KToolTip;


/**
 * This widget class provides the shares icon view of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesIconView : public QListWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent          The parent widget
     */
    Smb4KSharesIconView( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KSharesIconView();

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
     */
    void itemExecuted( QListWidgetItem *item );

    /**
     * This signal is emitted when something has been dropped onto
     * @p item and the drop event was accepted.
     *
     * @param item          The item on which something has been dropped.
     *
     * @param e             The drop event.
     */
    void acceptedDropEvent( Smb4KSharesIconViewItem *item,
                            QDropEvent *e );

  protected:
    /**
     * Reimplemented from QListWidget.
     */
    bool event( QEvent *e );

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
    virtual void dragEnterEvent( QDragEnterEvent *e );

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
    QMimeData *mimeData( const QList<QListWidgetItem *> list ) const;

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
     */
    void slotItemEntered( QListWidgetItem *item );

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
     * The tool tip timer
     */
    QTimer *m_tooltip_timer;

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
    QListWidgetItem *m_auto_select_item;
};

#endif
