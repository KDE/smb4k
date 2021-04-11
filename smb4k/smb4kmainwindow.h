/***************************************************************************
    The main window of Smb4K
                             -------------------
    begin                : Di Jan 1 2008
    copyright            : (C) 2008-2021 by Alexander Reinholdt
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

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QActionGroup>
#include <QDockWidget>
#include <QLabel>
#include <QProgressBar>
#include <QUrl>

// KDE includes
#include <KXmlGui/KXmlGuiWindow>

// forward declarations
class Smb4KSystemTray;
class Smb4KPrintInfo;
class Smb4KSynchronizationInfo;

/**
 * This is the main window of Smb4K. It provides the network browser, the
 * shares view and all other dialogs as well as a menu, status and tool bar.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KMainWindow : public KXmlGuiWindow
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
    bool queryClose() override;

    /**
     * Reimplemented from KMainWindow
     */
    bool eventFilter(QObject *obj, QEvent *e) override;

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
    void slotSettingsChanged(const QString &dialogName);

    /**
     * This slot is called when a bookmark should be added.
     *
     * @param checked       TRUE if the action is checked
     */
    void slotAddBookmarks();

    /**
     * This slot is connected to the Smb4KWalletManager::initialized() signal.
     * It checks the state of the wallet manager and sets the icon in the status
     * bar accordingly.
     */
    void slotWalletManagerInitialized();

    /**
     * This slot shows a busy bar and a status message according to the action performed by
     * the client. It is connected to the Smb4KClient::aboutToStart() signal.
     *
     * @param item          The network item
     *
     * @param process       The process
     */
    void slotClientAboutToStart(const NetworkItemPtr &item, int process);

    /**
     * this slot shows a status message according to the action that was just finished by the
     * client and hides the status bar if appropriate. It is connected to the Smb4KClient::finished()
     * signal.
     *
     * @param item          The network item
     *
     * @param process       The process
     */
    void slotClientFinished(const NetworkItemPtr &item, int process);

    /**
     * This slot shows a status message according to the action performed by the
     * mounter as well as a busy bar. It is connected to the Smb4KMounter::aboutToStart()
     * signal.
     * @param process       The process
     */
    void slotMounterAboutToStart(int process);

    /**
     * This shows a status message according to the action that was just finished by
     * the mounter and hides the busy bar if appropriate. It is connected to the
     * Smb4KMounter::finished() signal.
     * @param process       The process
     */
    void slotMounterFinished(int process);

    /**
     * This slot gives the visual mount feedback in the status bar. It is
     * connected to the Smb4KMounter::mounted() signal.
     *
     * @param share         The share object
     */
    void slotVisualMountFeedback(const SharePtr &share);

    /**
     * This slot gives the visual unmount feedback in the status bar. It is
     * connected to the Smb4KMounter::unmounted() signal.
     *
     * @param share         The share object
     */
    void slotVisualUnmountFeedback(const SharePtr &share);

    /**
     * This slot shows a message according to the action performed by the synchronizer.
     * It is connected to the Smb4KSynchronizer::aboutToStart() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerAboutToStart(const QString &dest);

    /**
     * This slot shows a message according to the finished action that were reported
     * by the synchronizer. It is connected to the Smb4KSynchronizer::finished() signal.
     *
     * @param dest          The path of the destination
     */
    void slotSynchronizerFinished(const QString &dest);

    /**
     * This slot hides the feedback icon in the status bar. It is connected to
     * a QTimer::singleShot() signal.
     */
    void slotEndVisualFeedback();

    /**
     * Enable/disable the "Add Bookmark" action
     */
    void slotEnableBookmarkAction();

    /**
     * This slot is connected to the visibilityChanged() signals of the network browser
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotNetworkBrowserVisibilityChanged(bool visible);

    /**
     * This slot is connected to the visibilityChanged() signals of the shares view
     * dock widget. It is used to get the tool bars right.
     *
     * @param visible         If the dock widget is visible.
     */
    void slotSharesViewVisibilityChanged(bool visible);

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
     * Setup the dynamic action list
     */
    void setupDynamicActionList(QDockWidget *dock);

    /**
     * This is the progress bar in the status bar.
     */
    QProgressBar *m_progress_bar;

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
     * This is the widget (embedded into a dock widget) that has
     * the focus.
     */
    QWidget *m_focusWidget;

    /**
     * Dock widgets action group;
     */
    QActionGroup *m_dockWidgets;
};

#endif
