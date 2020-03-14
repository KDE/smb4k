/***************************************************************************
    smb4knetworkbrowser  -  The network browser widget of Smb4K.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2019 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKBROWSER_H
#define SMB4KNETWORKBROWSER_H

// Qt includes
#include <QTreeWidget>

// KDE includes
#include <KWidgetsAddons/KToolTipWidget>

// forward declarations
class Smb4KNetworkBrowserItem;

/**
 * This is the network neighborhood browser widget.
 * 
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KNetworkBrowser : public QTreeWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent        The parent widget
     */
    explicit Smb4KNetworkBrowser(QWidget *parent = 0);

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

  protected:
    /**
     * Reimplemented from QWidget.
     */
    bool event(QEvent *e) override;

    /**
     * Reimplemented from QAbstractItemView. This function handles
     * mouse press events.
     *
     * @param e             The mouse event object
     */
    void mousePressEvent(QMouseEvent *e) override;

  protected slots:
    /**
     * This slot is called when the user activated an item. It is used
     * to open the item if it is expandable.
     * @param item          The item that has been activated.
     * @param column        The column where the item was activated.
     */
    void slotItemActivated(QTreeWidgetItem *item, int column);

    /**
     * Take care that only shares are selected when the user marks multiple
     * shares.
     */    
    void slotItemSelectionChanged();

  private:
    /**
     * The tool top widget
     */
    KToolTipWidget *m_toolTip;
};

#endif
