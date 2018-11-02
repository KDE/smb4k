/***************************************************************************
    The network neighborhood browser dock widget 
                             -------------------
    begin                : Sat Apr 28 2018
    copyright            : (C) 2018 by Alexander Reinholdt
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

#ifndef SMB4KNETWORKBROWSERDOCKWIDGET_H
#define SMB4KNETWORKBROWSERDOCKWIDGET_H

// application specific includes
#include "smb4knetworkbrowser.h"
#include "core/smb4kglobal.h"

// Qt includes
#include <QDockWidget>

// KDE includes
#include <KWidgetsAddons/KActionMenu>
#include <KXmlGui/KActionCollection>

using namespace Smb4KGlobal;


class Smb4KNetworkBrowserDockWidget : public QDockWidget
{
  Q_OBJECT
  
  public:
    /**
    * Constructor
    */
    Smb4KNetworkBrowserDockWidget(const QString &title, QWidget *parent = 0);
    
    /**
    * Destructor
    */
    ~Smb4KNetworkBrowserDockWidget();
    
    /**
     * Load settings
     */
    void loadSettings();

    /**
     * Save settings
     */
    void saveSettings();
    
    /**
     * Returns the action collection of this dock widget
     * @returns the action collection
     */
    KActionCollection *actionCollection();
    
  protected Q_SLOTS:
    /**
     * This slot is called if the user requests the context menu. It shows
     * the menu with the actions defined for the widget.
     * @param pos                 The position where user clicked.
     */
    void slotContextMenuRequested(const QPoint &pos);
    
    /**
     * This slot is invoked when the user activated an item in the network
     * neighborhood browser.
     * @param item                The item that was executed.
     * @param column              The column where the item was executed.
     */
    void slotItemActivated(QTreeWidgetItem *item, int column);
    
    /**
     * Is called when the selection changed. This slot takes care of the
     * actions being enabled or disabled accordingly. All widget specific
     * stuff has to be done in the browser widget itself.
     * @param item                The selection list view item.
     */
    void slotItemSelectionChanged();
    
    /**
     * This slot is connected to the Smb4KClient::aboutToStart() signal.
     * 
     * @param item                The Smb4KBasicNetworkItem object
     * @param process             The process
     */
    void slotClientAboutToStart(const NetworkItemPtr &item, int process);

    /**
     * This slot is connected to the Smb4KClient::finished() signal.
     * 
     * @param item                The Smb4KBasicNetworkItem object
     * @param process             The process
     */
    void slotClientFinished(const NetworkItemPtr &item, int process);    
    
    /**
     * This slot is called when an authentication error occurred. Depending on
     * the @p process parameter it will take respective actions.
     * @param host                The host where the authentication error occurred
     * @param process             The process (action) that was running while the
     *                            authentication error occurred
     */
    void slotAuthError(const HostPtr &host, int process);
    
    /**
     * This slot is called when workgroups/domains were discovered
     */
    void slotWorkgroups();
    
    /**
     * This slot is called when the list of servers of workgroup/domain 
     * @p workgroup was discovered. 
     * @param workgroup           The workgroup/domain that was queried
     */
    void slotWorkgroupMembers(const WorkgroupPtr &workgroup);
    
    /**
     * This slot is called when the list of shared resources of host @p host was
     * queried.
     * @param host                The host that was queried
     */
    void slotShares(const HostPtr &host);
    
    /**
     * Rescan the network or abort a network scan.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotRescanAbortActionTriggered(bool checked);
    
    /**
     * Bookmark a remote share. This slot is connected to the 'Add Bookmark'
     * action.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotAddBookmark(bool checked);
    
    /**
     * Manually mount a share. This slot is connected to the 'Mount Manually'
     * action and opens a mount dialog.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotMountManually(bool checked);
    
    /**
     * Provide authentication for the current network object. This slot is
     * connected to the 'Authentication' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotAuthentication(bool checked);
    
    /**
     * Provide custom options for a server or share. This slot is connected
     * to the 'Custom Options' action.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotCustomOptions(bool checked);
    
    /**
     * Preview a share. This slot is connected to the 'Preview' action.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotPreview(bool checked);
    
    /**
     * Print a document on a remote printer. This slot is connected to the
     * 'Print File' action.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotPrint(bool checked);
    
    /**
     * Mount or unmount a share. This slot is connected to the 'Mount'/'Unmount' dual action.
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotMountActionTriggered(bool checked);
    
    /**
     * Change the state of the 'Mount'/'Unmount' dual action.
     * @param active              TRUE if the action is in the active state.
     */
    void slotMountActionChanged(bool active);
    
    /**
     * This slot is connected to the Smb4KScanner::aboutToStart() signal.
     * @param item                The Smb4KBasicNetworkItem object
     * @param process             The process
     */
//     void slotScannerAboutToStart(const NetworkItemPtr &item, int process);

    /**
     * This slot is connected to the Smb4KScanner::finished() signal.
     * @param item                The Smb4KBasicNetworkItem object
     * @param process             The process
     */
//     void slotScannerFinished(const NetworkItemPtr &item, int process);
    
    /**
     * This slot is called whenever a share has been mounted. It marks the
     * respective share in the tree widget as mounted.
     * @param share               The Smb4KShare object
     */
    void slotShareMounted(const SharePtr &share);

    /**
     * This slot is called whenever a share has been unmounted. It marks the
     * respective share in the tree widget as not mounted.
     * @param share               The Smb4KShare object
     */
    void slotShareUnmounted(const SharePtr &share);
    
    /**
     * This slot is connected to the Smb4KMounter::aboutToStart() signal.
     * @param process             The process
     */
    void slotMounterAboutToStart(int process);

    /**
     * This slot is connected to the Smb4KMounter::finished() signal.
     * @param process             The process
     */
    void slotMounterFinished(int process);
    
    /**
     * This slot is called if the icon size was changed.
     * @param group               The icon group
     */
    void slotIconSizeChanged(int group);
    
  private:
    /**
     * Set up the actions
     */
    void setupActions();
    
    /**
     * The network browser
     */
    Smb4KNetworkBrowser *m_networkBrowser;
    
    /**
     * Action collection
     */
    KActionCollection *m_actionCollection;
    
    /**
     * Context menu
     */
    KActionMenu *m_contextMenu;
};

#endif
