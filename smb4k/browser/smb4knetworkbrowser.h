/***************************************************************************
    smb4knetworkbrowser  -  The network browser widget of Smb4K.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2011 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKBROWSER_H
#define SMB4KNETWORKBROWSER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidget>

// forward declarations
class Smb4KNetworkBrowserItem;
class Smb4KToolTip;

class Smb4KNetworkBrowser : public QTreeWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent        The parent widget
     */
    Smb4KNetworkBrowser( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KNetworkBrowser();

    /**
     * Enumeration for the columns in the list view.
     */
    enum Columns{ Network = 0,
                  Type = 1,
                  IP = 2,
                  Comment = 3 };

    /**
     * Returns a pointer to the current tool tip or NULL, if there
     * is no tool tip at the moment. Please note, that the tool tip
     * is generated 2 sec before it is shown.
     *
     * @returns a pointer to the current tool tip.
     */
    Smb4KToolTip *tooltip() const { return m_tooltip; }

    /**
     * This function returns TRUE if the mouse is inside the network
     * browser widget and FALSE otherwise.
     *
     * @returns TRUE if the mouse is inside the widget.
     */
    bool mouseInsideWidget() { return m_mouse_inside; }

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

  protected:
    /**
     * Reimplemented from QWidget.
     */
    bool event( QEvent *e );

    /**
     * Reimplemented from QWidget. This function keeps track of the
     * mouse position and handles the tool tips.
     *
     * @param e             The mouse event
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

  protected slots:
   /**
     * This slot is used to change the cursor over an item if appropriate.
     *
     * @param item          The item that was entered.
     *
     * @param column        The column where the item was entered.
     */
    void slotItemEntered( QTreeWidgetItem *item,
                          int column );

    /**
     * This slot is invoked when the viewport is entered. It is used
     * to hide the tool tip if needed.
     */
    void slotViewportEntered();

    /**
     * This slot is called when the user executed an item. It is used
     * to open the item if it is expandable.
     *
     * @param item          The item that has been activated.
     *
     * @param column        The column where the item was activated.
     */
    void slotItemExecuted( QTreeWidgetItem *item,
                           int column );

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
     * The tool tip for the network browser
     */
    Smb4KToolTip *m_tooltip;

    /**
     * Mouse inside the widget?
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
     * The timer for auto selection.
     */
    QTimer *m_auto_select_timer;

    /**
     * The item for auto selection.
     */
    QTreeWidgetItem *m_auto_select_item;
};

#endif
