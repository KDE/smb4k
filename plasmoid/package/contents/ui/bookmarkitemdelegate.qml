/***************************************************************************
    bookmarkspage.qml - The bookmarks page of Smb4K's plasmoid
                             -------------------
    begin                : So Mai 11 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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
// Delegate for the items in the bookmarks list
//
PlasmaComponents.ListItem {
  id: delegate
  
  signal itemClicked()
  signal removeClicked()

  width: bookmarksListView.width
  height: theme.mediumIconSize + 8
  
  Row {
    spacing: 10
    Column {
      anchors.verticalCenter: parent.verticalCenter
      QIconItem {
        id: delegateItemIcon
        icon: itemIcon
        width: theme.mediumIconSize
        height: theme.mediumIconSize
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
        text: itemDescription
        clip: true
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
        }
      }
    }
  }
  QIconItem {
    id: removeButton
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.leftMargin: 10
    icon: "edit-delete"
    height: theme.smallIconSize
    width: theme.smallIconSize
    opacity: 0.2
    visible: !itemIsGroup
    enabled: !itemIsGroup
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
