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
  
  property BookmarkObject currentObject: BookmarkObject{}
  
  PlasmaExtras.ScrollArea {
    id: bookmarksScrollArea
    anchors.fill: parent
    ListView {
      id: bookmarksListView
      delegate: BookmarkItemDelegate {
        id: bookmarkDelegate
        onItemClicked: {
          currentObject = bookmarksListView.model.get( index ).object
          bookmarkOrGroupClicked()
        }
        onRemoveClicked: {
          currentObject = bookmarksListView.model.get( index ).object
          removeBookmark()
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
    target: bookmarkHandler
    onUpdated: refillView()
  }
  
  Connections {
    target: mounter
    onMounted: shareMounted
    onUnmounted: shareUnmounted
  }
  
  //
  // Initially fill the view
  //
  function refillView() {
    print( "FIXME: Reload current view" )
  }
  
  //
  // Get the groups
  //
  function getGroups() {
    // Get the groups
    if ( bookmarkHandler.groups.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.groups.length; ++i ) {
        if ( bookmarkHandler.groups[i].description.length != 0 ) {
          bookmarksListView.model.append( {
            "object": bookmarkHandler.groups[i] } )
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
  function getBookmarks() {
    if ( bookmarkHandler.bookmarks.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.bookmarks.length; ++i ) {
        if ( currentObject.isGroup ) {
          if ( bookmarkHandler.bookmarks[i].group == currentObject.group ) {
            bookmarksListView.model.append( {
              "object": bookmarkHandler.bookmarks[i] } )
          }
          else {
            // Do nothing
          }
        }
        else {
          if ( bookmarkHandler.bookmarks[i].group.length == 0 ) {
            bookmarksListView.model.append( {
              "object": bookmarkHandler.bookmarks[i] } )
          }
          else {
            // Do nothing
          }
        }
      }
    }
  }
  
  //
  // Bookmark or group was clicked
  //
  function bookmarkOrGroupClicked() {
    if ( currentObject.isGroup ) {
      while ( bookmarksListView.model.count != 0 ) {
        bookmarksListView.model.remove( 0 )
      }
      getBookmarks()
    }
    else {
      mounter.mount( currentObject.url )
    }
  }
  
  function removeBookmark() {
    print( "FIXME: Remove bookmark" )
  }
  
  function shareMounted() {
    print( "FIXME: Share has been mounted" )
  }
  
  function shareUnmounted() {
    print( "FIXME: Share has been unmounted" )
  }
}
