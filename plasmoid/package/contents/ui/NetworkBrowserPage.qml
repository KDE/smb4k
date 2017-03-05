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
  
  property var currentObject: 0

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
    
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: rescanButton
        tooltip: i18n("Rescan")
        iconSource: "view-refresh"
        onClicked: {
          rescan()
        }
      }

      PlasmaComponents.ToolButton {
        id: abortButton
        tooltip: i18n("Abort")
        iconSource: "process-stop"
        onClicked: {
          abort()
        }
      }
      
      PlasmaComponents.ToolButton {
        id: upButton
        tooltip: i18n("Go one level up")
        iconSource: "go-up-symbolic"
        onClicked: {
          if (networkBrowserListView.model.count != 0) {
            var object = networkBrowserListView.model.get(0).object
            up(object)
          }
          else {
            // Do nothing
          }
        }
      }
      
      PlasmaComponents.ToolButton {
        id: mountDialogButton
        tooltip: i18n("Open the mount dialog")
        iconSource: "view-form"
        onClicked: {
          // FIXME: Use Plasma dialog
          iface.openMountDialog()
        }
      }
    }
  }
      
  //
  // List view (the scroll area adjusts itself around the listview)
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
      clip: true
      
      delegate: NetworkBrowserItemDelegate {
        id: networkBrowserItemDelegate
        
        onItemClicked: {
          currentObject = networkBrowserListView.model.get(index).object
          networkItemClicked(currentObject)
        }
        
        onBookmarkClicked: {
          var object = networkBrowserListView.model.get(index).object
          if (object !== null) {
            currentObject = object
            iface.addBookmark(object)
          }
          else {
            // Do nothing
          }
        }
        
        onConfigureClicked: {
          var object = networkBrowserListView.model.get(index).object
          if (object !== null) {
            currentObject = object
            iface.openCustomOptionsDialog(object)
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
    onWorkgroupsChanged: getWorkgroups()
    onHostsChanged: getHosts()
    onSharesChanged: getShares()
    onMountedSharesChanged: shareMountedOrUnmounted()
  }
  
  //
  // Functions
  //
  function rescan() {
    if (currentObject !== null) {
      iface.lookup(currentObject)
    }
    else {
      iface.lookup()
    }
  }
  
  function abort() {
    console.log("Stop only if running")
    iface.stopScanner()
    iface.stopMounter()
    iface.stopPrinter()
  }
  
  function up(object) {
    if (object !== null) {
      switch (object.type) {
        case NetworkObject.Workgroup:
          currentObject = null
          break
        case NetworkObject.Host:
          currentObject = null
          iface.lookup()
          break
        case NetworkObject.Share:
          var parentObject = iface.findNetworkItem(object.parentURL, object.parentType)
          
          if (parentObject !== null) {
            var grandparentObject = iface.findNetworkItem(parentObject.parentURL, parentObject.parentType)
            
            if (grandparentObject !== null) {
              currentObject = grandparentObject
              iface.lookup(grandparentObject)
            }
            else {
              // Do nothing
            }
          }
          else {
            // Do nothing
          }
          break
        default:
          break
      }
    }
    else {
      // Do nothing
    }
  }
  
  function networkItemClicked(object) {
    if (object !== null) {
      if (object.type == NetworkObject.Share) {
        if (!object.isPrinter) {
          iface.mount(object)
        }
        else {
          iface.print(object)
        }
      }
      else {
        iface.lookup(object)
      }
    }
    else {
      // Do nothing
    }
  }
  
  function getWorkgroups() {
    while (networkBrowserListView.model.count != 0) {
      networkBrowserListView.model.remove(0)
    }
    
    if (iface.workgroups.length != 0) {
      for (var i = 0; i < iface.workgroups.length; i++) {
        networkBrowserListView.model.append({"object": iface.workgroups[i]})
      }
    }
    else {
      // Do nothing
    }
  }
  
  function getHosts() {
    var workgroupName = ""
    
    if (currentObject !== null) {
      workgroupName = currentObject.workgroupName
    }
    else {
      // Do nothing
    }
    
    while (networkBrowserListView.model.count != 0) {
      networkBrowserListView.model.remove(0)
    }
    
    if (iface.hosts.length != 0) {
      for (var i = 0; i < iface.hosts.length; i++) {
        if (workgroupName.length != 0 && workgroupName == iface.hosts[i].workgroupName) {
          networkBrowserListView.model.append({"object": iface.hosts[i]})
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
  }
  
  function getShares() {
    var hostName = ""
    
    if (currentObject !== null) {
      hostName = currentObject.hostName
    }
    else {
      // Do nothing
    }
    
    while (networkBrowserListView.model.count != 0) {
      networkBrowserListView.model.remove(0)
    }
    
    if (iface.shares.length != 0) {
      for (var i = 0; i < iface.shares.length; i++) {
        if (hostName.length != 0 && hostName == iface.shares[i].hostName) {
          networkBrowserListView.model.append({"object": iface.shares[i]})
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
  }
  
  function shareMountedOrUnmounted() {
    if (networkBrowserListView.model.count != 0) {
      for (var i = 0; i < networkBrowserListView.model.count; i++) {
        var object = networkBrowserListView.model.get(i).object
        if (object !== null) {
          if (object.type == NetworkObject.Share) {
            var mountedShare = iface.findMountedShare(object.url, false)
            if (mountedShare !== null) {
//               object.icon = mountedShare.icon
            }
            else {
//               object.icon = // FIXME
            }
          }
          else {
            break
          }
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    networkBrowserListView.model.sync
  }
}
