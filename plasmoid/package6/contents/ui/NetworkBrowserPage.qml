/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQml.Models
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.smb4k.smb4kqmlplugin


PlasmaComponents.Page {
  id: networkBrowserPage
  
  property var parentObject: null
  
  //
  // Tool bar
  //
  PlasmaComponents.ToolBar {
    id: networkBrowserToolBar

    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    RowLayout {
      PlasmaComponents.ToolButton {
        id: rescanButton

        hoverEnabled: true
        icon.name: "view-refresh"
        flat: true

        PlasmaComponents.ToolTip.delay: 1000
        PlasmaComponents.ToolTip.timeout: 5000
        PlasmaComponents.ToolTip.text: i18n("Rescan")
        PlasmaComponents.ToolTip.visible: hovered

        onClicked: {
          rescan()
        }
      }

      PlasmaComponents.ToolButton {
        id: abortButton

        hoverEnabled: true
        icon.name: "process-stop"
        flat: true

        PlasmaComponents.ToolTip.delay: 1000
        PlasmaComponents.ToolTip.timeout: 5000
        PlasmaComponents.ToolTip.text: i18n("Abort")
        PlasmaComponents.ToolTip.visible: hovered

        onClicked: {
          abort()
        }
      }
      
      PlasmaComponents.ToolButton {
        id: upButton

        hoverEnabled: true
        icon.name: "go-up-symbolic"
        flat: true

        PlasmaComponents.ToolTip.delay: 1000
        PlasmaComponents.ToolTip.timeout: 5000
        PlasmaComponents.ToolTip.text: i18n("Go one level up")
        PlasmaComponents.ToolTip.visible: hovered

        onClicked: {
          up()
        }
      }
      
      PlasmaComponents.ToolButton {
        id: mountDialogButton

        hoverEnabled: true
        icon.name: "view-form"
        flat: true

        PlasmaComponents.ToolTip.delay: 1000
        PlasmaComponents.ToolTip.timeout: 5000
        PlasmaComponents.ToolTip.text: i18n("Open the mount dialog")
        PlasmaComponents.ToolTip.visible: hovered

        onClicked: {
          // FIXME: Use Plasma dialog
          iface.openMountDialog()
        }
      }
    }
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: networkBrowserItemDelegateModel
    
    function lessThan(left, right) {
      var less = false
      
      switch (left.type) {
        case NetworkObject.Workgroup:
          less = (left.workgroupName < right.workgroupName)
          break
        case NetworkObject.Host:
          less = (left.hostName < right.hostName)
          break
        case NetworkObject.Share:
          less = (left.shareName < right.shareName)
          break
        default:
          break          
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
      while (unsortedItems.count > 0) {
        var item = unsortedItems.get(0)
        var index = insertPosition(item)
        
        item.groups = "items"
        items.move(item.itemsIndex, index)
      }
    }
    
    items.includeByDefault: false
    
    groups: [ 
      DelegateModelGroup {
        id: unsortedItems
        name: "unsorted"
      
        includeByDefault: true
      
        onChanged: {
          networkBrowserItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: NetworkBrowserItemDelegate {
      id: networkBrowserItemDelegate
        
      onItemClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        networkItemClicked()
      }
        
      onBookmarkClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.addBookmark(object)
      }
        
      onPreviewClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.preview(object)
      }
        
      onConfigureClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.openCustomOptionsDialog(object)
      }
    }
  }
      
  //
  // List view 
  //
  ScrollView {
    id: networkBrowserScrollArea

    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    
    // FIXME: Use layout
    anchors {
      top: networkBrowserToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: networkBrowserListView
      anchors.fill: parent
      model: networkBrowserItemDelegateModel
      clip: true
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
//       highlight: Rectangle {
//         color: theme.highlightColor
//         radius: 5
//         opacity: 0.2
//       }
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    function onWorkgroupsChanged() { getWorkgroups() }
    function onHostsChanged() { getHosts() }
    function onSharesChanged() { getShares() }
    function onMountedSharesChanged() { shareMountedOrUnmounted() }
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    if (networkBrowserListView.count == 0) {
      getWorkgroups()
    }
  }  
  
  //
  // Functions
  //
  function rescan() {
    if (parentObject !== null) {
      iface.lookup(parentObject)
    }
    else {
      iface.lookup()
    }
  }
  
  function abort() {
    iface.abortClient()
    iface.abortMounter()
  }
  
  function up() {
    if (parentObject !== null) {
      
      switch (parentObject.type) {
        case NetworkObject.Workgroup:
          networkBrowserListView.currentIndex = -1
          iface.lookup()
          break
        case NetworkObject.Host:
          var object = iface.findNetworkItem(parentObject.parentUrl, parentObject.parentType)
          if (object !== null) {
            parentObject = object
            iface.lookup(object)
          }
          break
        default:
          break
      }
    }
  }
  
  function networkItemClicked() {
    if (networkBrowserListView.currentIndex != -1) {
      var object = networkBrowserItemDelegateModel.items.get(networkBrowserListView.currentIndex).model.object
      
      if (object.type == NetworkObject.Share) {
        if (!object.isPrinter) {
          iface.mountShare(object)
        }
        else {
          iface.print(object)
        }
      }
      else {
        parentObject = object
        iface.lookup(object)
      }
    }
  }
  
  function getWorkgroups() {
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.workgroups.length != 0) {
      for (var i = 0; i < iface.workgroups.length; i++) {
        networkBrowserItemDelegateModel.model.append({"object": iface.workgroups[i]})
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getHosts() {
    var workgroupName = parentObject.workgroupName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.hosts.length != 0) {
      for (var i = 0; i < iface.hosts.length; i++) {
        if (workgroupName.length != 0 && workgroupName == iface.hosts[i].workgroupName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.hosts[i]})
        }
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getShares() {
    var hostName = parentObject.hostName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.shares.length != 0) {
      for (var i = 0; i < iface.shares.length; i++) {
        if (hostName.length != 0 && hostName == iface.shares[i].hostName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.shares[i]})
        }
      }
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function shareMountedOrUnmounted() {
    for (var i = 0; i < networkBrowserItemDelegateModel.model.count; i++) {
      var object = networkBrowserItemDelegateModel.model.get(i).object
      
      if (object !== null && object.type == NetworkObject.Share) {
        object.isMounted = iface.isShareMounted(object.url)
        networkBrowserItemDelegateModel.model.set(i, {"object": object})
      }
    }
  }
}
