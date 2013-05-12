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
  
  signal busy()
  signal idle()
  
  BookmarkHandler {
    id: bookmarkHandler
    onUpdated: {
      getGroupsAndBookmarks()
    }
  }
  
  Mounter {
    id: mounter
    onMounted: {
      shareMounted()
    }
    onUnmounted: {
      shareUnmounted()
    }
    onAboutToStart: {
      bookmarksPage.busy()
    }
    onFinished: {
      bookmarksPage.idle()
    }
  }
  
  PlasmaExtras.ScrollArea {
    id: bookmarksScrollArea
    anchors.fill: parent
    ListView {
      id: bookmarksListView
      delegate: BookmarkItemDelegate {
        id: bookmarkDelegate
        onItemClicked: {
          bookmarksListView.currentIndex = index
          bookmarkClicked()
        }
        onRemoveClicked: {
          bookmarksListView.currentIndex = index
          removeBookmark()
        }
      }
      model: ListModel {}
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  Component.onCompleted: {
    getGroupsAndBookmarks()
    mounter.start()
  }
  
  //
  // Get the bookmarks and groups
  //
  function getGroupsAndBookmarks() {
    while ( bookmarksListView.model.count != 0 ) {
      bookmarksListView.model.remove( 0 )
    }
    
    // Get the groups
    if ( bookmarkHandler.groups.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.groups.length; ++i ) {
        if ( bookmarkHandler.groups[i].description.length != 0 ) {
          bookmarksListView.model.append( {
            "itemIsGroup": bookmarkHandler.groups[i].isGroup,
            "itemIcon": bookmarkHandler.groups[i].icon,
            "itemDescription": bookmarkHandler.groups[i].description } )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    // Get the bookmarks that do not belong to a group
    if ( bookmarkHandler.bookmarks.length != 0 ) {
      for ( var i = 0; i < bookmarkHandler.bookmarks.length; ++i ) {
        if ( bookmarkHandler.bookmarks[i].group.length == 0 ) {
          bookmarksListView.model.append( {
            "itemIsGroup": bookmarkHandler.bookmarks[i].isGroup,
            "itemIcon": bookmarkHandler.bookmarks[i].icon,
            "itemDescription": bookmarkHandler.bookmarks[i].description,
            "itemURL": bookmarkHandler.bookmarks[i].url } )
        }
        else {
          // Do nothing
        }
      }
    }
  }
  
  function bookmarkClicked() {
  }
  
  function removeBookmark() {
    print( "FIXME: Remove bookmark" )
  }
  
  function shareMounted() {
  }
  
  function shareUnmounted() {
  }
}
