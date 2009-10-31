/***************************************************************************
    smb4ksharesiconviewitem  -  The items for Smb4K's shares icon view.
                             -------------------
    begin                : Di Dez 5 2006
    copyright            : (C) 2006-2008 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

#ifndef SMB4KSHARESICONVIEWITEM_H
#define SMB4KSHARESICONVIEWITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QListWidgetItem>

// application specific includes
#include <core/smb4kshare.h>
#include <smb4ksharesviewitemdata.h>

// forward declarations
class Smb4KSharesIconView;

/**
 * This class provides the items for the shares icon view
 * of Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesIconViewItem : public QListWidgetItem
{
  public:
    /**
     * The constructor.
     *
     * @param share         The Smb4KShare object that represents the share.
     *
     * @param parent        The parent widget of this item.
     */
    Smb4KSharesIconViewItem( Smb4KShare *share,
                             Smb4KSharesIconView *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSharesIconViewItem();

    /**
     * Returns the data object that carries all information that is needed to
     * set up the item.
     *
     * @returns the Smb4KSharesViewItemData object.
     */
    Smb4KSharesViewItemData *itemData() { return &m_data; }

    /**
     * This function tells the item to show the mount point instead of the
     * UNC (the default).
     *
     * @param show          TRUE if the mount point is to be shown and FALSE
     *                      otherwise.
     */
    void setShowMountPoint( bool show );

    /**
     * This function compares the encapsulated Smb4KShare object with @p item
     * and returns TRUE if they contain equal values.
     *
     * @param item          A Smb4KShare object that should be compared
     *
     * @returns             TRUE if @p item has the same values stored as the
     *                      encapsulated Smb4KShare object.
     */
    bool sameShareObject( Smb4KShare *share );

    /**
     * Replace the encapsulated Smb4KShare object. This function just passes
     * the share object to setupItem() which does all the work.
     *
     * @param share         The new Smb4KShare object
     */
    void replaceShareObject( Smb4KShare *share );

  private:
    /**
     * The item data object
     */
    Smb4KSharesViewItemData m_data;

    /**
     * Set up the icon and text of the item with respect to @p share and @p mountpoint.
     *
     * @param share         The Smb4KShare object.
     *
     * @param mountpoint    If TRUE, the mount point will be shown instead of the
     *                      share name.
     */
    void setupItem( Smb4KShare *share,
                    bool mountpoint = false );
};

#endif

