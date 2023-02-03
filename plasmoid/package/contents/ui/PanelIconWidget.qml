/*
    SPDX-FileCopyrightText: 2017 A. Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    function onBusy() { busy() }
    function onIdle() { idle() }
  }
  
  //
  // Functions
  //
  function busy() {
    if (!plasmoid.expanded) {
      busyIndicator.visible = true
      busyIndicator.running = true
    }
  }
  
  function idle() {
    if (!plasmoid.expanded) {
      busyIndicator.visible = false
      busyIndicator.running = false
    }
  }
}
