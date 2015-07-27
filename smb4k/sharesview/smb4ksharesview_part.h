/***************************************************************************
    smb4ksharesview_part  -This Part includes the shares view of Smb4K.
                             -------------------
    begin                : Sa Jun 30 2007
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
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KSHARESVIEW_PART_H
#define SMB4KSHARESVIEW_PART_H

// applications specific includes
#include "smb4kshareslistview.h"
#include "smb4ksharesiconview.h"

// Qt includes
#include <QtWidgets/QGridLayout>

// KDE includes
#include <KParts/Part>
#include <KWidgetsAddons/KActionMenu>

// forward declarations
class Smb4KShare;

/**
 * This is one of the parts of Smb4K. It contains the shares view.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Q_DECL_EXPORT Smb4KSharesViewPart : public KParts::Part
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
     *                            bookmark_shortcut="true"|"false"
     *                            silent="true"|"false"
     */
    explicit Smb4KSharesViewPart(QWidget *parentWidget = 0,
                                 QObject *parent = 0,
                                 const QList<QVariant> &args = QList<QVariant>());

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
    void customEvent(QEvent *e);

  protected slots:
    /**
     * This slot is called if the user requests the context menu. It shows
     * the menu with the actions defined for the widget.
     *
     * @param pos                 The position where user clicked.
     */
    void slotContextMenuRequested(const QPoint &pos);

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
    void slotItemPressed(QTreeWidgetItem *item, int column);

    /**
     * Is called when the user pressed a mouse button somewhere over a icon view item.
     * In addition to Smb4KNetworkBrowserPart::slotSelectionChanged() this slot
     * takes care of the actions being enabled or disabled accordingly. All
     * widget specific stuff has to be done in the shares view widget itself.
     *
     * @param item                The icon view item where the mouse button was pressed.
     */
    void slotItemPressed(QListWidgetItem *item);

    /**
     * This slot is invoked when the user activated a list view item. It is used to mount
     * shares.
     * @param item                The item that was executed.
     * @param column              The column where the item was executed.
     */
    void slotItemActivated(QTreeWidgetItem *item, int column);

    /**
     * This slot is invoked when the user activated an icon view item. It is used to mount
     * shares.
     * @param item                The item that was executed.
     */
    void slotItemActivated(QListWidgetItem *item);

    /**
     * This slot is used to process an accepted drop event in the list view.
     *
     * @param item                The item where the drop event occurred.
     *
     * @param e                   The drop event that encapsulates the necessary data.
     */
    void slotListViewDropEvent(Smb4KSharesListViewItem *item, QDropEvent *e);

    /**
     * This slot is used to process an accepted drop event in the icon view.
     *
     * @param item                The item where the drop event occurred.
     *
     * @param e                   The drop event that encapsulates the necessary data.
     */
    void slotIconViewDropEvent(Smb4KSharesIconViewItem *item, QDropEvent *e);
                                
    /**
     * This slot is connected to the Smb4KMounter::mounted() signal and adds the 
     * mounted share @p share to the shares view.
     * 
     * @param share               The Smb4KShare item
     */
    void slotShareMounted(Smb4KShare *share);
    
    /**
     * This slot is connected to the Smb4KMounter::unmounted() signal and removes
     * the share @p share from the share view.
     * 
     * @param share               The Smb4KShare item
     */
    void slotShareUnmounted(Smb4KShare *share);
    
    /**
     * This slot is connected to the Smb4KMounter::updated() signal and updates
     * the item in the shares view corresponding to @p share.
     * 
     * This slot does not remove or add any share, it only updates the present 
     * items.
     * 
     * @param share               The Smb4KShare item
     */
    void slotShareUpdated(Smb4KShare *share);

    /**
     * This slot is connected to the 'Unmount action'. You will be able to
     * unmount a certain share when activating this slot.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountShare(bool checked);

    /**
     * This slot is connected to the 'Unmount All' action. All shares - either of
     * the user or that are present on the system (depending on the settings the
     * user chose) - will be unmounted.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotUnmountAllShares(bool checked);

    /**
     * This slot is connected to the 'Synchronize' action. The current item will be
     * synchronized with a local copy (or vice versa) if you activate it.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotSynchronize(bool checked);

    /**
     * This slot is connected to the 'Konsole' action. The mount point of the current
     * share item will be opened in Konsole.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotKonsole(bool checked);

    /**
     * This slot is connected to the 'Konqueror' action. The contents of the current
     * share item will be opened in the file manager.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotFileManager(bool checked);

    /**
     * This slot lets you add a share to the bookmarks.
     *
     * @param checked             TRUE if the action is checked and FALSE otherwise.
     */
    void slotAddBookmark(bool checked);

    /**
     * This slot is connected to the Smb4KMounter::aboutToStart() signal.
     *
     * @param share               The Smb4KShare object
     *
     * @param process             The process
     */
    void slotMounterAboutToStart(Smb4KShare *share, int process);

    /**
     * This slot is connected to the Smb4KMounter::finished() signal.
     *
     * @param share               The Smb4KShare object
     *
     * @param process             The process
     */
    void slotMounterFinished(Smb4KShare *share, int process);

    /**
     * This slot is invoked shortly before the application quits. It is used
     * to save widget related settings.
     */
    void slotAboutToQuit();
    
    /**
     * This slot is called if the icon size was changed.
     *
     * @param group               The icon group
     */
    void slotIconSizeChanged(int group);
    
    /**
     * Enable or disable the Open With action
     */
    void slotEnableOpenWithAction();

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
     * Emit status messages
     */
    bool m_silent;

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
};

#endif
