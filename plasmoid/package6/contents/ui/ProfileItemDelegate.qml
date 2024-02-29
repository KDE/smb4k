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

Item {
  id: delegate
  
  signal itemClicked()
  
  width: parent.width
  implicitWidth: parent.implicitWidth
  // FIXME: Use something like margin instead of the 3 * Kirigami.Units.smallSpacing that was found
  // by trial and error ...
  height: Math.max(delegateItemIcon.paintedHeight + 3 * Kirigami.Units.smallSpacing, delegateItemText.height + 3 * Kirigami.Units.smallSpacing)
  implicitHeight: Math.max(delegateItemIcon.paintedHeight + 3 * Kirigami.Units.smallSpacing, delegateItemText.height + 3 * Kirigami.Units.smallSpacing)
  focus: true
  
  MouseArea {
    anchors.fill: parent
    
    onClicked: {
      delegate.itemClicked()
    }
  
    Row {
      spacing: Kirigami.Units.largeSpacing
      Column {
        anchors.verticalCenter: parent.verticalCenter
        Kirigami.Icon {
          id: delegateItemIcon
          source: "format-list-unordered"
          width: Kirigami.Units.iconSizes.medium
          height: Kirigami.Units.iconSizes.medium
        }
      }
      Column {
        anchors.verticalCenter: parent.verticalCenter
        PlasmaComponents.Label {
          id: delegateItemText
          elide: Text.ElideRight
          text: {
            object.profileName+(object.isActiveProfile ? " "+i18n("(active)") : "")
          }
        }
      }
    }
  }
}

