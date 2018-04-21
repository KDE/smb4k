/***************************************************************************
 *   Copyright (C) 2018 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras


PlasmaComponents.CommonDialog {
  titleText: i18n("Edit Bookmarks")
  buttonTexts: {[i18n("Ok"), i18n("Cancel")]}
  visualParent: parent
  
  content: ColumnLayout {
    //
    // Tool bar
    //
    PlasmaComponents.ToolBar {
      id: editorToolBar
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }
      
      tools: PlasmaComponents.ToolBarLayout {
        PlasmaComponents.ToolButton {
          id: addGroupButton
          tooltip: i18n("Add Group")
          iconSource: "bookmark-add-folder"
          width: minimumWidth
          onClicked: {
            // FIXME
          }
        }
        PlasmaComponents.ToolButton {
          id: clearButton
          tooltip: i18n("Clear")
          iconSource: "edit-clear"
          width: minimumWidth
          onClicked: {
            // FIXME
          }
        }
      }
    }
    
    //
    // List view
    //
    PlasmaExtras.ScrollArea {
      id: bookmarksScrollArea
      
      anchors {
        top: editorToolBar.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
      
      ListView {
        id: bookmarksListView
//         model: editorItemDelegateModel
        clip: true
        focus: true
        highlightRangeMode: ListView.StrictlyEnforceRange
      }
    }
  }

  onButtonClicked: {
    switch(index) {
      case 0:
        console.log("Ok")
        break
      case 1:
        console.log("Cancel")
        break
      default:
        break
    }
  }
}
