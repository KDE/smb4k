/***************************************************************************
    This is the shares view of Smb4K.
                             -------------------
    begin                : Mo Dez 4 2006
    copyright            : (C) 2006-2019 by Alexander Reinholdt
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

#ifndef SMB4KSHARESVIEW_H
#define SMB4KSHARESVIEW_H

// Qt includes
#include <QTimer>
#include <QMimeData>
#include <QListWidget>

// forward declarations
class Smb4KSharesViewItem;
class Smb4KToolTip;


/**
 * This widget class provides the shares view of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KSharesView : public QListWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent        The parent widget
     */
    explicit Smb4KSharesView(QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KSharesView();
    
    /**
     * Set the view mode.
     * 
     * @param mode          The view mode
     * @param iconSize      The size of the icons
     */
    void setViewMode(ViewMode mode, int iconSize);

  signals:
    /**
     * This signal is emitted when something has been dropped onto
     * @p item and the drop event was accepted.
     *
     * @param item          The item on which something has been dropped.
     *
     * @param e             The drop event.
     */
    void acceptedDropEvent(Smb4KSharesViewItem *item, QDropEvent *e);
    
    /**
     * This signal is emitted when a tool tip is about to be shown.
     * 
     * @param item          The shares list view item
     */
    void aboutToShowToolTip(Smb4KSharesViewItem *item);
    
    /**
     * This signal is emitted when a tool tip is about to be hidden.
     * 
     * @param item          The shares list view item
     */
    void aboutToHideToolTip(Smb4KSharesViewItem *item);

  protected:
    /**
     * Reimplemented from QListWidget.
     */
    bool event(QEvent *e);

    /**
     * Reimplemented from QWidget. This function emits the signal
     * mouseOutside().
     *
     * @param e             The event object
     */
    void leaveEvent(QEvent *e);

    /**
     * Reimplemented from QWidget. This function emits the signal
     * mouseInside().
     *
     * @param e             The event object
     */
    void enterEvent(QEvent *e);

    /**
     * Reimplemented from QAbstractItemView. This function handles
     * mouse press events.
     *
     * @param e             The mouse event object
     */
    void mousePressEvent(QMouseEvent *e);

    /**
     * Reimplemented from QAbstractItemView. This function is used
     * to stop the auto selection.
     *
     * @param e             The focus event
     */
    void focusOutEvent(QFocusEvent *e);

    /**
     * Reimplemented from QWidget. This function is used to handle
     * the tooltip.
     *
     * @param e             The wheel event
     */
    void wheelEvent(QWheelEvent *e);

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drag event
     */
    virtual void dragEnterEvent(QDragEnterEvent *e);

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drag move event
     */
    void dragMoveEvent(QDragMoveEvent *e);

    /**
     * Reimplemented to allow dragging and dropping.
     *
     * @param e             The drop event
     */
    void dropEvent(QDropEvent *e);

    /**
     * Reimplemented to allow only the copy drop action.
     */
    Qt::DropActions supportedDropActions() const;

    /**
     * Reimplemented to allow dragging.
     */
    QMimeData *mimeData(const QList<QListWidgetItem *> list) const;

    /**
     * Reimplemented to allow dragging.
     */
    void startDrag(Qt::DropActions supported);

  protected slots:
    /**
     * This slot is invoked when an item in the shares view has been entered.
     * It is used to display the tool tips.
     *
     * @param item            The item that is under the mouse.
     */
    void slotItemEntered(QListWidgetItem *item);

    /**
     * This slot is invoked when the viewport is entered. It is used
     * to hide the tool tip if needed.
     */
    void slotViewportEntered();

  private:
    /**
     * The item for that a tool tip is shown
     */
    Smb4KSharesViewItem *m_tooltipItem;

    /**
     * Is the mouse inside the widget?
     */
    bool m_mouseInside;
};

#endif
