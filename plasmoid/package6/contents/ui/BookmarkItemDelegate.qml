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
  enabled: !object.isMounted
  
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
          source: (object.isCategory ? "folder-bookmark" : "folder-network-symbolic")
          width: Kirigami.Units.iconSizes.medium
          height: Kirigami.Units.iconSizes.medium
          enabled: delegate.enabled
        }
      }
      Column {
        anchors.verticalCenter: parent.verticalCenter
        PlasmaComponents.Label {
          id: delegateItemText
          elide: Text.ElideRight
          text: {
            if (!object.isCategory) {
              object.shareName+(object.label.length != 0 ? " ("+object.label+")" : "")+
              "<br>"+i18n("<font size=\"-1\">on %1</font>", object.hostName)
            }
            else {
              object.categoryName
            }
          }
          enabled: delegate.enabled
        }
      }
    }
  }
}

