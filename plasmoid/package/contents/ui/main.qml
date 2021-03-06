/***************************************************************************
 *   Copyright (C) 2017 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin 2.0

Item {
  id: root
  
  Plasmoid.toolTipMainText: i18n("Network Neighborhood")
//   Plasmoid.toolTipSubText: sinkModel.preferredSink ? i18n("Volume at %1%\n%2", volumePercent(sinkModel.preferredSink.volume), sinkModel.preferredSink.description) : ""
//   Plasmoid.icon: "smb4k"
  Plasmoid.switchWidth: units.gridUnit * 10
  Plasmoid.switchHeight: units.gridUnit * 10
  
  //
  // Smb4K interface
  //
  Interface {
    id: iface
  }
  
  //
  // Plasmoid representations
  //
  Plasmoid.compactRepresentation: PanelIconWidget {} // FIXME: Look at plasma-nm how this can be done
  Plasmoid.fullRepresentation: PopupDialog {
    id: main
    Layout.minimumWidth: units.iconSizes.medium * 10
    Layout.minimumHeight: units.gridUnit * 20
    anchors.fill: parent
    focus: true
  }
  
  //
  // Start interface
  //
  Component.onCompleted: {
    iface.startClient();
    iface.startMounter();
  }
}
