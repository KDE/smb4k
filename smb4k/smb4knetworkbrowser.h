/*
    smb4knetworkbrowser  -  The network browser widget of Smb4K.
    -------------------
    begin                : Mo Jan 8 2007
    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNETWORKBROWSER_H
#define SMB4KNETWORKBROWSER_H

// Qt includes
#include <QTreeWidget>

// forward declarations
class Smb4KNetworkBrowserItem;
class Smb4KToolTip;

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
    enum Columns { Network = 0, Type = 1, IP = 2, Comment = 3 };

    /**
     * The tooltip
     */
    Smb4KToolTip *toolTip();

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

    /**
     * Reimplemented from QAbstractItemView. This function handles
     * mouse move events.
     *
     * @param e             The mouse event object
     */
    void mouseMoveEvent(QMouseEvent *e) override;

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
    Smb4KToolTip *m_toolTip;
};

#endif
