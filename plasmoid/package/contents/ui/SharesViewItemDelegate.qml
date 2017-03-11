/***************************************************************************
 *   Copyright (C) 2017 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

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
  signal unmountClicked()
  signal syncClicked()
  
  width: parent.width
  height: theme.mediumIconSize + 8
  focus: true
  
  Row {
    spacing: 10
    Column {
      anchors.verticalCenter: parent.verticalCenter
      PlasmaCore.IconItem {
        id: delegateItemIcon
        source: object.icon
        width: theme.mediumIconSize
        height: theme.mediumIconSice
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
        }
      }
    }
    Column {
      anchors.verticalCenter: parent.verticalCenter
      PlasmaComponents.Label {
        id: delegateItemText
        elide: Text.ElideRight
        text: object.shareName+"<br>"+i18n("<font size=\"-1\">on %1</font>", object.hostName)
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
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
    spacing: 1
    
    PlasmaComponents.ToolButton {
      id: bookmarkButton
      iconSource: "favorite"
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
    
    PlasmaComponents.ToolButton {
      id: syncButton
      iconSource: "folder-sync"
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
    
    PlasmaComponents.ToolButton {
      id: unmountButton
      iconSource: "emblem-unmounted"
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

