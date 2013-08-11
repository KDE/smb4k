/***************************************************************************
    smb4kshareslistviewitem  -  The shares list view item class of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
    copyright            : (C) 2007-2013 by Alexander Reinholdt
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

#ifndef SMB4KSHARESLISTVIEWITEM_H
#define SMB4KSHARESLISTVIEWITEM_H

// application specific includes
#include "core/smb4kshare.h"
#include "../tooltips/smb4ktooltip.h"

// Qt includes
#include <QTreeWidgetItem>

// forward declarations
class Smb4KSharesListView;

/**
 * This class provides the items for the shares icon view
 * of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesListViewItem : public QTreeWidgetItem
{
  public:
    /**
     * Enumeration for the columns.
     */
#ifndef Q_OS_FREEBSD
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
     * The constructor.
     *
     * @param parent        The parent widget of this item.
     * 
     * @param share         The Smb4KShare object that represents the share.
     *
     * @param mountpoint    Show the mountpoint instead of the UNC
     */
    Smb4KSharesListViewItem( Smb4KSharesListView *parent,
                             Smb4KShare *share,
                             bool mountpoint = false );

    /**
     * The destructor
     */
    ~Smb4KSharesListViewItem();
    
    /**
     * This function returns the encapsulated Smb4KShare item.
     * 
     * @returns the encapsulated Smb4KShare item.
     */
    Smb4KShare *shareItem() { return m_share; }

    /**
     * This function tells the item to show the mount point instead of the
     * UNC (the default).
     *
     * @param show          TRUE if the mount point is to be shown and FALSE
     *                      otherwise.
     */
    void setShowMountPoint( bool show );
    
    /**
     * This function updates the encapsulated Smb4KShare object.
     * 
     * @param share         The Smb4KShare item that is used for the update
     */
    void update( Smb4KShare *share );
    
    /**
     * This function returns the tool tip of this item.
     * 
     * @returns the tool tip.
     */
    Smb4KToolTip *tooltip();

  private:
    /**
     * The Smb4KShare item
     */
    Smb4KShare *m_share;
    
    /**
     * Show the mountpoint
     */
    bool m_mountpoint;
    
    /**
     * The tool tip
     */
    Smb4KToolTip *m_tooltip;
};

#endif
