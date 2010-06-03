/***************************************************************************
    smb4ksystemtray  -  This is the system tray window class of Smb4K.
                             -------------------
    begin                : Mi Jun 13 2007
    copyright            : (C) 2007-2009 by Alexander Reinholdt
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

#ifndef SMB4KSYSTEMTRAY_H
#define SMB4KSYSTEMTRAY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QAction>
#include <QActionGroup>

// KDE includes
#include <kstatusnotifieritem.h>
#include <kactionmenu.h>

// forward declarations
class Smb4KShare;
class Smb4KWorkgroup;

class Smb4KSystemTray : public KStatusNotifierItem
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent        The parent widget of the system tray window
     */
    Smb4KSystemTray( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KSystemTray();

    /**
     * This function (re-)loads the settings for this widget. It basically just
     * runs the slot slotSetupBookmarkMenu() and slotSetupSharesMenu(), that will
     * do everything to properly set up the menus.
     *
     * This slot *does not* manage the appearance (or disappearance) of this widget
     * in the system tray. You need to use embed() to do this.
     */
    void loadSettings();

  signals:
    /**
     * This signal is emitted when the config dialog has been closed and the
     * settings changed.
     *
     * This signal is forwared from @see Smb4KConfigDialog.
     */
    void settingsChanged( const QString &dialogName );

  protected slots:
    /**
     * This slot opens the manual mount dialog.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotMountDialog( bool checked );

    /**
     * This slot opens the configurations dialog.
     */
    void slotConfigDialog();

    /**
     * This slot is invoked when the config dialog is closed and the settings have
     * been changed. Emits the reloadSettings() signal and adjusts the system tray
     * widget to the new settings afterwards.
     *
     * @param dialogName      The name of the dialog.
     */
    void slotSettingsChanged( const QString &dialogName );

    /**
     * This slot is invoked when the bookmarks have been updated. It sets up the
     * bookmark menu, inserts the bookmark actions into it and automatically
     * disables them if they were already mounted (see slotSetupSharesMenu() as well).
     */
    void slotSetupBookmarksMenu();

    /**
     * This slot opens the bookmark editor.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotBookmarkEditor( bool checked );

    /**
     * This slot is called when a bookmark action has been triggered. It initializes the
     * mounting of the represented share.
     *
     * @param action        The bookmark action.
     */
    void slotBookmarkTriggered( QAction *action );

    /**
     * This slot enables or disables bookmarks. If a @p share is mounted that is represented
     * by a bookmark, the bookmark will be disabled. It will be enabled, when the @p share is 
     * unmounted again.
     * 
     * @param share         The share item
     */
    void slotEnableBookmarks( Smb4KShare *share );

    /**
     * This slot initializes the umounting of all shares. It is connected to the
     * "Unmount All" action of the "Mounted Shares" menu.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotUnmountAllTriggered( bool checked );

    /**
     * This slot is called whan a share related action has been triggered. It
     * executes all actions that can be performed: Unmounting, Synchronization,
     * etc.
     *
     * @param action        The action that has been triggered.
     */
    void slotShareActionTriggered( QAction *action );
    
    /**
     * React on the mounting/unmounting of shares.
     * 
     * @param share         The share that was mounted or unmounted
     */
    void slotMountEvent();
    
    /**
     * React on the finding of workgroups/domains.
     * 
     * @param workgroups    The list of workgroups that was found
     */
    void slotNetworkEvent();

  private:
    /**
     * This function sets up the "Mounted Shares" menu and also enables/disables 
     * the bookmarks in the "Bookmarks" menu.
     */
    void setupSharesMenu();
    
    /**
     * The action menu for the bookmarks.
     */
    KActionMenu *m_bookmarks_menu;

    /**
     * The action menu for the mounted shares.
     */
    KActionMenu *m_shares_menu;

    /**
     * This QActionGroup manages the bookmark actions.
     */
    QActionGroup *m_bookmarks;

    /**
     * This QActionGroup manages all menus representing a share.
     */
    QActionGroup *m_share_menus;

    /**
     * This QActionGoup manages all actions associated with
     * shares.
     */
    QActionGroup *m_shares_actions;
};

#endif
