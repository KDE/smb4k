/*
    SPDX-FileCopyrightText: 2019-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.components as PlasmaComponents

PlasmaComponents.Page {
  id: configurationPage

  ColumnLayout {
    anchors.fill: parent

    PlasmaComponents.Button {
      id: openConfigDialogButton
      Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
      text: i18n("Configuration Dialog")
      icon.name: "configure"
    
      onClicked: {
        iface.openConfigurationDialog()
      }
    }
  }
}
