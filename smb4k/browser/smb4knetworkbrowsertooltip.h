/***************************************************************************
    smb4knetworkbrowsertooltip  -  Tool tip for the network browser.
                             -------------------
    begin                : Sa Jan 20 2007
    copyright            : (C) 2007-2010 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKBROWSERTOOLTIP_H
#define SMB4KNETWORKBROWSERTOOLTIP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QLabel>
#include <QFrame>
#include <QGridLayout>

// Forward declarations:
class Smb4KNetworkBrowserItem;


/**
 * This class provides the tool tip for the network browser
 * of Smb4K. It shows information about the associated share.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KNetworkBrowserToolTip : public QLabel
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param parent          The parent widget of the tool tip.
     */
    Smb4KNetworkBrowserToolTip( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KNetworkBrowserToolTip();

    /**
     * If you need to update the tool tip while it is shown, this is the function
     * you want to use. It rereads the entries from the assossiated
     * Smb4KNetworkBrowserItem object and modifies the tool tip if changes happened.
     */
    void update();

    /**
     * Returns the Smb4KNetworkBrowserItem object for which the tool tip
     * should be shown.
     *
     * @returns a pointer to a Smb4KNetworkBrowserItem object.
     */
    Smb4KNetworkBrowserItem *item() { return m_item; }

    /**
     * Set up the tool tip. This function takes the network browser item @p item
     * for which the tool tip should be shown. Before the tool tip is set up according
     * to the data of @p item, the tool tip is cleared.
     *
     * @param item            The network browser item for which the tool tip should
     *                        be shown.
     */
    void setupToolTip( Smb4KNetworkBrowserItem *item );

    /**
     * Clear the tool tip and reset the internal pointer to the network item. The
     * tool tip will be blank after this function was called.
     */
    void clearToolTip();

    /**
     * This function returns TRUE if the tool tip is cleared, i.e. is blank, and
     * FALSE otherwise.
     *
     * @returns TRUE if the tool tip is cleared.
     */
    bool isCleared() const { return m_cleared; }

  signals:
    /**
     * This signal is emitted when the tool tip is about to be shown.
     *
     * @param item            The network browser item
     */
    void aboutToShow( Smb4KNetworkBrowserItem *item );

    /**
     * This signal is emitted when the tool tip is about to be hidden.
     * 
     * @param item            The network browser item
     */
    void aboutToHide( Smb4KNetworkBrowserItem *item );

  protected:
    /**
     * Reimplemented from QLabel.
     */
    void mousePressEvent( QMouseEvent *e );

    /**
     * Reimplemented from QLabel.
     */
    void leaveEvent( QEvent *e );

    /**
     * Reimplemented from QLabel.
     */
    void showEvent( QShowEvent *e );

    /**
     * Reimplemented from QLabel.
     */
    void hideEvent( QHideEvent *e );

  private:
    /**
     * The pointer to the Smb4KNetworkBrowserItem object
     */
    Smb4KNetworkBrowserItem *m_item;

    /**
     * The layout for the tool tip
     */
    QGridLayout *m_layout;

    /**
     * Workgroup label
     */
    QLabel *m_workgroup_label;

    /**
     * Workgroup
     */
    QLabel *m_workgroup;

    /**
     * Master browser label
     */
    QLabel *m_master_browser_label;

    /**
     * Master browser
     */
    QLabel *m_master_browser;

    /**
     * Host label
     */
    QLabel *m_host_label;

    /**
     * Host
     */
    QLabel *m_host;

    /**
     * Comment label
     */
    QLabel *m_comment_label;

    /**
     * Comment
     */
    QLabel *m_comment;

    /**
     * IP address label
     */
    QLabel *m_ip_label;

    /**
     * IP address
     */
    QLabel *m_ip;

    /**
     * Operating system label
     */
    QLabel *m_os_label;

    /**
     * Operating system
     */
    QLabel *m_os;

    /**
     * Server label
     */
    QLabel *m_server_label;

    /**
     * Server
     */
    QLabel *m_server;

    /**
     * Separating line
     */
    QFrame *m_line;

    /**
     * Share label
     */
    QLabel *m_share_label;

    /**
     * Share
     */
    QLabel *m_share;

    /**
     * Type label
     */
    QLabel *m_type_label;

    /**
     * Type
     */
    QLabel *m_type;

    /**
     * Mounted label
     */
    QLabel *m_mounted_label;

    /**
     * Mounted
     */
    QLabel *m_mounted;

    /**
     * Icon
     */
    QLabel *m_icon;

    /**
     * Is the tool tip cleared?
     */
    bool m_cleared;
};

#endif
