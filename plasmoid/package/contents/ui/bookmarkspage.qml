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
  
  // FIXME: Implement tool bar with back and edit tool button
  
  PlasmaExtras.ScrollArea {
    id: bookmarksScrollArea
    anchors.fill: parent
    ListView {
      id: bookmarksListView
      delegate: BookmarkItemDelegate {
        id: bookmarkDelegate
        onItemClicked: {
          var object = bookmarksListView.model.get(index).object
          if ( typeof( object ) != 'null' ) {
            bookmarkOrGroupClicked( object )
          }
          else {
            // Do nothing
          }
        }
        onRemoveClicked: {
          var object = bookmarksListView.model.get( index ).object
          if ( typeof( object ) != 'null' ) {
            if ( object.isGroup ) {
              removeGroup( object, index )
            }
            else {
              removeBookmark( object, index )
            }
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
    target: bookmarkHandler
    onUpdated: fillView()
  }
  
  Connections {
    target: mounter
    onMounted: shareMounted()
    onUnmounted: shareUnmounted()
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
        bookmarksListView.model.remove( 0 )
      }
      getBookmarks( object.groupName )
    }
    else {
      mounter.mount( object.url )
    }
  }
  
  //
  // Remove a group
  //
  function removeGroup( object, index ) {
    bookmarksListView.model.remove( index )
    bookmarksHandler.removeGroup( object.groupName )
  }
  
  //
  // Remove a bookmark
  //
  function removeBookmark( object, index ) {
    bookmarksListView.model.remove( index )
    bookmarkHandler.removeBookmark( object.url )
  }
  
  //
  // Fill the view
  //
  function fillView() {
    if ( bookmarksListView.model.count == 0 ) {
      getGroups()
      getBookmarks( "" )
    }
    else {
      var object = bookmarksListView.model.get(0).object
      if ( object.isGroup || object.groupName.length == 0 ) {
        while ( bookmarksListView.model.count != 0 ) {
          bookmarksListView.model.remove(0)
        }
        getGroups()
        getBookmarks( "" )
      }
      else {
        while ( bookmarksListView.model.count != 0 ) {
          bookmarksListView.model.remove(0)
        }
        getBookmarks( object.groupName )
      }
    }
  }
  
  //
  // Get the groups
  //
  function getGroups() {
    // Get the groups
    if ( bookmarkHandler.groups.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.groups.length; ++i ) {
        if ( bookmarkHandler.groups[i].description.length != 0 ) {
          bookmarksListView.model.append( { "object": bookmarkHandler.groups[i] } )
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
    if ( bookmarkHandler.bookmarks.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.bookmarks.length; ++i ) {
        if ( bookmarkHandler.bookmarks[i].groupName == group_name ) {
          bookmarksListView.model.append( { "object": bookmarkHandler.bookmarks[i] } )
        }
        else {
          // Do nothing
        }
      }
    }
  }
  
  function shareMounted() {
    print( "FIXME: Share has been mounted" )
  }
  
  function shareUnmounted() {
    print( "FIXME: Share has been unmounted" )
  }
}
