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

PlasmoidItem {
  id: root
  
  toolTipMainText: i18n("Network Neighborhood")
//   Plasmoid.toolTipSubText: sinkModel.preferredSink ? i18n("Volume at %1%\n%2", volumePercent(sinkModel.preferredSink.volume), sinkModel.preferredSink.description) : ""
//   Plasmoid.icon: "smb4k"
  switchWidth: Kirigami.Units.gridUnit * 10
  switchHeight: Kirigami.Units.gridUnit * 10
  
  //
  // Smb4K interface
  //
  Interface {
    id: iface
  }
  
  //
  // Plasmoid representations
  //
  compactRepresentation: PanelIconWidget {} // FIXME: Look at plasma-nm how this can be done
  fullRepresentation: PopupDialog {
    id: main
    Layout.minimumWidth: Kirigami.Units.iconSizes.medium * 10
    Layout.minimumHeight: Kirigami.Units.gridUnit * 20
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
