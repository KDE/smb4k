/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin
import org.kde.kirigami as Kirigami


MouseArea {
  id: panelIconWidget
  anchors.fill: parent

  Kirigami.Icon {
    id: panelIcon
    anchors.fill: parent
    source: "smb4k"
    // colorGroup: PlasmaCore.ColorScope.colorGroup
    
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
    root.expanded = !root.expanded
    
    if (!root.expanded && busyIndicator.running) {
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
