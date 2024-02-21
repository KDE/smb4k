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
          text: object.name+(object.comment.length != 0 ? "<br><font size=\"-1\">"+object.comment+"</font>" : "")
          color: ((object.type == NetworkObject.Host && object.isMasterBrowser) ? "darkblue" : theme.textColor)
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
    
    ToolButton {
      id: previewButton
      icon.name: "preview"
      text: i18n("Preview")
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
    
    ToolButton {
      id: configureButton
      icon.name: "settings-configure"
      text: i18n("Configure")
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
