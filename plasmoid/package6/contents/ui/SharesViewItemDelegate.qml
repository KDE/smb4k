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

Item {
  id: delegate
  
  signal itemClicked()
  signal bookmarkClicked()
  signal unmountClicked()
  signal syncClicked()
  
  width: parent.width
  implicitWidth: parent.implicitWidth
  // FIXME: Use something like margin instead of the 3 * units.smallSpacing that was found
  // by trial and error ...
  height: Math.max(delegateItemIcon.paintedHeight + 3 * units.smallSpacing, delegateItemText.height + 3 * units.smallSpacing) 
  implicitHeight: Math.max(delegateItemIcon.paintedHeight + 3 * units.smallSpacing, delegateItemText.height + 3 * units.smallSpacing) 
  focus: true
  
  MouseArea {
    anchors.fill: parent
    
    onClicked: {
      delegate.itemClicked()
    }
    
    Row {
      spacing: units.largeSpacing
      Column {
        anchors.verticalCenter: parent.verticalCenter
        Kirigami.Icon {
          id: delegateItemIcon
          source: object.icon
          width: units.iconSizes.medium
          height: units.iconSizes.medium
        }
      }
      Column {
        anchors.verticalCenter: parent.verticalCenter
        PlasmaComponents.Label {
          id: delegateItemText
          elide: Text.ElideRight
          text: object.shareName+"<br>"+i18n("<font size=\"-1\">on %1</font>", object.hostName)
        }
      }
    }
  }
  
  RowLayout {
    anchors {
     verticalCenter: parent.verticalCenter
      right: parent.right
    }
    spacing: 0
    
    ToolButton {
      id: bookmarkButton
      icon.name: "favorite"
      text: i18n("Bookmark")
      flat: true
      opacity: 0.2
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
    
    ToolButton {
      id: syncButton
      icon.name: "folder-sync"
      text: i18n("Synchronize")
      flat: true
      opacity: 0.2
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
    
    ToolButton {
      id: unmountButton
      icon.name: "media-eject"
      text: i18n("Unmount")
      flat: true
      opacity: 0.2
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

