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
  signal bookmarkClicked()
  signal previewClicked()
  signal configureClicked()
  
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
        
        PlasmaCore.IconItem {
          id: delegateItemIcon
          source: {
            switch (object.type) {
              case NetworkObject.Workgroup:
                "network-workgroup-symbolic"
                break
              case NetworkObject.Host:
                "network-server-symbolic"
                break
              case NetworkObject.Share:
                "folder-network-symbolic"
                break
              default:
                ""
                break
            }
          }
          overlays: [
            (object.isMounted ? "emblem-mounted" : "")
          ]
          width: units.iconSizes.medium
          height: units.iconSizes.medium
        }
      }
      Column {
        anchors.verticalCenter: parent.verticalCenter
        
        PlasmaComponents.Label {
          id: delegateItemText
          elide: Text.ElideRight
          text: object.name+(object.comment.length != 0 ? "<br><font size=\"-1\">"+object.comment+"</font>" : "")
          color: ((object.type == NetworkObject.Host && object.isMasterBrowser) ? "darkblue" : theme.textColor)
        }
      }
    }
  }
  
  PlasmaComponents.ButtonRow {
    anchors {
      verticalCenter: parent.verticalCenter
      right: parent.right
    }
    exclusive: false
    spacing: 0
    
    PlasmaComponents.ToolButton {
      id: bookmarkButton
      iconSource: "favorite"
      tooltip: i18n("Bookmark")
      flat: true
      opacity: 0.2
      visible: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
      enabled: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
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
      id: previewButton
      iconSource: "preview"
      flat: true
      opacity: 0.2
      visible: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
      enabled: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
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
          delegate.previewClicked()
        }      
      }
    }
    
    PlasmaComponents.ToolButton {
      id: configureButton
      iconSource: "settings-configure"
      flat: true
      opacity: 0.2
      visible: (object.type != NetworkObject.Network && object.type != NetworkObject.Workgroup) ? true : false
      enabled: (object.type != NetworkObject.Network && object.type != NetworkObject.Workgroup) ? true : false
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
          delegate.configureClicked()
        }      
      }
    }
  }
}
