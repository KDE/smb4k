/***************************************************************************
    sharesviewitemdelegate.qml - The item delegate for the shares view
    in Smb4K's plasmoid
                             -------------------
    begin                : Do Apr 12 2012
    copyright            : (C) 2012 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1


//
// Delegate for the items in the shares view
//
Item {
  id: delegate
      
  signal unmountClicked()
  signal openClicked()
      
  width: sharesView.cellWidth
  height: sharesView.cellHeight
  
  anchors.verticalCenter: parent.verticalCenter

  Column {
    anchors.horizontalCenter: parent.horizontalCenter
    Row {
      anchors.horizontalCenter: parent.horizontalCenter
      spacing: 10
      QIconItem {
        icon: itemIcon
        width: theme.largeIconSize
        height: theme.largeIconSize
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.openClicked()
          }
        }
      }
      QIconItem {
        id: unmountButton
        icon: "media-eject"
        width: theme.smallIconSize
        height: theme.smallIconSize
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
    Text {
      text: itemName
      anchors.horizontalCenter: parent.horizontalCenter
      MouseArea {
        anchors.fill: parent
        onClicked: {
          delegate.openClicked()
        }
      }
    }
    Text {
      text: i18n( "<font size=\"-1\">on %1</font>" ).arg( itemHost )
      anchors.horizontalCenter: parent.horizontalCenter
      MouseArea {
        anchors.fill: parent
        onClicked: {
          delegate.openClicked()
        }
      }
    }
  }
}
