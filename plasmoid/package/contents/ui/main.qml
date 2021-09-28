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
