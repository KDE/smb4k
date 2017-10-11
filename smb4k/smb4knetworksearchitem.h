/***************************************************************************
    smb4ksearchdialogitem  -  This class is an enhanced version of a list
    box item for Smb4K.
                             -------------------
    begin                : So Jun 3 2007
    copyright            : (C) 2007-2015 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

#ifndef SMB4KNETWORKSEARCHITEM_H
#define SMB4KNETWORKSEARCHITEM_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QListWidget>
#include <QListWidgetItem>

/**
 * This class is an enhanced version of QListWidgetItem, that is used
 * by the search dialog of Smb4K to show the search results. It
 * encapsulates a Smb4KHost which carries all the information needed.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KNetworkSearchItem : public QListWidgetItem
{
  public:
    /**
     * This is the type of the item.
     */
    enum ItemType{ Share = 1001,
                   Failure = 1002 };

    /**
     * The constructor for a search result representing a share. It will
     * construct a list widget item and set its text according to the
     * contents of @p share.
     *
     * @param listWidget        The parent list widget.
     *
     * @param share             The share item that represents the search
     *                          result.
     */
    Smb4KNetworkSearchItem(QListWidget *listWidget, const SharePtr &share);

    /**
     * The constructor for a "The network search returned no results."
     * list widget item.
     *
     * @param listWidget        The parent list widget.
     */
    explicit Smb4KNetworkSearchItem(QListWidget *listWidget);

    /**
     * The destructor
     */
    virtual ~Smb4KNetworkSearchItem();

    /**
     * This function returns the Smb4KShare object that is encapsulated
     * here.
     *
     * @returns the encapsulated Smb4KShare object.
     */
    const SharePtr &shareItem() { return m_share; }

    /**
     * Update the item.
     */
    void update();

  private:
    /**
     * The Smb4KShare object
     */
    SharePtr m_share;

    /**
     * This function sets up the item.
     */
    void setupItem();
};


#endif
