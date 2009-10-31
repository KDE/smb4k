/***************************************************************************
    smb4ksharesview_part  -This Part includes the shares view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
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

#ifndef SMB4KSHARESVIEW_PART_H
#define SMB4KSHARESVIEW_PART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QGridLayout>

// KDE includes
#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kactionmenu.h>

// applications specific includes
#include <smb4kshareslistview.h>
#include <smb4ksharesiconview.h>

// forward declarations
class Smb4KShare;

/**
 * This is one of the parts of Smb4K. It contains the shares view.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KSharesViewPart : public KParts::Part
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parentWidget        The parent widget
     *
     * @param parent              The parent object
     *
     * @param args                A list of arguments. At the moment there are
     *                            the following arguments defined:
     *                            viewmode="icon"|"list"
     */
    Smb4KSharesViewPart( QWidget *parentWidget = 0,
                         QObject *parent = 0,
                         const QStringList &args = QStringList() );

    /**
     * The destructor.
     */
    virtual ~Smb4KSharesViewPart();

    /**
     * This function creates the KAboutData on request and returns it.
     *
     * @returns the KAboutData object.
     */
    static KAboutData *createAboutData();

  protected:
    /**
     * Reimplemented from KParts::Part.
     */
    void customEvent( QEvent *e );

  protected slots:
    /**
     * This slot is called if the user requests the context menu. It shows
     * the menu with the actions defined for the widget.
     *
     * @param pos                 The position where user clicked.
     */
    void slotContextMenuRequested( const QPoint &pos );

    /**
     * Is called when the selection changed. This slot takes care of the
     * actions being enabled or disabled accordingly. All widget specific
     * stuff has to be done in the browser widget itself.
     *
     * @param item                The selection list view item.
     */
    void slotItemSelectionChanged();

    /**
     * Is called when the user pressed a mouse button somewhere over a list view item.
     * In addition to Smb4KNetworkBrowserPart::slotSelectionChanged() this slot
     * takes care of the actions being enabled or disabled accordingly. All
     * widget specific stuff has to be done in the shares view widget itself.
     *
     * @param item                The list view item where the mouse button was pressed.
     *
     * @param column              The column where the user pressed the mouse
     *                            button.
     */
    void slotItemPressed( QTreeWidgetItem *item,
                          int column );

    /**
     * Is called when the user pressed a mouse button somewhere over a icon view item.
     * In addition to Smb4KNetworkBrowserPart::slotSelectionChanged() this slot
     * takes care of the actions being enabled or disabled accordingly. All
     * widget specific stuff has to be done in the shares view widget itself.
     *
     * @param item                The icon view item where the mouse button was pressed.
     */
    void slotItemPressed( QListWidgetItem *item );

    /**
     * This slot is invoked when the user executed a list view item. It is used to mount
     * shares.
     *
     * @param item                The item that was executed.
     *
     * @param column              The column where the item was executed.
     */
    void slotItemExecuted( QTreeWidgetItem *item,
                           int column );

    /**
     * This slot is invoked when the user executed an icon view item. It is used to mount
     * shares.
     *
     * @param item                The item that was executed.
     */
    void slotItemExecuted( QListWidgetItem *item );

    /**
     * This slot is used to process an accepted drop event in the list view.
     *
     * @param item                The item where the drop event occurred.
     *
     * @param e                   The drop event that encapsulates the necessary data.
     */
    void slotListViewDropEvent( Smb4KSharesListViewItem *item,
                                QDropEvent *e );

    /**
     * This slot is used to process an accepted drop event in the icon view.
     *
     * @param item                The item where the drop event occurred.
     *
     * @param e                   The drop event that encapsulates the necessary data.
     */
    void slotIconViewDropEvent( Smb4KSharesIconViewItem *item,
                                QDropEvent *e );

    /**
     * This slot is called by the Smb4KMounter::updated() signal and updates
     * the list view according to the list that is returned by Smb4KMounter::getShares().
     */
    void slotMountedShares();

    /**
     * This slot is connected to the 'Unmount action'. You will be able to
     * unmount a certain share when activating this slot.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountShare( bool checked );

    /**
     * This slot is connected to the 'Force Unmounting' action and is, thus,
     * only useful under Linux, because only there the possibility for a forced
     * (i.e. lazy) unmount exists.
     *
     * When activating this slot, the selected share will be unmounted, even if
     * it is not accessible or the server already went offline.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotForceUnmountShare( bool checked );

    /**
     * This slot is connected to the 'Unmount All' action. All shares - either of
     * the user or that are present on the system (depending on the settings the
     * user chose) - will be unmounted.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountAllShares( bool checked );

    /**
     * This slot is connected to the 'Synchronize' action. The current item will be
     * synchronized with a local copy (or vice versa) if you activate it.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotSynchronize( bool checked );

    /**
     * This slot is connected to the 'Konsole' action. The mount point of the current
     * share item will be opened in Konsole.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotKonsole( bool checked );

    /**
     * This slot is connected to the 'Konqueror' action. The contents of the current
     * share item will be opened in the file manager.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotFileManager( bool checked );

    /**
     * This slot lets you add a share to the bookmarks.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotAddBookmark( bool checked );

    /**
     * This slot is connected to the Smb4KMounter::aboutToStart() signal.
     *
     * @param share               The Smb4KShare object
     *
     * @param process             The process
     */
    void slotMounterAboutToStart( Smb4KShare *share,
                                  int process );

    /**
     * This slot is connected to the Smb4KMounter::finished() signal.
     *
     * @param share               The Smb4KShare object
     *
     * @param process             The process
     */
    void slotMounterFinished( Smb4KShare *share,
                              int process );

    /**
     * This slot is invoked shortly before the application quits. It is used
     * to save widget related settings.
     */
    void slotAboutToQuit();

  private:
    /**
     * View mode enumeration
     */
    enum ViewMode{ IconMode,
                   ListMode };

    /**
     * The current view mode
     */
    ViewMode m_mode;

    /**
     * The bookmark action has got a shortcut
     */
    bool m_bookmark_shortcut;

    /**
     * This is the container widget for the
     * shares views.
     */
    QWidget *m_container;

    /**
     * This is the layout of the container
     * widget.
     */
    QGridLayout *m_layout;

    /**
     * Set up the actions
     */
    void setupActions();

    /**
     * Set up view
     */
    void setupView();

    /**
     * Load settings
     */
    void loadSettings();

    /**
     * Save settings
     */
    void saveSettings();

    /**
     * The list view.
     */
    Smb4KSharesListView *m_list_view;

    /**
     * The icon view.
     */
    Smb4KSharesIconView *m_icon_view;

    /**
     * The action menu.
     */
    KActionMenu *m_menu;

    /**
     * The menu title
     */
    QAction *m_menu_title;
};

#endif
