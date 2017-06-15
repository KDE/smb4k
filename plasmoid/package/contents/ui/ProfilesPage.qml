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
import QtQml.Models 2.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0

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
  PlasmaExtras.ScrollArea {
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
    onProfilesListChanged: fillView()
    onActiveProfileChanged: fillView()
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
    else {
      // Do nothing
    }
  }  
}
