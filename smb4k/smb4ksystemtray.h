/*
    smb4ksystemtray  -  This is the system tray window class of Smb4K.

    SPDX-FileCopyrightText: 2007-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSYSTEMTRAY_H
#define SMB4KSYSTEMTRAY_H

// Qt includes
#include <QString>
#include <QWidget>

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
    explicit Smb4KSystemTray(QWidget *parent = nullptr);

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
     * This signal is forwarded from @see Smb4KConfigDialog.
     */
    void settingsChanged(const QString &dialogName);

protected slots:
    /**
     * This slot opens the manual mount dialog.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotMountDialog();

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
