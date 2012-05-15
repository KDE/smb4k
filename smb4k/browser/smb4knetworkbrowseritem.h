/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2012 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKBROWSERITEM_H
#define SMB4KNETWORKBROWSERITEM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// application specific includes
#include "core/smb4kworkgroup.h"
#include "core/smb4khost.h"
#include "core/smb4kshare.h"

// Qt includes
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeWidget>

// KDE includes
#include <kicon.h>

class Smb4KNetworkBrowser;
class Smb4KNetworkBrowserItem : public QTreeWidgetItem
{
  public:
    /**
     * The constructor for toplevel (workgroup) items.
     *
     * @param parent        The parent tree widget.
     *
     * @param workgroup     The Smb4KWorkgroup item that carries all the data
     *                      needed to set up a "workgroup item".
     */
    Smb4KNetworkBrowserItem( QTreeWidget *parent,
                             Smb4KWorkgroup *workgroup );

    /**
     * The constructor for the host items.
     *
     * @param parent        The parent tree widget item.
     *
     * @param host          The Smb4KHost item that carries all the data.
     */
    Smb4KNetworkBrowserItem( QTreeWidgetItem *parent,
                             Smb4KHost *host );

    /**
     * The constructor for the share items.
     *
     * @param parent        The parent tree widget item.
     *
     * @param share         The Smb4KShare item that carries all the data.
     */
    Smb4KNetworkBrowserItem( QTreeWidgetItem *parent,
                             Smb4KShare *share );

    /**
     * The destructor.
     */
    virtual ~Smb4KNetworkBrowserItem();

    /**
     * Type of the item.
     */
    enum ItemType{ Workgroup = 1000,
                   Host = 1001,
                   Share = 1002 };

    /**
     * Columns of the item.
     */
    enum Columns{ Network = 0,
                  Type = 1,
                  IP = 2,
                  Comment = 3 };

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KWorkgroup object if it is present or NULL if it is not.
     *
     * @returns a pointer to the workgroup item or NULL.
     */
    Smb4KWorkgroup *workgroupItem();

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KHost object if it is present or NULL if it is not.
     *
     * @returns a pointer to the host item or NULL.
     */
    Smb4KHost *hostItem();

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KShare object if it is present or NULL if it is not.
     *
     * @returns a pointer to the share item or NULL.
     */
    Smb4KShare *shareItem();
    
    /**
     * This function returns the encapsulated Smb4KBasicNetworkItem object.
     * 
     * @returns a pointer to the encapsulated Smb4KBasicNetworkItem object
     * or NULL if there is no item defined (this should never happen).
     */
    Smb4KBasicNetworkItem *networkItem();
    
    /**
     * This function updates the internal network item.
     * 
     * @param item          A Smb4KBasicNetworkItem object
     */
    void update( Smb4KBasicNetworkItem *item );

  private:
    /**
     * The workgroup item
     */
    Smb4KWorkgroup *m_workgroup;

    /**
     * The host item
     */
    Smb4KHost *m_host;

    /**
     * The share item
     */
    Smb4KShare *m_share;
};

#endif
