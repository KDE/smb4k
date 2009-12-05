/***************************************************************************
    smb4kmainwindow  -  The main window of Smb4K.
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2009 by Alexander Reinholdt
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

#ifndef SMB4KMAINWINDOW_H
#define SMB4KMAINWINDOW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QActionGroup>
#include <QLabel>
#include <QProgressBar>

// KDE includes
#include <kxmlguiwindow.h>
#include <kparts/mainwindow.h>
#include <kparts/part.h>

// forward declarations
class Smb4KSystemTray;
class Smb4KBasicNetworkItem;
class Smb4KShare;
class Smb4KPrintInfo;
class Smb4KSynchronizationInfo;
class Smb4KPreviewItem;

/**
 * This is the main window of Smb4K. It provides the network browser, the
 * shares view and all other dialogs as well as a menu, status and tool bar.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KMainWindow : public KParts::MainWindow
{
  Q_OBJECT

  public:
    /**
     * The constructor
     */
    Smb4KMainWindow();

    /**
     * The destructor
     */
    ~Smb4KMainWindow();

  protected:
    /**
     * Reimplemented from KMainWindow.
     */
    bool queryClose();

    /**
     * Reimplemented from KMainWindow.
     */
    bool queryExit();

    /**
     * Reimplemented from QObject.
     */
    void timerEvent( QTimerEvent *e );

  protected slots:
    /**
     * Quits the application.
     */
    void slotQuit();

    /**
     * Opens the configuration dialog.
     */
    void slotConfigDialog();

    /**
     * Reloads settings, if necessary. This slot is connected to
     * the KConfigDialog::settingsChanged() signal.
     *
     * @param dialogName      The name of the configuration dialog
     */
    void slotSettingsChanged( const QString &dialogName );

    /**
     * This slot opens the bookmark editor.
     *
     * @param checked         TRUE when the action is checked and
     *                        FALSE otherwise.
     */
    void slotOpenBookmarkEditor( bool checked );

    /**
     * This slot is called when the list of bookmarks was updated.
     */
    void slotBookmarksUpdated();

    /**
     * This slot enables/disables the bookmarks due to mount events.
     * 
     * @param share           The share item that changed
     */
    void slotEnableBookmarks( Smb4KShare *share );

    /**
     * This slot is called when a bookmark action has been triggered. It initializes the
     * mounting of the represented share.
     *
     * @param action        The triggered bookmark action
     */
    void slotBookmarkTriggered( QAction *action );

    /**
     * This slot is called when a bookmark should be added and either the action in the
     * "Bookmarks" menu has been triggered or the CTRL+B shortcut was pressed.
     *
     * @param checked       TRUE if the action is checked
     */
    void slotAddBookmark( bool checked );

    /**
     * This slot is called when an action from the "Shares View" menu was triggered.
     * It changes the view mode of the shares view part.
     *
     * @param action        The triggered action
     */
    void slotViewModeTriggered( QAction *action );

    /**
     * This slot is connected to the Smb4KWalletManager::initialized() signal.
     * It checks the state of the wallet manager and sets the icon in the status
     * bar accordingly.
     */
    void slotWalletManagerInitialized();

    /**
     * Reimplemented from KParts::MainWindow.
     */
    void slotSetStatusBarText( const QString &text );

    /**
     * This slot sets up the visual mount feedback in the status bar. It is
     * connected to the Smb4KMounter::finished() signal.
     *
     * @param share         The Smb4KShare object
     *
     * @param process       The process
     */
    void slotVisualMountFeedback( Smb4KShare *share,
                                  int process );

    /**
     * This slot shows a message according the to actions performed by the print
     * manager. It is connected to the Smb4KPrint::aboutToStart() signal.
     *
     * @param info          The Smb4KPrintInfo object
     */
    void slotPrintStartMessages( Smb4KPrintInfo *info );

    /**
     * This slot shows a message according to the finished actions that were reported
     * by the print manager. It is connected to the Smb4KPrint::finished() signal.
     *
     * @param info          The Smb4KPrintInfo object
     */
    void slotPrintFinishMessages( Smb4KPrintInfo *info );

    /**
     * This slot shows a message according to the action performed by the synchronizer.
     * It is connected to the Smb4KSynchronizer::aboutToStart() signal.
     *
     * @param info          The Smb4KSynchronizationInfo object
     */
    void slotSynchronizerStartMessages( Smb4KSynchronizationInfo *info );

    /**
     * This slot shows a message according to the finished action that were reported
     * by the synchronizer. It is connected to the Smb4KSynchronizer::finished() signal.
     *
     * @param info          The Smb4KSynchronizationInfo object
     */
    void slotSynchronizerFinishMessages( Smb4KSynchronizationInfo *info );

    /**
     * This slot shows a message according to the action performed by the previewer.
     * It is connected to the Smb4KPreviewer::aboutToStart() signal.
     *
     * @param info          The Smb4KPreviewItem item
     */
    void slotPreviewerStartMessages( Smb4KPreviewItem *item );

    /**
     * This slot shows a message according to the finished action that was reported
     * by the previewer. It is connected to the Smb4KPreviewer::finished() signal.
     *
     * @param item          The Smb4KPreviewItem item
     */
    void slotPreviewerFinishMessages( Smb4KPreviewItem *item );

    /**
     * This slot hides the feedback icon in the status bar. It is connected to
     * a QTimer::singleShot() signal.
     */
    void slotEndVisualFeedback();

  private:
    /**
     * Set up the main window actions
     */
    void setupActions();

    /**
     * Set up the status bar
     */
    void setupStatusBar();

    /**
     * Set up the main window's view
     */
    void setupView();

    /**
     * Set up the system tray widget
     */
    void setupSystemTrayWidget();

    /**
     * Set up the bookmark menu
     */
    void setupBookmarksMenu();

    /**
     * Loads the settings
     */
    void loadSettings();

    /**
     * Saves the settings
     */
    void saveSettings();

    /**
     * This is the progress bar in the status bar.
     */
    QProgressBar *m_progress_bar;

    /**
     * The id of the timer that set the values of the
     * progress bar.
     */
    int m_timer_id;

    /**
     * This is the pixmap label that represents the state of
     * the password handler in the status bar.
     */
    QLabel *m_pass_icon;

    /**
     * This icon gives feedback on actions that took place like
     * mounting a share, etc.
     */
    QLabel *m_feedback_icon;

    /**
     * The system tray widget
     */
    Smb4KSystemTray *m_system_tray_widget;

    /**
     * The network browser part
     */
    KParts::Part *m_browser_part;

    /**
     * The search dialog part
     */
    KParts::Part *m_search_part;

    /**
     * The shares view
     */
    KParts::Part *m_shares_part;

    /**
     * Bookmarks action group
     */
    QActionGroup *m_bookmarks;

    /**
     * Dock widgets action group;
     */
    QActionGroup *m_dock_widgets;
};

#endif
