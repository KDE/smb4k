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
  id: profilesPage
  
  //
  // Tool bar
  //
  // FIXME: Include tool bar
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: profileItemDelegateModel
    
    function lessThan(left, right) {
      return (left.profileName < right.profileName)
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
          profileItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: ProfileItemDelegate {
      id: profileItemDelegate
        
      onItemClicked: {
        profilesListView.currentIndex = DelegateModel.itemsIndex
        iface.activeProfile = object.profileName
      }
    }
  }
  
  //
  // List view
  //
  PlasmaComponents.ScrollView {
    id: profilesScrollArea
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: profilesListView
      model: profileItemDelegateModel
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
    function onProfilesListChanged() { fillView() }
    function onActiveProfileChanged() { fillView() }
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    fillView()
  }
  
  //
  // Functions
  //
  function fillView() {
    while (profileItemDelegateModel.model.count != 0) {
      profileItemDelegateModel.model.remove(0)
    }
    
    if (iface.profileUsage && iface.profiles.length != 0) {
      for (var i = 0; i < iface.profiles.length; i++) {
        profileItemDelegateModel.model.append({"object": iface.profiles[i]})
      }
    }
  }  
}
