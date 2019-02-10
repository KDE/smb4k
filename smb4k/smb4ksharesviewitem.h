/***************************************************************************
    The item for Smb4K's shares view.
                             -------------------
    begin                : Di Dez 5 2006
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

#ifndef SMB4KSHARESVIEWITEM_H
#define SMB4KSHARESVIEWITEM_H

// application specific includes
#include "core/smb4kglobal.h"
#include "smb4ktooltip.h"

// Qt includes
#include <QListWidgetItem>

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
    const SharePtr &shareItem() { return m_share; }

    /**
     * This function updates the encapsulated Smb4KShare object.
     */
    void update();
    
    /**
     * This function returns the tool tip of this item.
     * 
     * @returns the tool tip.
     */
    Smb4KToolTip *tooltip();
    
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
    
    /**
     * The tool tip
     */
    Smb4KToolTip *m_tooltip;
};

#endif

