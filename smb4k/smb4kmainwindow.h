/***************************************************************************
    smb4kmainwindow  -  The main window of Smb4K.
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2013 by Alexander Reinholdt
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

#ifndef SMB4KMAINWINDOW_H
#define SMB4KMAINWINDOW_H

// Qt includes
#include <QActionGroup>
#include <QLabel>
#include <QProgressBar>

// KDE includes
#include <kxmlguiwindow.h>
#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>

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
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
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
     * This slot is called when a bookmark should be added.
     *
     * @param checked       TRUE if the action is checked
     */
    void slotAddBookmark();

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
     * This slot shows a status message according to the action performed by the 
     * scanner as well as a busy bar. It is connected to the Smb4KScanner::aboutToStart() 
     * signal.
     *
     * @param item          The network item
     *
     * @param process       The process
     */
    void slotScannerAboutToStart( Smb4KBasicNetworkItem *item,
                                  int process );
                                  
    /**
     * This shows a status message according to the action that was just finished by
     * the scanner and hides the busy bar if appropriate. It is connected to the 
     * Smb4KScanner::finished() signal.
     * 
     * @param item          The network item
     * 
     * @param process       The process
     */
    void slotScannerFinished( Smb4KBasicNetworkItem *item,
                              int process );
                                   
    /**
     * This slot shows a status message according to the action performed by the 
     * mounter as well as a busy bar. It is connected to the Smb4KMounter::aboutToStart() 
     * signal.
     * 
     * @param share         The share item
     *
     * @param process       The process
     */
    void slotMounterAboutToStart( Smb4KShare *share,
                                  int process );
                                  
    /**
     * This slot gives the visual mount feedback in the status bar. It is 
     * connected to the Smb4KMounter::mounted() signal.
     * 
     * @param share         The Smb4KShare object
     */
    void slotVisualMountFeedback( Smb4KShare *share );
    
    /**
     * This slot gives the visual unmount feedback in the status bar. It is 
     * connected to the Smb4KMounter::unmounted() signal.
     * 
     * @param share         The Smb4KShare object
     */
    void slotVisualUnmountFeedback( Smb4KShare *share );
                                  
    /**
     * This shows a status message according to the action that was just finished by
     * the mounter and hides the busy bar if appropriate. It is connected to the 
     * Smb4KMounter::finished() signal.
     * 
     * @param share         The share item
     * 
     * @param process       The process
     */
    void slotMounterFinished( Smb4KShare *share,
                              int process );
                              
    /**
     * This slot shows a status message as well as a busy bar. It is connected to the 
     * Smb4KSearch::aboutToStart() signal.
     *
     * @param string        The search string
     */
    void slotSearchAboutToStart( const QString &string );
    
    /**
     * This shows a status message and hides the busy bar if appropriate. It is 
     * connected to the Smb4KSearch::finished() signal.
     * 
     * @param string        The search string
     */
   void slotSearchFinished( const QString &string );
     
    /**
     * This slot shows a status message as well as a busy bar. It is connected to the 
     * Smb4KPrint::aboutToStart() signal.
     *
     * @param printer       The remote printer
     */
    void slotPrintingAboutToStart( Smb4KShare *printer );

    /**
     * This shows a status message and hides the busy bar if appropriate. It is 
     * connected to the Smb4KPrint::finished() signal.
     *
     * @param printer       The remote printer
     */
    void slotPrintingFinished( Smb4KShare *printer );

    /**
     * This slot shows a message according to the action performed by the synchronizer.
     * It is connected to the Smb4KSynchronizer::aboutToStart() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerAboutToStart( const QString &dest );

    /**
     * This slot shows a message according to the finished action that were reported
     * by the synchronizer. It is connected to the Smb4KSynchronizer::finished() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerFinished( const QString &dest );

    /**
     * This slot shows a message according to the action performed by the previewer.
     * It is connected to the Smb4KPreviewer::aboutToStart() signal.
     *
     * @param share         The share
     *
     * @param url           The URL of the current location
     */
    void slotPreviewerAboutToStart( Smb4KShare *share,
                                    const KUrl &url );

    /**
     * This slot shows a message according to the finished action that was reported
     * by the previewer. It is connected to the Smb4KPreviewer::finished() signal.
     *
     * @param share         The share
     *
     * @param url           The URL of the current location
     */
    void slotPreviewerFinished( Smb4KShare *share,
                                const KUrl &url );

    /**
     * This slot hides the feedback icon in the status bar. It is connected to
     * a QTimer::singleShot() signal.
     */
    void slotEndVisualFeedback();

    /**
     * This slot is connected to the PartManager::activePartChanged() signal. It
     * modifies the states of actions that are defined in the main window but 
     * manage things in the parts.
     *
     * @param part          The now active KPart
     */
    void slotActivePartChanged( KParts::Part *part );

    /**
     * Enable/disable the "Add Bookmark" action
     */
    void slotEnableBookmarkAction();

    /**
     * Enable/disable the "Mount"/"Unmount" action
     */
    void slotEnableMountAction();

    /**
     * Called when the global mount action (in the toolbar) has been triggered
     */
    void slotMountActionTriggered();

    /**
     * Called when the mount dual action needs to change its active state.
     */
    void slotMountActionChanged( bool active );
    
    /**
     * Enable/disable the "Open With" action
     */
    void slotEnableOpenWithAction();
    
    /**
     * This slot is connected to the visibilityChanged() signals of the network browser
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotNetworkBrowserVisibilityChanged( bool visible );
    
    /**
     * This slot is connected to the visibilityChanged() signals of the shares view
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotSharesViewVisibilityChanged( bool visible );
    
    
    /**
     * This slot is connected to the visibilityChanged() signals of the network browser
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotSearchDialogVisibilityChanged( bool visible );
    
    

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
     * Set up menu bar
     */
    void setupMenuBar();

    /**
     * Set up the system tray widget
     */
    void setupSystemTrayWidget();

    /**
     * Loads the settings
     */
    void loadSettings();

    /**
     * Saves the settings
     */
    void saveSettings();
    
    /**
     * Set up the mount indicator
     */
    void setupMountIndicator();
    
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
     * The part manager
     */
    KParts::PartManager *m_manager;

    /**
     * The active part
     */
    KParts::Part *m_active_part;

    /**
     * Dock widgets action group;
     */
    QActionGroup *m_dock_widgets;
};

#endif
