/***************************************************************************
    bookmarkspage.qml - The bookmarks page of Smb4K's plasmoid
                             -------------------
    begin                : So Apr 28 2013
    copyright            : (C) 2013 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaComponents.Page {
  id: bookmarksPage
  
  //
  // The tool bar
  //
  PlasmaComponents.ToolBar {
    id: bookmarksToolBar
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      topMargin: 2
      rightMargin: 4
      leftMargin: 4
    }
    PlasmaComponents.ToolBarLayout {
      id: bookmarksToolBarLayout
      spacing: 2
          
      PlasmaComponents.ToolButton {
        id: backButton
        text: i18n( "Back" )
        iconSource: "go-previous"
        width: minimumWidth
        onClicked: {
          back()
        }
      }
      PlasmaComponents.ToolButton {
        id: editButton
        text: i18n( "Edit" )
        iconSource: "bookmarks-organize"
        width: minimumWidth
        onClicked: {
          iface.editBookmarks() 
        }
      }
      Item {
        id: spacer
      }
    }
        
    tools: bookmarksToolBarLayout
  }
  
  PlasmaExtras.ScrollArea {
    id: bookmarksScrollArea
    anchors {
      top: bookmarksToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: 5
    }
    ListView {
      id: bookmarksListView
      delegate: BookmarkItemDelegate {
        id: bookmarkDelegate
        onItemClicked: {
          var object = bookmarksListView.model.get(index).object
          if ( object !== null ) {
            bookmarkOrGroupClicked( object )
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
    onBookmarksListChanged: fillView()
  }
  
  //
  // Do initial things
  //
  Component.onCompleted: {
    fillView()
  }
  
  //
  // Bookmark or group was clicked
  //
  function bookmarkOrGroupClicked( object ) {
    if ( object.isGroup ) {
      while ( bookmarksListView.model.count != 0 ) {
        bookmarksListView.model.remove(0)
      }
      getBookmarks( object.groupName )
    }
    else {
      iface.mount( object.url )
    }
  }
  
  //
  // Back button
  //
  function back() {
    // Since the 'Back' button is only useful when you
    // are currently in a group subfolder and want to 
    // go back to the toplevel, just run fillView() here.
    fillView()
  }
  
  //
  // Fill the view
  //
  function fillView() {
    //
    // Fill the view with the groups and those bookmarks
    // that do not belong into any group. Since this function
    // is only invoked when bookmarks are added or edited, we
    // can live with this very basic approach.
    //
    while ( bookmarksListView.model.count != 0 ) {
      bookmarksListView.model.remove(0)
    }
    getGroups()
    getBookmarks( "" )
  }
  
  //
  // Get the groups
  //
  function getGroups() {
    // Get the groups
    if ( iface.bookmarkGroups.length != 0 ) {
      for ( var i = 0; i < iface.bookmarkGroups.length; ++i ) {
        if ( iface.bookmarkGroups[i].groupName.length != 0 ) {
          bookmarksListView.model.append( { "object": iface.bookmarkGroups[i] } )
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
  
  //
  // Get the bookmarks
  //
  function getBookmarks( group_name ) {
    if ( iface.bookmarks.length != 0 ) {
      for ( var i = 0; i < iface.bookmarks.length; i++ ) {
        if ( iface.bookmarks[i].groupName == group_name ) {
          bookmarksListView.model.append( { "object": iface.bookmarks[i] } )
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
    for ( var i = 0; i < bookmarksListView.model.count; i++ ) {
      if ( !bookmarksListView.model.get(i).object.isGroup ) {
        var object = iface.findMountedShare( bookmarksListView.model.get(i).object.url, false )
        if ( object !== null ) {
          bookmarksListView.model.get(i).object.icon = object.icon
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
}
