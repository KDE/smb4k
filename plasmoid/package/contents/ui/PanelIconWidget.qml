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


MouseArea {
  id: panelIconWidget
  anchors.fill: parent
  
  PlasmaCore.IconItem {
    id: panelIcon
    anchors.fill: parent
    source: "smb4k"
    colorGroup: PlasmaCore.ColorScope.colorGroup
    
    //
    // Busy indicator
    //
    PlasmaComponents.BusyIndicator {
      id: busyIndicator
      running: false
      visible: false
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter
    }
  }
  
  onClicked: {
    plasmoid.expanded = !plasmoid.expanded
    
    if (!plasmoid.expanded && busyIndicator.running) {
      busyIndicator.visible = false
      busyIndicator.running = false
    }
    else {
      // Do nothing
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onBusy: busy()
    onIdle: idle()
  }
  
  //
  // Functions
  //
  function busy() {
    if (!plasmoid.expanded) {
      busyIndicator.visible = true
      busyIndicator.running = true
    }
    else {
      // Do nothing
    }
  }
  
  function idle() {
    if (!plasmoid.expanded) {
      busyIndicator.visible = false
      busyIndicator.running = false
    }
    else {
      // Do nothing
    }
  }
}
