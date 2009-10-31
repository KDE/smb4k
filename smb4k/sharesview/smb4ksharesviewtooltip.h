/***************************************************************************
    smb4ksharesviewtooltip  -  Tool tip for the shares view.
                             -------------------
    begin                : So Jul 8 2007
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

#ifndef SMB4KSHARESVIEWTOOLTIP_H
#define SMB4KSHARESVIEWTOOLTIP_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QLabel>
#include <QGridLayout>

// Forward declarations:
class Smb4KSharesViewItemData;

/**
 * This class provides the tool tip for the shares view of Smb4K. 
 * It shows information about the associated share.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesViewToolTip : public QLabel
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent      The parent widget of this tool tip.
     */
    Smb4KSharesViewToolTip( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KSharesViewToolTip();

    /**
     * If you need to update the tool tip while it is shown, this is the function
     * you want to use. It rereads the entries from the assossiated
     * Smb4KSharesViewItem object and modifies the tool tip if changes happened.
     */
    void update();

    /**
     * The Smb4KSharesListViewItemData object of the share item for which the
     * tool tip should be shown.
     *
     * @returns a pointer to a Smb4KSharesViewItemData object.
     */
    Smb4KSharesViewItemData *data() { return m_data; }

   /**
     * Set up the tool tip. This function takes the data object @p data of a shares
     * view item for which the tool tip should be shown. Before the tool tip is set
     * up according to @p data, it is cleared.
     *
     * @param data            The data object of a shares view item for which the
     *                        tool tip should be shown.
     */
    void setupToolTip( Smb4KSharesViewItemData *data );

    /**
     * Clear the tool tip and reset the internal pointer to the shares view item. The
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
     * @param data            The shares view item data.
     */
    void aboutToShow( Smb4KSharesViewItemData *data );

    /**
     * This signal is emitted when the tool tip is about to be hidden.
     */
    void aboutToHide();

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
     * The item for which the tool tip should be shown
     */
    Smb4KSharesViewItemData *m_data;

    /**
     * The layout of the tool tip.
     */
    QGridLayout *m_layout;

    /**
     * The UNC label
     */
    QLabel *m_unc_label;

    /**
     * The UNC
     */
    QLabel *m_unc;

    /**
     * The mount point label
     */
    QLabel *m_mount_point_label;

    /**
     * The mount point
     */
    QLabel *m_mount_point;

    /**
     * The owner label
     */
    QLabel *m_owner_label;

    /**
     * The owner
     */
    QLabel *m_owner;

    /**
     * The CIFS login label
     */
    QLabel *m_login_label;

    /**
     * The CIFS login
     */
    QLabel *m_login;

    /**
     * The file system label
     */
    QLabel *m_file_system_label;

    /**
     * The file system
     */
    QLabel *m_file_system;

    /**
     * The horizontal line
     */
    QFrame *m_line;

    /**
     * The free disk space label
     */
    QLabel *m_free_label;

    /**
     * The free disk space
     */
    QLabel *m_free;

    /**
     * The used disk space label
     */
    QLabel *m_used_label;

    /**
     * The used disk space
     */
    QLabel *m_used;

    /**
     * The total disk space label
     */
    QLabel *m_total_label;

    /**
     * The total disk space
     */
    QLabel *m_total;

    /**
     * The disk usage label
     */
    QLabel *m_usage_label;

    /**
     * The disk usage
     */
    QLabel *m_usage;

    /**
     * This label holds the pixmap.
     */
    QLabel *m_icon;

    /**
     * The inaccessible label
     */
    QLabel *m_inaccessible;

    /**
     * Is the tool tip cleared?
     */
    bool m_cleared;
};

#endif
