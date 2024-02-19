/*
    SPDX-FileCopyrightText: 2017-2023 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQml.Models 2.3
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
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: sharesViewItemDelegateModel
    
    function lessThan(left, right) {
      var less = false
      
      if (left.hostName == right.hostName) {
        less = (left.shareName < right.shareName)
      }
      else {
        less = (left.shareName < right.shareName && left.hostName < right.hostName)
      }

      return less
    }
    
    function insertPosition(item) {
      var lower = 0
      var upper = items.count
      
      while (lower < upper) {
        var middle = Math.floor(lower + (upper - lower) / 2)
        var result = lessThan(item.model.object, items.get(middle).model.object)
        if (result) {
          upper = middle
        }
        else {
          lower = middle + 1
        }
      }
      return lower
    }
    
    function sort() {
      while (unsortedMountedShares.count > 0) {
        var item = unsortedMountedShares.get(0)
        var index = insertPosition(item)
        
        item.groups = "items"
        items.move(item.itemsIndex, index)
      }
    }
    
    items.includeByDefault: false
    
    groups: [ 
      DelegateModelGroup {
        id: unsortedMountedShares
        name: "unsorted"
      
        includeByDefault: true
      
        onChanged: {
          sharesViewItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: SharesViewItemDelegate {
      id: sharesViewItemDelegate
        
      onItemClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        Qt.openUrlExternally(object.mountpoint)
      }
        
      onUnmountClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        iface.unmount(object)
      }
        
      onBookmarkClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        iface.addBookmark(object)
      }
        
      onSyncClicked: {
        sharesViewListView.currentIndex = DelegateModel.itemsIndex
        iface.synchronize(object)
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
      model: sharesViewItemDelegateModel
      clip: true
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    function onMountedSharesChanged() { shareMountedOrUnmounted() }
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    if (sharesViewListView.count == 0) {
      shareMountedOrUnmounted()
    }
  }
  
  //
  // Functions
  //
  function shareMountedOrUnmounted() {
    while (sharesViewItemDelegateModel.model.count != 0) {
      sharesViewItemDelegateModel.model.remove(0)
    }
    
    for (var i = 0; i < iface.mountedShares.length; i++) {
      var mountedShare = iface.mountedShares[i]
      
      if (mountedShare !== null) {
        sharesViewItemDelegateModel.model.append({"object": mountedShare})
      }
    }
    
    sharesViewListView.currentIndex = 0
  }
}
