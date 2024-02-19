/*
    SPDX-FileCopyrightText: 2017-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0

PlasmaComponents.ListItem {
  id: delegate
  
  signal itemClicked()
  
  width: parent.width
  implicitWidth: parent.implicitWidth
  // FIXME: Use something like margin instead of the 3 * units.smallSpacing that was found
  // by trial and error ...
  height: Math.max(delegateItemIcon.paintedHeight + 3 * units.smallSpacing, delegateItemText.height + 3 * units.smallSpacing) 
  implicitHeight: Math.max(delegateItemIcon.paintedHeight + 3 * units.smallSpacing, delegateItemText.height + 3 * units.smallSpacing) 
  focus: true
  enabled: !object.isMounted
  
  MouseArea {
    anchors.fill: parent
    
    onClicked: {
      delegate.itemClicked()
    }
  
    Row {
      spacing: units.largeSpacing
      Column {
        anchors.verticalCenter: parent.verticalCenter
        PlasmaCore.IconItem {
          id: delegateItemIcon
          source: (object.isCategory ? "folder-bookmark" : "folder-network-symbolic")
          overlays: [
            (object.isMounted ? "emblem-mounted" : "")
          ]
          width: units.iconSizes.medium
          height: units.iconSizes.medium
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

