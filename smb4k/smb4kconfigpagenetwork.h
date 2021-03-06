/***************************************************************************
    The configuration page for the network settings of Smb4K
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2021 by Alexander Reinholdt
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
