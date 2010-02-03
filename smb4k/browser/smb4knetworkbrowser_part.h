/***************************************************************************
    smb4knetworkbrowser_part  -  This Part encapsulates the network
    browser of Smb4K.
                             -------------------
    begin                : Fr Jan 5 2007
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

#ifndef SMB4KNETWORKBROWSERPART_H
#define SMB4KNETWORKBROWSERPART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidgetItem>
#include <QList>
#include <QAction>

// KDE includes
#include <kparts/part.h>
#include <kparts/genericfactory.h>
#include <kactionmenu.h>

// forward declarations
class Smb4KNetworkBrowser;
class Smb4KNetworkBrowserItem;
class Smb4KWorkgroup;
class Smb4KHost;
class Smb4KShare;
class Smb4KBasicNetworkItem;

/**
 * This is one of the parts of Smb4K. It contains the network browser.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KNetworkBrowserPart : public KParts::Part
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
     * @param args                A list of arguments. At the moment the following
     *                            arguments are recognized:
     *                            bookmark_shortcut="true"|"false"
     *                            silent="true"|"false"
     */
    Smb4KNetworkBrowserPart( QWidget *parentWidget = 0,
                             QObject *parent = 0,
                             const QStringList &args = QStringList() );

    /**
     * The destructor.
     */
    virtual ~Smb4KNetworkBrowserPart();

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
     * Is called when the user pressed a mouse button somewhere over an item.
     * In addition to Smb4KNetworkBrowserPart::slotSelectionChanged() this slot
     * takes care of the actions being enabled or disabled accordingly. All
     * widget specific stuff has to be done in the browser widget itself.
     *
     * @param item                The item where the mouse button was pressed.
     *
     * @param column              The column where the user pressed the mouse
     *                            button.
     */
    void slotItemPressed( QTreeWidgetItem *item,
                          int column );

    /**
     * This slot is called when an item has been expanded. It is used to
     * invoke the functions of the scanner that need to be run.
     *
     * @param item                The item that has been expanded.
     */
    void slotItemExpanded( QTreeWidgetItem *item );

    /**
     * This slot is called when an item has been collapsed.
     *
     * @param item                The item that has been collapsed.
     */
    void slotItemCollapsed( QTreeWidgetItem *item );

    /**
     * This slot is invoked when the user executed an item. It is used to mount
     * shares.
     *
     * @param item                The item that was executed.
     *
     * @param column              The column where the item was executed.
     */
    void slotItemExecuted( QTreeWidgetItem *item,
                           int column );

    /**
     * This slot is called when the user moved the mouse over an @p item in the
     * network browser and a tool tip is about to be shown. It will initiate any
     * actions that have to be taken by the scanner to complete the information
     * on the network items shown in the browser.
     *
     * @param item                The item for which the additional info should
     *                            be retrieved
     */
    void slotAboutToShowToolTip( Smb4KNetworkBrowserItem *item );

    /**
     * This slot is called when the tool tip is about to be closed. It stops the
     * search for addtitional information if necessary.
     */
    void slotAboutToHideToolTip();

    /**
     * This slot receives the workgroups/domains found by the scanner. It takes
     * a list of workgroup items @p list and inserts the respective workgroups
     * into the browser window. Obsolete items will be deleted from the network
     * browser.
     *
     * @param list                A list of Smb4KWorkgroup item objects
     */
    void slotWorkgroups( const QList<Smb4KWorkgroup *> &list );

    /**
     * This slot receives the list of workgroup/domain members that were found
     * by the scanner. It takes this @p list and inserts it into the list view.
     * Obsolete items will be deleted from the network browser.
     *
     * @param list                A list of Smb4KHostItem objects
     */
    void slotWorkgroupMembers( Smb4KWorkgroup *workgroup,
                               const QList<Smb4KHost *> &list );

    /**
     * This slot receives the list of shared resources a host provides. It takes
     * this @p list and inserts its items as children of @p host into the list
     * view. Obsolete items will be deleted from the network browser.
     *
     * @param host                The host where the shares belong
     *
     * @param list                The list of shares that belong to @p host
     */
    void slotShares( Smb4KHost *host,
                     const QList<Smb4KShare *> &list );

    /**
     * This slot takes a Smb4KHostItem object @p item, reads the IP address entry
     * from it and updates the list view item representing the host with it.
     *
     * @param host                A Smb4KHost item with an updated IP address.
     */
    void slotAddIPAddress( Smb4KHost *host );

    /**
     * This slot adds additional information to a browser item. It takes an
     * Smb4KHostItem @p item, searches the assossiated browser item and updates its
     * contents.
     *
     * @param host                A Smb4KHost item with updated contents.
     */
    void slotAddInformation( Smb4KHost *host );

    /**
     * This slots is connected to the Smb4KScanner::hostAdded() signal and inserts
     * a single host in the list view. If the host is already present, nothing is
     * done.
     *
     * @param host                A Smb4KHost item that is to be added to the
     *                            tree widget.
     */
    void slotInsertHost( Smb4KHost *host );

    /**
     * Rescan the network. This slot is connected to the 'Rescan' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotRescan( bool checked );

    /**
     * Abort a network scan. This slot is connected to the 'Abort' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotAbort( bool checked );

    /**
     * Manually mount a share. This slot is connected to the 'Mount Manually'
     * action and opens a mount dialog.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotMountManually( bool mount );

    /**
     * Provide authentication for the current network object. This slot is
     * connected to the 'Authentication' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotAuthentication( bool checked );

    /**
     * Provide custom options for a server or share. This slot is connected
     * to the 'Custom Options' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotCustomOptions( bool checked );

    /**
     * Bookmark a remote share. This slot is connected to the 'Add Bookmark'
     * action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotAddBookmark( bool checked );

    /**
     * Preview a share. This slot is connected to the 'Preview' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotPreview( bool checked );

    /**
     * Print a document on a remote printer. This slot is connected to the
     * 'Print File' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotPrint( bool checked );

    /**
     * Mount a remote share. This slot is connected to the 'Mount' action.
     *
     * @param checked             Is TRUE if the action is checked (not used here).
     */
    void slotMount( bool checked );

    /**
     * This slot is connected to the Smb4KScanner::aboutToStart() signal.
     *
     * @param item                The Smb4KBasicNetworkItem object
     *
     * @param process             The process
     */
    void slotScannerAboutToStart( Smb4KBasicNetworkItem *item,
                                  int process );

    /**
     * This slot is connected to the Smb4KScanner::finished() signal.
     *
     * @param item                The Smb4KBasicNetworkItem object
     *
     * @param process             The process
     */
    void slotScannerFinished( Smb4KBasicNetworkItem *item,
                              int process );

    /**
     * This slot is connected to the Smb4KMounter::aboutToStart() signal.
     *
     * @param share               The Smb4KShare object
     *
     * @param process             The process
     */
    void slotMounterAboutToStart( Smb4KShare *share,
                                  int process);

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
     * This slot is called whenever a share has been mounted. It marks the
     * respective share in the tree widget as mounted.
     *
     * @param share               The Smb4KShare object
     */
    void slotShareMounted( Smb4KShare *share );

    /**
     * This slot is called whenever a share has been unmounted. It marks the
     * respective share in the tree widget as not mounted.
     *
     * @param share               The Smb4KShare object
     */
    void slotShareUnmounted( Smb4KShare *share );

    /**
     * This slot is invoked shortly before the application quits. It is used
     * to save widget related settings.
     */
    void slotAboutToQuit();

  private:
    /**
     * Set up the actions
     */
    void setupActions();

    /**
     * Load settings
     */
    void loadSettings();

    /**
     * Save settings
     */
    void saveSettings();

    /**
     * The action menu
     */
    KActionMenu *m_menu;

    /**
     * The network browser widget
     */
    Smb4KNetworkBrowser *m_widget;

    /**
     * The bookmark action has got a shortcut
     */
    bool m_bookmark_shortcut;
    
    /**
     * Emit status messages
     */
    bool m_silent;

    /**
     * The menu title
     */
    QAction *m_menu_title;
};

#endif
