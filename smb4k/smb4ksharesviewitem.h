/*
    The item for Smb4K's shares view.

    SPDX-FileCopyrightText: 2006-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSHARESVIEWITEM_H
#define SMB4KSHARESVIEWITEM_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QListWidgetItem>
#include <QPointer>
#include <QWidget>

// forward declarations
class Smb4KSharesView;

/**
 * This class provides the items for the shares icon view
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KSharesViewItem : public QListWidgetItem
{
public:
    /**
     * The constructor.
     *
     * @param share         The Smb4KShare object that represents the share.
     *
     * @param parent        The parent widget of this item.
     */
    Smb4KSharesViewItem(Smb4KSharesView *parent, const SharePtr &share);

    /**
     * The destructor
     */
    ~Smb4KSharesViewItem();

    /**
     * This function returns the encapsulated Smb4KShare item.
     *
     * @returns the encapsulated Smb4KShare item.
     */
    const SharePtr &shareItem()
    {
        return m_share;
    }

    /**
     * This function updates the encapsulated Smb4KShare object.
     */
    void update();

    /**
     * This function modifies the alignment according to the @p mode used in
     * the parent list widget.
     */
    void setItemAlignment(QListView::ViewMode mode);

private:
    /**
     * The Smb4KShare item
     */
    SharePtr m_share;
};

#endif
