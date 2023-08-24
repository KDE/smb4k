/*
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.

    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KNETWORKBROWSERITEM_H
#define SMB4KNETWORKBROWSERITEM_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QTreeWidget>
#include <QTreeWidgetItem>

// forward declarations
class Smb4KNetworkBrowser;

/**
 * This class provides the items for the network neighborhood browser
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KNetworkBrowserItem : public QTreeWidgetItem
{
public:
    /**
     * The constructor for toplevel items.
     *
     * @param parent        The parent tree widget
     * @param item          The network item
     */
    Smb4KNetworkBrowserItem(QTreeWidget *parent, const NetworkItemPtr &item);

    /**
     * The constructor for child items.
     *
     * @param parent        The parent tree widget item.
     * @param item          The network item
     */
    Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, const NetworkItemPtr &item);

    /**
     * The destructor.
     */
    virtual ~Smb4KNetworkBrowserItem();

    /**
     * Columns of the item.
     */
    enum Columns { Network = 0, Type = 1, IP = 2, Comment = 3 };

    /**
     * This function is provided for convenience. It returns a pointer to
     * the Smb4KWorkgroup object if it is present or NULL if it is not.
     *
     * @returns a pointer to the workgroup item or NULL.
     */
    WorkgroupPtr workgroupItem();

    /**
     * This function is provided for convenience. It returns a pointer to
     * the Smb4KHost object if it is present or NULL if it is not.
     *
     * @returns a pointer to the host item or NULL.
     */
    HostPtr hostItem();

    /**
     * This function is provided for convenience. It returns a pointer to
     * the Smb4KShare object if it is present or NULL if it is not.
     *
     * @returns a pointer to the share item or NULL.
     */
    SharePtr shareItem();

    /**
     * This function returns the encapsulated network item.
     *
     * @returns a pointer to the encapsulated Smb4KBasicNetworkItem object
     * or NULL if there is no item defined (this should never happen).
     */
    NetworkItemPtr networkItem();

    /**
     * This function updates the internal network item.
     */
    void update();

private:
    /**
     * The network item
     */
    NetworkItemPtr m_item;
};

#endif
