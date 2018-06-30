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
import QtQuick.Controls 2.3
import QtQml.Models 2.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0


PlasmaComponents.CommonDialog {
  titleText: i18n("Edit Bookmarks")
  buttonTexts: {[i18n("OK"), i18n("Cancel")]}
  visualParent: parent
  
  property int editorWidth: units.iconSizes.medium * 15
  property var bookmarkList: []
  property var bookmarkGroups: []
  
  BookmarkObject { 
    id: newGroup
    isGroup: true
  }
  
  content: ColumnLayout {
    //
    // Tool bar
    //
    PlasmaComponents.ToolBar {
      id: bookmarkEditorToolBar

      Layout.minimumWidth: editorWidth
      Layout.alignment: Qt.AlignCenter
      Layout.fillWidth: true
      
      tools: PlasmaComponents.ToolBarLayout {
        PlasmaComponents.ToolButton {
          id: clearButton
          tooltip: i18n("Clear")
          iconSource: "edit-clear"
          width: minimumWidth
          onClicked: {
            // Clear the list of bookmark groups
            bookmarkGroups.length = 0
    
            // Clear the bookmarks list
            bookmarkList.length = 0
            
            // Clear the editor widget
            clearEditor()
          }
        }
      }
    }
    
    //
    // List view
    //
    PlasmaExtras.ScrollArea {
      id: bookmarkEditorScrollArea

      Layout.minimumWidth: editorWidth
      Layout.minimumHeight: editorWidth / 2 // Yes, that's correct.
      Layout.alignment: Qt.AlignCenter
      Layout.fillWidth: true
      
      ListView {
        id: bookmarkEditorListView
        model: bookmarkEditorItemDelegateModel
        clip: true
        focus: true
        highlightRangeMode: ListView.StrictlyEnforceRange
        highlightFollowsCurrentItem: true
      }
    }

    //
    // Editor widgets
    // 
    GridLayout {
      id: bookmarkEditorInputWidgets
      columns: 2
      
      enabled: false
      
      //
      // Text input for the label
      //
      PlasmaComponents.Label {
        text: i18n("Label:")
      }
      PlasmaComponents.TextField {
        id: bookmarkEditorLabelInput
      
        Layout.minimumWidth: editorWidth
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        onEditingFinished: {
          changeBookmark()
        }
      }
    
      //
      // Text input for the login
      // 
      PlasmaComponents.Label {
        text: i18n("Login:");
      }
      PlasmaComponents.TextField {
        id: bookmarkEditorLoginInput
        
        Layout.minimumWidth: editorWidth
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        onEditingFinished: {
          changeBookmark()
        }
      }
    
      //
      // Text input for the IP address
      // 
      PlasmaComponents.Label {
        text: i18n("IP Address:")
      }
      PlasmaComponents.TextField {
        id: bookmarkEditorIPInput
        
        Layout.minimumWidth: editorWidth
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
        onEditingFinished: {
          changeBookmark()
        }
      }
    
      //
      // Input combo box for the group name
      // 
      PlasmaComponents.Label {
        text: i18n("Group Name:")
      }
      PlasmaComponents.ComboBox {
        id: bookmarkEditorGroupInput
        
        Layout.minimumWidth: editorWidth
        Layout.alignment: Qt.AlignCenter
        Layout.fillWidth: true
      
        editable: true
        model: ListModel {}
        
        onAccepted: {
          changeBookmark()
        }
        onActivated: {
          changeBookmark()
        }
      }
    }
  }

  onButtonClicked: {
    switch(index) {
      case 0: // OK
        changeBookmark()
        iface.replaceBookmarks(bookmarkList)
        break
      case 1: // Cancel
        break
      default:
        break
    }
  }
  
  //
  // Connections
  // 
  Connections {
    target: iface
    onBookmarksListChanged: fillView()
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: bookmarkEditorItemDelegateModel
    
    function lessThan(left, right) {
      var less = false

      if (left.isGroup && right.isGroup) {
        less = (left.groupName < right.groupName)
      }
      else if (left.groupName == right.groupName) {
        if (left.hostName == right.hostName) {
          less = left.shareName < right.shareName
        }
        else {
          less = (left.hostName < right.hostName && left.shareName < right.shareName)
        }
      }
      else {
        less = (left.groupName < right.groupName)
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
          bookmarkEditorItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: BookmarkEditorItemDelegate {
      id: bookmarkEditorItemDelegate
        
      onItemClicked: {
        bookmarkEditorListView.currentIndex = DelegateModel.itemsIndex
        bookmarkOrGroupClicked(object)
      }
      
      onRemoveClicked: {
        bookmarkEditorListView.currentIndex = DelegateModel.itemsIndex
        removeBookmark(object)
      }
    }
  }
  
  //
  // Functions
  //
  function setup() {
    // Get the lists
    bookmarkList = iface.bookmarks
    bookmarkGroups = iface.bookmarkGroups
    
    // Fill the view
    fillView()
  }
  
  
  function fillView() {
    // Clear the editor
    clearEditor()

    // Insert groups and bookmarks
    if (bookmarkGroups.length != 0) {
      for (var i = 0; i < bookmarkGroups.length; i++) {
        // Insert groups and bookmarks into the view
        bookmarkEditorItemDelegateModel.model.append({"object": bookmarkGroups[i]})
        getBookmarks(bookmarkGroups[i].groupName)
        // Insert groups into the group name combo box
        bookmarkEditorGroupInput.model.append({"entry": bookmarkGroups[i].groupName})
      }
    }
    else {
      // Insert toplevel bookmarks to the view
      getBookmarks("")
      // Insert an empty group into the combo box
      bookmarkEditorGroupInput.model.append({"entry": ""})
    }
    
    // Set the empty group as default in the combo box
    var defaultIndex = bookmarkEditorGroupInput.find("")
    if (defaultIndex != -1) {
      bookmarkEditorGroupInput.currentIndex = defaultIndex
    }
    else {
      // Do nothing
    }
    
    // Set the default index of the view to the first item
    bookmarkEditorListView.currentIndex = 0
  }
  
  function getBookmarks(groupName) {
    if (bookmarkList.length != 0) {
      for (var i = 0; i < bookmarkList.length; i++) {
        if (bookmarkList[i].groupName == groupName) {
          bookmarkEditorItemDelegateModel.model.append({"object": bookmarkList[i]})
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
  
  function bookmarkOrGroupClicked(object) {
    // Enable editor widgets
    bookmarkEditorInputWidgets.enabled = true
    
    // Set texts
    bookmarkEditorLabelInput.text = object.label
    bookmarkEditorLoginInput.text = object.login
    bookmarkEditorIPInput.text = object.hostIP
    var newIndex = bookmarkEditorGroupInput.find(object.groupName)
    if (newIndex != -1) {
      bookmarkEditorGroupInput.currentIndex = newIndex
    }
    else {
      // Do nothing
    }
  }
  
  function changeBookmark() {
    if (bookmarkEditorItemDelegateModel.model.count != 0) {
      // Get the selected bookmark and modify it according to the changes
      // made in the editor widgets
      var object = bookmarkEditorItemDelegateModel.items.get(bookmarkEditorListView.currentIndex).model.object
      if (object !== 0) {
        object.label = bookmarkEditorLabelInput.text
        object.login = bookmarkEditorLoginInput.text
        object.hostIP = bookmarkEditorIPInput.text
        object.groupName = bookmarkEditorGroupInput.currentText
      }
      else {
        // Do nothing
      }
      
      // Add the new group name, if needed
      var groupIndex = bookmarkEditorGroupInput.find(object.groupName)
      if (groupIndex == -1) {
        newGroup.groupName = object.groupName 
        bookmarkGroups.push(newGroup)
      }
      else {
        // Do nothing
      }
    }
    else {
      // Do nothing
    }

    // Fill the view
    fillView()
  }
  
  function clearEditor() {
    // Clear the list view
    while (bookmarkEditorItemDelegateModel.model.count != 0) {
      bookmarkEditorItemDelegateModel.model.remove(0)
    }
    
    // Clear and disable the editor widgets
    // Enable editor widgets
    bookmarkEditorLabelInput.text = ""
    bookmarkEditorLoginInput.text = ""
    bookmarkEditorIPInput.text = ""
    var newIndex = bookmarkEditorGroupInput.find("")
    if (newIndex != -1) {
      bookmarkEditorGroupInput.currentIndex = newIndex
    }
    else {
      // Do nothing
    }
    bookmarkEditorInputWidgets.enabled = false
  }
  
  function removeBookmark(object) {
    // Create a new, temporal list without the object to be removed
    var newBookmarkList = []
    var groupNames = []
    for (var i = 0; i < bookmarkList.length; i++) {
      if (bookmarkList[i] !== object) {
        newBookmarkList.push(bookmarkList[i])
        if (groupNames.indexOf(bookmarkList[i].groupName) == -1) {
          groupNames.push(object.groupName)
        }
        else {
          // Do nothing
        }
      }
      else {
        // Do nothing
      }
    }
    
    // Clear the list of bookmarks
    bookmarkList.length = 0
    
    // Assign the temporal list to the list property
    bookmarkList = newBookmarkList
    
    // Create a temporal list of groups 
    var newGroupList = []
    for (var i = 0; i < bookmarkGroups.length; i++) {
      if (groupNames.indexOf(bookmarkGroups[i].groupName) !== -1) {
        newGroupList.push(bookmarkGroups[i])
      }
      else {
        // Do nothing
      }
    }
    
    // Clear the list of bookmark groups
    bookmarkGroups.length = 0
    
    // Assign the group list to the list property
    bookmarkGroups = newGroupList

    // Fill the view
    fillView()
  }
}
