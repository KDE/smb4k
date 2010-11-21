/***************************************************************************
    smb4ksearchdialogitem  -  This class is an enhanced version of a list
    box item for Smb4K.
                             -------------------
    begin                : So Jun 3 2007
    copyright            : (C) 2007-2008 by Alexander Reinholdt
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

#ifndef SMB4KNETWORGSEARCHITEM_H
#define SMB4KNETWORHSEARCHITEM_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QListWidgetItem>

// KDE includes
#include <klistwidget.h>

// system includes
#include <core/smb4khost.h>
#include <core/smb4kshare.h>

/**
 * This class is an enhanced version of QListWidgetItem, that is used
 * by the search dialog of Smb4K to show the search results. It
 * encapsulates a Smb4KHost which carries all the information needed.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KNetworkSearch;
class Smb4KNetworkSearchItem : public QListWidgetItem
{
  public:
    /**
     * This is the type of the item.
     */
    enum ItemType{ Host = 1001,
                   Share = 1002,
                   Failure = 1003 };

    /**
     * The constructor for a search result representing a host. It will
     * construct a list widget item and set its text according to the
     * contents of @p host.
     *
     * @param listWidget        The parent list widget.
     *
     * @param host              The host item that represents the search
     *                          result.
     */
    Smb4KNetworkSearchItem( KListWidget *listWidget,
                            Smb4KHost *host );

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
    Smb4KNetworkSearchItem( KListWidget *listWidget,
                            Smb4KShare *share );

    /**
     * The constructor for a "The network search returned no results."
     * list widget item.
     *
     * @param listWidget        The parent list widget.
     */
    Smb4KNetworkSearchItem( KListWidget *listWidget );

    /**
     * The destructor
     */
    ~Smb4KNetworkSearchItem();

    /**
     * This function returns the Smb4KHost object that is encapsulated
     * here.
     *
     * @returns the encapsulated Smb4KHost object.
     */
    Smb4KHost *hostItem() { return &m_host; }

    /**
     * This function returns the Smb4KShare object that is encapsulated
     * here.
     *
     * @returns the encapsulated Smb4KShare object.
     */
    Smb4KShare *shareItem() { return &m_share; }

    /**
     * This function notifies the item that the host that it represents is
     * known to the application, i.e. it is in the list of hosts. It will set
     * the icon and m_is_known accordingly..
     *
     * @param known             Should be TRUE if the host is known.
     */
    void setKnown( bool known );

    /**
     * This function returns TRUE, if the item is already known to the application.
     *
     * @returns TRUE if the item is already known.
     */
    bool isKnown() const { return m_is_known; }

    /**
     * This functions notifies the item that the share that it represents
     * is mounted, i.e. it is in the list of mounted shares. It will set the
     * icon and m_is_mounted accordingly.
     *
     * @param mounted           Should be TRUE if the share is mounted.
     */
    void setMounted( bool mounted );

    /**
     * This function returns TRUE if the item is already mounted.
     *
     * @returns TRUE if the item is already mounted.
     */
    bool isMounted() const { return m_is_mounted; }

  private:
    /**
     * The Smb4KHost object
     */
    Smb4KHost m_host;

    /**
     * The Smb4KShare object
     */
    Smb4KShare m_share;

    /**
     * TRUE if the item is known
     */
    bool m_is_known;

    /**
     * TRUE if the item is mounted
     */
    bool m_is_mounted;

    /**
     * This function sets up the item.
     */
    void setupItem();
};


#endif
