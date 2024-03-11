/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.components as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin
import org.kde.kirigami as Kirigami

RowLayout {
  id:delegate

  signal itemClicked()
  signal bookmarkClicked()
  signal previewClicked()
  signal configureClicked()

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
        width: Kirigami.Units.iconSizes.medium
        height: Kirigami.Units.iconSizes.medium
      }

      PlasmaComponents.Label {
        id: delegateItemText

        anchors.verticalCenter: parent.verticalCenter

        elide: Text.ElideRight
        text: object.name+(object.comment.length != 0 ? "<br><font size=\"-1\">"+object.comment+"</font>" : "")
        color: (object.type == NetworkObject.Host && object.isMasterBrowser) ? "darkblue" : Kirigami.Theme.textColor
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
      visible: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
      enabled: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false

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
      id: previewButton

      hoverEnabled: true
      icon.name: "preview"
      flat: true
      opacity: 0.2
      visible: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false
      enabled: (object.type == NetworkObject.Share && !object.isPrinter) ? true : false

      PlasmaComponents.ToolTip.delay: 1000
      PlasmaComponents.ToolTip.timeout: 5000
      PlasmaComponents.ToolTip.text: i18n("Preview")
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
          delegate.previewClicked()
        }
      }
    }

    PlasmaComponents.ToolButton {
      id: configureButton

      hoverEnabled: true
      icon.name: "settings-configure"
      flat: true
      opacity: 0.2
      visible: (object.type != NetworkObject.Network && object.type != NetworkObject.Workgroup) ? true : false
      enabled: (object.type != NetworkObject.Network && object.type != NetworkObject.Workgroup) ? true : false

      PlasmaComponents.ToolTip.delay: 1000
      PlasmaComponents.ToolTip.timeout: 5000
      PlasmaComponents.ToolTip.text: i18n("Configure")
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
          delegate.configureClicked()
        }
      }
    }
  }
}
