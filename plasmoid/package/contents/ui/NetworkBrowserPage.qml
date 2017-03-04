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

PlasmaComponents.Page {
  id: networkBrowserPage
  anchors.fill: parent
  
  //
  // Tool bar
  //
  
  // FIXME: The toolbar basically looks ugly at the moment ...
  
  PlasmaComponents.ToolBar {
    id: networkBrowserToolBar
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: rescanButton
        tooltip: i18n("Rescan")
        iconSource: "view-refresh"
        width: minimumWidth
        onClicked: {} // FIXME
      }
      PlasmaComponents.ToolButton {
        id: abortButton
        tooltip: i18n("Abort")
        iconSource: "process-stop"
        width:minimumWidth
        onClicked: {} // FIXME
      }
      PlasmaComponents.ToolButton {
        id: upButton
        tooltip: i18n("Go one level up")
        iconSource: "go-up-symbolic"
        width: minimumWidth
        onClicked: {} // FIXME
      }
      PlasmaComponents.ToolButton {
        id: mountDialogButton
        tooltip: i18n("Open the mount dialog")
        iconSource: "view-form"
        width: minimumWidth
        onClicked: {} // FIXME
      }
    }
  }
    
  //
  // List view
  //
  PlasmaExtras.ScrollArea {
    id: networkBrowserScrollArea
    anchors {
      top: networkBrowserToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    ListView {
      id: networkBrowserListView
      anchors.fill: parent
      delegate: NetworkBrowserItemDelegate {
        id: networkBrowserItemDelegate
        onItemClicked: {
          // FIXME
        }
      }
      model: ListModel {}
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
}
