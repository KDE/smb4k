/***************************************************************************
    smb4ksharesviewitemdata  -  This class is a container for the widget
    independed data needed by the various parts of the shares view.
                             -------------------
    begin                : Di Jun 3 2008
    copyright            : (C) 2008 by Alexander Reinholdt
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

#ifndef SMB4KSHARESVIEWITEMDATA_H
#define SMB4KSHARESVIEWITEMDATA_H

// Qt includes
#include <QPixmap>
#include <QIcon>

// KDE includes
#include <kicon.h>

// application specific includes
#include <core/smb4kshare.h>

/**
 * This class is a container for the widget independed data needed by the
 * shares view KPart and the tool tip.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesViewItemData
{
  public:
    /**
     * The constructor.
     */
    Smb4KSharesViewItemData();

    /**
     * The destructor.
     */
    ~Smb4KSharesViewItemData();

    /**
     * Set the share object.
     *
     * @param share         The Smb4KShare object
     */
    void setShare( Smb4KShare *share );

    /**
     * Return the share object.
     *
     * @returns the Smb4KShare object.
     */
    Smb4KShare *share() { return &m_share; }

    /**
     * Set if the mount point should be shown.
     *
     * @param show          Set this value to TRUE if the mount point
     *                      should be shown.
     */
    void setShowMountPoint( bool show );

    /**
     * Returns TRUE if the mount point should be shown and FALSE otherwise.
     *
     * @returns TRUE if the mount point should be shown.
     */
    bool showMountPoint() const { return m_show_mountpoint; }

    /**
     * Set the item's icon. It is used to return the various pixmaps this
     * class offers.
     *
     * @param icon          The item's icon.
     *
     * @param mode          The mode the icon should be in.
     *
     * @param
     */
    void setIcon( const QIcon &icon,
                  QIcon::Mode mode = QIcon::Normal,
                  QIcon::State state = QIcon::Off );

    /**
     * Returns the item's pixmap according to @param size.
     *
     * @returns the icon's pixmap.
     */
    QPixmap pixmap( int size ) const;

    /**
     * Returns the item's pixmap according to @param size.
     *
     * @returns the icon's pixmap.
     */
    QPixmap pixmap( const QSize &size ) const;

  private:
    /**
     * The share
     */
    Smb4KShare m_share;

    /**
     * Do we show the mount point?
     */
    bool m_show_mountpoint;

    /**
     * The items icon
     */
    QIcon m_icon;

    /**
     * The icon mode
     */
    QIcon::Mode m_mode;

    /**
     * The icon state
     */
    QIcon::State m_state;

    /**
     * Desktop pixmap
     */
    QPixmap m_desktop_icon;
};

#endif
