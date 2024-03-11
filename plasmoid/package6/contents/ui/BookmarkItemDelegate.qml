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

RowLayout {
  id: delegate
  
  signal itemClicked()
  
  width: parent.width
  implicitWidth: parent.implicitWidth
  height: Math.max(delegateItemIcon.paintedHeight + Kirigami.Units.smallSpacing, delegateItemText.height + Kirigami.Units.smallSpacing)
  implicitHeight: Math.max(delegateItemIcon.paintedHeight + Kirigami.Units.smallSpacing, delegateItemText.height + Kirigami.Units.smallSpacing)
  focus: true
  enabled: !object.isMounted
  
  MouseArea {
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    onClicked: {
      delegate.itemClicked()
    }
  
    Row {
      spacing: Kirigami.Units.largeSpacing

      Kirigami.Icon {
        id: delegateItemIcon

        anchors.verticalCenter: parent.verticalCenter

        source: (object.isCategory ? "folder-bookmark" : "folder-network-symbolic")
        width: Kirigami.Units.iconSizes.medium
        height: Kirigami.Units.iconSizes.medium
        enabled: delegate.enabled
      }

      PlasmaComponents.Label {
        id: delegateItemText

        anchors.verticalCenter: parent.verticalCenter

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

