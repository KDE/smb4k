/***************************************************************************
    browserpage.qml - The browser of Smb4K's plasmoid
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
  id: browserPage
  
  property url lastUrl: ""
  property int lastType: 0
  
  //
  // The tool bar
  //
  PlasmaComponents.ToolBar {
    id: browserToolBar
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      topMargin: 2
      rightMargin: 4
      leftMargin: 4
    }
    PlasmaComponents.ToolBarLayout {
      id: browserToolBarLayout
      spacing: 2
          
      PlasmaComponents.ToolButton {
        id: rescanButton
        text: i18n( "Rescan" )
        iconSource: "view-refresh"
        width: minimumWidth
        onClicked: {
          rescan()
        }
      }
      PlasmaComponents.ToolButton {
        id: abortButton
        text: i18n( "Abort" )
        iconSource: "process-stop"
        width: minimumWidth
        onClicked: {
          abort()
        }
      }
      PlasmaComponents.ToolButton {
        id: upButton
        text: i18n( "Up" )
        iconSource: "go-up"
        width: minimumWidth
        onClicked: {
          if ( browserListView.model.count != 0 ) {
            var object = browserListView.model.get(0).object
            if ( object !== null ) {
              // Go one level up.
              up( object )
            }
            else {
              // Do nothing
            }
          }
          else {
            rescan()
          }
        }
      }
      PlasmaComponents.ToolButton {
        id: mountDialogButton
        text: i18n( "Mount Dialog" )
        iconSource: "view-form"
        width: minimumWidth
        onClicked: {
          iface.openMountDialog()
        }
      }
      Item {
        id: spacer
      }
    }
        
    tools: browserToolBarLayout
  }
  
  //
  // The list view
  //
  PlasmaExtras.ScrollArea {
    id: browserScrollArea
    anchors {
      top: browserToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      topMargin: 5
    }
    ListView {
      id: browserListView
      delegate: BrowserItemDelegate {
        id: browserDelegate
        onItemClicked: {
          var object = browserListView.model.get(index).object
          if ( object !== null ) {
            lastUrl = object.url
            lastType = object.type
            networkItemClicked( object )
          }
          else {
            // Do nothing
          }
        }
        onBookmarkClicked: {
          var object = browserListView.model.get(index).object
          if ( object !== null ) {
            lastUrl = object.url
            lastType = object.type            
            bookmarkHandler.addBookmark( object )
          }
          else {
            // Do nothing
          }
        }
        onConfigureClicked: {
          var object = browserListView.model.get(index).object
          if ( object !== null ) {
            lastUrl = object.url
            lastType = object.type
            iface.openCustomOptionsDialog( object )
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
  // A network item was clicked
  //
  function networkItemClicked( object ) {
    if ( object.type == 3 ) {
      if ( !object.isPrinter ) {
        iface.mount( object.url )
      }
      else {
        iface.print( object.url )
      }
    }
    else {
      iface.lookup( object )
    }
  }
  
  //
  // Rescan the current level
  //
  function rescan() {
    if ( browserListView.model.count != 0 ) {
      var object = browserListView.model.get(0).object
      if ( object ) {
        iface.lookup( object )
      }
      else {
        iface.lookup()
      }
    }
    else {
      // Fallback method. Use the last known URL and Type
      // for rescanning.
      var object = iface.findNetworkItem( lastUrl, lastType )
      if ( object !== null ) {
        iface.lookup( object )
      }
      else {
        iface.lookup()
      } 
    }
  }
  
  //
  // Abort any process run by the involved core classes
  //
  function abort() {
    iface.abortScanner();
    iface.abortMounter();
    iface.abortPrinter();
  }
  
  //
  // Go one level up
  //
  function up( object ) {
    switch ( object.type ) {
      case 2:       // host
        iface.lookup()
        break
      case 3:       // share
        var parent = iface.findNetworkItem( object.parentURL, object.parentType )
        var grandparent = iface.findNetworkItem( parent.parentURL, parent.parentType )
        if ( grandparent !== null ) {
          iface.lookup( grandparent )
        }
        else {
          // Do nothing
        }
        break
      default:
        break
    }
  }

  //
  // Get the workgroups and show them in the list view
  //
  function getWorkgroups() {
    //
    // Clear the list view
    //
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }
    
    //
    // Add the workgroups
    //
    if ( iface.workgroups.length != 0 ) {
      for ( var i = 0; i < iface.workgroups.length; i++ ) {
        browserListView.model.append( { "object": iface.workgroups[i] } )
      }
    }
    else {
      // Do nothing
    }
  }

  //
  // Get the hosts and show them in the list view
  //
  function getHosts() {
    //
    // Get the workgroup name the hosts were looked up for
    //
    var workgroup_name = ""
    var object = iface.findNetworkItem( lastUrl, lastType )
    
    if ( object !== null ) {
      workgroup_name = object.workgroupName
    }
    else {
      // Do nothing
    }
    
    //
    // Clear the list view
    //
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }
    
    //
    // Add the workgroup members
    //
    if ( iface.hosts.length != 0 ) {
      for ( var i = 0; i < iface.hosts.length; i++ ) {
        if ( workgroup_name.length != 0 && workgroup_name == iface.hosts[i].workgroupName ) {
          browserListView.model.append( { "object": iface.hosts[i] } )
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
  // Get the shares and show them in the list view
  //
  function getShares() {
    //
    // Get the host name the shares were looked up for
    //
    var host_name = ""
    var object = iface.findNetworkItem( lastUrl, lastType )
    
    if ( object !== null ) {
      host_name = object.hostName
    }
    else {
      // Do nothing
    }

    //
    // Clear the list view
    //
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }
      
    //
    // Add the shares
    //
    if ( iface.shares.length != 0 ) {
      for ( var i = 0; i < iface.shares.length; i++ ) {
        if ( host_name.length != 0 && host_name == iface.shares[i].hostName ) {
          browserListView.model.append( { "object": iface.shares[i] } )
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
  // A share has been mounted or unmounted
  //
  function shareMountedOrUnmounted() {
    if ( browserListView.model.count != 0 ) {
      // Check on which level we are.
      var object = browserListView.model.get(0).object
      if ( object !== null && object.type == 3 /* share */ ) {
        for ( var i = 0; i < browserListView.model.count; i++ ) {
          var obj = iface.findMountedShare( browserListView.model.get(i).object.url, false )
          if ( obj !== null ) {
            browserListView.model.get(i).object.icon = obj.icon
          }
          else {
            browserListView.model.get(i).object.icon = "folder-remote"
          }
        }
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

