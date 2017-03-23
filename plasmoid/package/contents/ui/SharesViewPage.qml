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
  id: sharesViewPage
  
  //
  // Tool bar
  //
  PlasmaComponents.ToolBar {
    id: sharesViewToolBar
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: unmountAllButton
        tooltip: i18n("Unmount all shares")
        iconSource: "system-run"
        width: minimumWidth
        onClicked: {
          iface.unmountAll()
        }
      }
    }
  }
  
  //
  // List view
  //
  PlasmaExtras.ScrollArea {
    id: sharesViewScrollArea
    
    anchors {
      top: sharesViewToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: sharesViewListView
      anchors.fill: parent
      clip: true
      
      delegate: SharesViewItemDelegate {
        id: sharesViewItemDelegate
        
        onItemClicked: {
          var object = sharesViewListView.model.get(index).object
          if (object !== null) {
            Qt.openUrlExternally(object.mountpoint)
          }
          else {
            // Do nothing
          }
        }
        
        onUnmountClicked: {
          var object = sharesViewListView.model.get(index).object
          if (object !== null) {
            iface.unmount(object)
          }
          else {
            // Do nothing
          }
        }
        
        onBookmarkClicked: {
          var object = sharesViewListView.model.get(index).object
          if (object !== null) {
            iface.addBookmark(object)
          }
          else {
            // Do nothing
          }
        }
        
        onSyncClicked: {
          var object = sharesViewListView.model.get(index).object
          if (object !== null) {
            iface.synchronize(object)
          }
          else {
            // Do nothing
          }
        }
      }      
      
      model: ListModel {}
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onMountedSharesChanged: shareMountedOrUnmounted()
  }
  
  //
  // Functions
  //
  function shareMountedOrUnmounted() {
    while (sharesViewListView.model.count != 0) {
      sharesViewListView.model.remove(0)
    }
    
    for (var i = 0; i < iface.mountedShares.length; i++) {
      // The unmounted() signal is emitted before the share is
      // actually removed from the list. So, we need to check 
      // here, if the share is still mounted.
      if (iface.mountedShares[i].isMounted) {
        sharesViewListView.model.append({"object": iface.mountedShares[i]})
      }
      else {
        // Do nothing
      }
    }
  }
}
