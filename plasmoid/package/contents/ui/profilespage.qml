/***************************************************************************
    profilespage.qml - The profiles page of Smb4K's plasmoid
                             -------------------
    begin                : So Nov 11 2014
    copyright            : (C) 2014 by Alexander Reinholdt
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
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaComponents.Page {
  id: profilesPage

//   //
//   // The tool bar
//   //
//   PlasmaComponents.ToolBar {
//     id: profilesToolBar
//     anchors {
//       top: parent.top
//       left: parent.left
//       right: parent.right
//       topMargin: 2
//       rightMargin: 4
//       leftMargin: 4
//     }
//     PlasmaComponents.ToolBarLayout {
//       id: profilesToolBarLayout
//       spacing: 2
//       PlasmaComponents.ToolButton {
//         id: editButton
//         text: i18n("Edit")
//         iconSource: "document-edit"
//         width: minimumWidth
//         onClicked: {
//           // FIXME
//         }
//       }
//       Item {
//         id: spacer
//       }
//     }
//         
//     tools: profilesToolBarLayout
//   }
  
  //
  // The list view
  //
  PlasmaExtras.ScrollArea {
    id: profilesPageScrollArea
    anchors {
      top: parent.top // Change this if we are using a toolbar
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: 5
    }
    ListView {
      id: profilesView
      delegate: ProfileItemDelegate {
        id: profileItemDelegate
        onItemClicked: {
          iface.activeProfile = object.profileName
        }
      }
      model: ListModel {}
      focus: true
//       highlight: PlasmaComponents.Highlight {}
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onProfilesListChanged: fillView()
    onActiveProfileChanged: fillView()
  }
  
  //
  // Do initial things
  //
  Component.onCompleted: {
    fillView()
  }
  
  //
  // Fill the view
  //
  function fillView() {
    // First, clear the view.
    while (profilesView.model.count != 0) {
      profilesView.model.remove(0)
    }
    
    // Now fill the view if the list of profiles is not empty
    // and the use of profiles is enabled.
    if (iface.profileUsage && iface.profiles.length != 0) {
      for (var i = 0; i < iface.profiles.length; i++) {
        profilesView.model.append( {"object": iface.profiles[i]} )
      }
    }
    else {
      // Do nothing
    }
  }
}

