/***************************************************************************
    smb4ksystemtray  -  This is the system tray window class of Smb4K.
                             -------------------
    begin                : Mi Jun 13 2007
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

#ifndef SMB4KSYSTEMTRAY_H
#define SMB4KSYSTEMTRAY_H

// Qt includes
#include <QtCore/QString>
#include <QtWidgets/QWidget>

// KDE includes
#include <KNotifications/KStatusNotifierItem>

class Smb4KSystemTray : public KStatusNotifierItem
{
  Q_OBJECT

  friend class Smb4KMainWindow;

  public:
    /**
     * The constructor.
     *
     * @param parent        The parent widget of the system tray window
     */
    explicit Smb4KSystemTray(QWidget *parent = 0);

    /**
     * The destructor.
     */
    ~Smb4KSystemTray();

    /**
     * This function (re-)loads the settings for this widget.
     */
    void loadSettings();

  signals:
    /**
     * This signal is emitted when the config dialog has been closed and the
     * settings changed.
     *
     * This signal is forwared from @see Smb4KConfigDialog.
     */
    void settingsChanged(const QString &dialogName);

  protected slots:
    /**
     * This slot opens the manual mount dialog.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotMountDialog(bool checked);

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
    void slotSettingsChanged(const QString &dialogName);

    /**
     * Set the status of the system tray icon. This slot checks the global
     * list of mounted shares and the global list of workgroups. If neither of
     * them contains any item, the icon is set to passive state until one of 
     * the lists is populated.
     */
    void slotSetStatus();
};

#endif
