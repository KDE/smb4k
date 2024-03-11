/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin
import org.kde.kirigami as Kirigami

RowLayout {
  id: delegate
  
  signal itemClicked()
  signal bookmarkClicked()
  signal unmountClicked()
  signal syncClicked()
  
  width: parent.width
  implicitWidth: parent.implicitWidth
  height: Math.max(delegateItemIcon.paintedHeight + Kirigami.Units.smallSpacing, delegateItemText.height + Kirigami.Units.smallSpacing)
  implicitHeight: Math.max(delegateItemIcon.paintedHeight + Kirigami.Units.smallSpacing, delegateItemText.height + Kirigami.Units.smallSpacing)
  focus: true
  
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

        source: !object.isInaccessible ? "folder-network" : "folder-locked"
        width: Kirigami.Units.iconSizes.medium
        height: Kirigami.Units.iconSizes.medium
      }

      PlasmaComponents.Label {
        id: delegateItemText

        anchors.verticalCenter: parent.verticalCenter

        elide: Text.ElideRight
        text: object.shareName+"<br>"+i18n("<font size=\"-1\">on %1</font>", object.hostName)
      }
    }
  }
  
  Row {
    Layout.alignment: Qt.AlignRight

    spacing: 0
    
    PlasmaComponents.ToolButton {
      id: bookmarkButton

      hoverEnabled: true
      icon.name: "favorite"
      flat: true
      opacity: 0.2

      PlasmaComponents.ToolTip.delay: 1000
      PlasmaComponents.ToolTip.timeout: 5000
      PlasmaComponents.ToolTip.text: i18n("Bookmark")
      PlasmaComponents.ToolTip.visible: hovered

      MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
          parent.opacity = 1.0
        }
        onExited: {
          parent.opacity = 0.2
        }
        onClicked: {
          delegate.bookmarkClicked()
        }
      }      
    }

    PlasmaComponents.ToolButton {
      id: syncButton

      hoverEnabled: true
      icon.name: "folder-sync"
      flat: true
      opacity: 0.2

      PlasmaComponents.ToolTip.delay: 1000
      PlasmaComponents.ToolTip.timeout: 5000
      PlasmaComponents.ToolTip.text: i18n("Synchronize")
      PlasmaComponents.ToolTip.visible: hovered

      MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
          parent.opacity = 1.0
        }
        onExited: {
          parent.opacity = 0.2
        }
        onClicked: {
          delegate.syncClicked()
        }
      }  
    }

    PlasmaComponents.ToolButton {
      id: unmountButton

      hoverEnabled: true
      icon.name: "media-eject"
      flat: true
      opacity: 0.2

      PlasmaComponents.ToolTip.delay: 1000
      PlasmaComponents.ToolTip.timeout: 5000
      PlasmaComponents.ToolTip.text: i18n("Unmount")
      PlasmaComponents.ToolTip.visible: hovered

      MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
          parent.opacity = 1.0
        }
        onExited: {
          parent.opacity = 0.2
        }
        onClicked: {
          delegate.unmountClicked()
        }
      }      
    }
  }
}

