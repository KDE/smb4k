/*
    smb4ksystemtray  -  This is the system tray window class of Smb4K.

    SPDX-FileCopyrightText: 2007-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KSYSTEMTRAY_H
#define SMB4KSYSTEMTRAY_H

// application specific includes
#include "smb4kmountdialog.h"

// Qt includes
#include <QString>
#include <QWidget>

// KDE includes
#include <KStatusNotifierItem>

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

public Q_SLOTS:
    /**
     * This function (re-)loads the settings for this widget.
     */
    void loadSettings();

protected Q_SLOTS:
    /**
     * This slot opens the manual mount dialog.
     *
     * @param checked         TRUE if the action can be and is checked and FALSE
     *                        otherwise.
     */
    void slotMountDialog();

    /**
     * Set the status of the system tray icon. This slot checks the global
     * list of mounted shares and the global list of workgroups. If neither of
     * them contains any item, the icon is set to passive state until one of
     * the lists is populated.
     */
    void slotSetStatus();
};

#endif
