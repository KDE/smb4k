/***************************************************************************
 *   Copyright (C) 2017-2019 by Alexander Reinholdt                        *
 *                              <alexander.reinholdt@kdemail.net>          *
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

PlasmaComponents.Page {
  id: bookmarksPage
  
  //
  // Tool bar
  //
  PlasmaComponents.ToolBar {
    id: bookmarksToolBar
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: backButton
        tooltip: i18n("Go back")
        iconSource: "go-previous"
        width: minimumWidth
        onClicked: {
          back()
        }
      }
      PlasmaComponents.ToolButton {
        id: editButton
        tooltip: i18n("Edit bookmarks")
        iconSource: "bookmarks-organize"
        width: minimumWidth
        onClicked: {
          iface.editBookmarks()
        }
      }
    }
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: bookmarkItemDelegateModel
    
    function lessThan(left, right) {
      var less = false

      if (left.isGroup && right.isGroup) {
        less = (left.groupName < right.groupName)
      }
      else if (!left.isGroup && !right.isGroup) {        
        if (left.hostName == right.hostName) {
          less = (left.shareName < right.shareName)
        }
        else {
          less = (left.shareName < right.shareName && left.hostName < right.hostName)
        }
      }
      else {
        // Do nothing
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
          bookmarkItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: BookmarkItemDelegate {
      id: bookmarkItemDelegate
        
      onItemClicked: {
        bookmarksListView.currentIndex = DelegateModel.itemsIndex
        bookmarkOrGroupClicked(object)
      }
    }
  }
  
  //
  // List view
  //
  PlasmaExtras.ScrollArea {
    id: bookmarksScrollArea
    
    anchors {
      top: bookmarksToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: bookmarksListView
      model: bookmarkItemDelegateModel
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
    onMountedSharesChanged: shareMountedOrUnmounted()
    onBookmarksListChanged: fillView()
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
  function back() {
    // Since the 'Back' button is only useful when you
    // are currently in a group subfolder and want to 
    // go back to the toplevel, just run fillView() here.
    fillView()
  }
  
  function bookmarkOrGroupClicked(object) {
    if (object.isGroup) {
      while (bookmarkItemDelegateModel.model.count != 0) {
        bookmarkItemDelegateModel.model.remove(0)
      }
      
      getBookmarks(object.groupName)
      
      // Set the current item to 0
      bookmarksListView.currentIndex = 0
    }
    else {
      iface.mountBookmark(object.url)
    }
  }
  
  function shareMountedOrUnmounted() {
    for (var i = 0; i < bookmarkItemDelegateModel.model.count; i++) {
      var object = bookmarkItemDelegateModel.model.get(i).object
      
      if (object !== null) {
        if (!object.isGroup) {
          var mountedShare = iface.findMountedShare(object.url, false)
          
          if (mountedShare !== null) {
            object.isMounted = mountedShare.isMounted
          }
          else {
            object.isMounted = false
          }
          
          bookmarkItemDelegateModel.model.set(i, {"object": object})
        }
        else {
          // Do nothing
        }
      }
      else {
        // Do nothing
      }
    }
  }
  
  function fillView() {
    while (bookmarkItemDelegateModel.model.count != 0) {
      bookmarkItemDelegateModel.model.remove(0)
    }
    
    // Get groups
    if (iface.bookmarkGroups.length != 0) {
      for (var i = 0; i < iface.bookmarkGroups.length; i++) {
        if (iface.bookmarkGroups[i].groupName.length != 0) {
          bookmarkItemDelegateModel.model.append({"object": iface.bookmarkGroups[i]})
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    // Get toplevel bookmarks
    getBookmarks("")
    
    // Set the current item to 0
    bookmarksListView.currentIndex = 0
  }
  
  function getBookmarks(groupName) {
    if (iface.bookmarks.length != 0) {
      for (var i = 0; i < iface.bookmarks.length; i++) {
        if (iface.bookmarks[i].groupName == groupName) {
          var bookmark = iface.bookmarks[i]
          var mountedShare = iface.findMountedShare(bookmark.url, false)
          if (mountedShare !== null) {
            bookmark.isMounted = mountedShare.isMounted
          }
          else {
            // Do nothing
          }
          bookmarkItemDelegateModel.model.append({"object": bookmark})
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
}
