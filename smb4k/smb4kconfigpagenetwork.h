/*
    The configuration page for the network settings of Smb4K

    SPDX-FileCopyrightText: 2003-2021 Alexander Reinholdt <alexander.reinholdt@kdemail.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SMB4KCONFIGPAGENETWORK_H
#define SMB4KCONFIGPAGENETWORK_H

// Qt includes
#include <QTabWidget>

/**
 * This is the configuration tab for the network settings
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KConfigPageNetwork : public QTabWidget
{
    Q_OBJECT

public:
    /**
     * The constructor
     *
     * @param parent        The parent widget
     */
    explicit Smb4KConfigPageNetwork(QWidget *parent = 0);

    /**
     * The destructor
     */
    ~Smb4KConfigPageNetwork();

protected Q_SLOTS:
    /**
     * This slot is called when the button for setting the SMB protocol
     * versions is toggled.
     */
    void slotSetProtocolVersionsToggled(bool on);

    /**
     * This slot is called when the button for setting the Wake-On-LAN
     * feature is toggled.
     */
    void slotEnableWakeOnLanFeatureToggled(bool on);
};
#endif
