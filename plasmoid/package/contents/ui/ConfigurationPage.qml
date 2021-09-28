/*
    SPDX-FileCopyrightText: 2019 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Page {
  id: configurationPage

  ColumnLayout {
    anchors.fill: parent

    PlasmaComponents.Button {
      id: openConfigDialogButton
      Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
      text: i18n("Configuration Dialog")
      iconSource: "configure"
    
      onClicked: {
        iface.openConfigurationDialog()
      }
    }
  }
}
