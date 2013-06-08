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
        onClicked: {
          if ( browserListView.model.count != 0 ) {
            var object = browserListView.model.get(browserListView.currentIndex).object
            if ( object !== null ) {
              rescan( object )
            }
            else {
              // Do nothing
            }
          }
          else {
            print( "FIXME: Handle empty network list view!" )
          }
        }
      }
      
      PlasmaComponents.ToolButton {
        id: abortButton
        text: i18n( "Abort" )
        iconSource: "process-stop"
        onClicked: {
          abort()
        }
      }
          
      PlasmaComponents.ToolButton {
        id: upButton
        text: i18n( "Up" )
        iconSource: "go-up"
        onClicked: {
          if ( browserListView.model.count != 0 ) {
            var object = browserListView.model.get(browserListView.currentIndex).object
            if ( object !== null ) {
              up( object )
            }
            else {
              // Do nothing
            }
          }
          else {
            print( "FIXME: Handle empty network list view!" )
          }
        }
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
            networkItemClicked( object )
          }
          else {
            // Do nothing
          }
        }
        onBookmarkClicked: {
          var object = browserListView.model.get(index).object
          if ( object !== null ) {
            bookmarkHandler.addBookmark( object.url )
          }
          else {
            // Do nothing
          }
        }
        onConfigureClicked: {
          var object = browserListView.model.get(index).object
          if ( object !== null ) {
            optionsManager.openCustomOptionsDialog( object.url )
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
    target: scanner
    onWorkgroupsChanged: getWorkgroups()
    onHostsChanged: getHosts()
    onSharesChanged: getShares()
  }
  
  Connections {
    target: mounter
    onMountedSharesListChanged: shareMountedOrUnmounted()
  }
  
  //
  // A network item was clicked
  //
  function networkItemClicked( object ) {
    if ( object.type == 3 ) {
      if ( !object.isPrinter ) {
        mounter.mount( object.url )
      }
      else {
        printer.print( object.url )
      }
    }
    else {
      scanner.lookup( object.url, object.type )
    }
  }
  
  //
  // Rescan the current level
  //
  function rescan( object ) {
    scanner.lookup( object.parentURL, object.parentType )
  }
  
  //
  // Abort any process run by the involved core classes
  //
  function abort() {
    scanner.abortAll()
    mounter.abortAll()
    printer.abortAll()
  }
  
  //
  // Go one level up
  //
  function up( object ) {
    var parentObject = scanner.find( object.parentURL, object.parentType )
    scanner.lookup( parentObject.parentURL, parentObject.parentType )
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
    if ( scanner.workgroups.length != 0 ) {
      for ( var i = 0; i < scanner.workgroups.length; i++ ) {
        browserListView.model.append( { "object": scanner.workgroups[i] } )
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
    // Clear the list view
    //
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }
    
    //
    // Add the workgroup members
    //
    if ( scanner.hosts.length != 0 ) {
      for ( var i = 0; i < scanner.hosts.length; i++ ) {
        browserListView.model.append( { "object": scanner.hosts[i] } )
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
    // Clear the list view
    //
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }
      
    //
    // Add the workgroup members
    //
    if ( scanner.shares.length != 0 ) {
      for ( var i = 0; i < scanner.shares.length; i++ ) {
        browserListView.model.append( { "object": scanner.shares[i] } )
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
  }
  
//   function shareMounted() {
//     if ( browserListView.model.count != 0 ) {
//       var object = browserListView.model.get(browserListView.currentIndex).object
//       if ( object.type == 3 /* share */ ) {
//         for ( var i = 0; i < browserListView.model.count; i++ ) {
//           var obj = mounter.find( browserListView.model.get(i).object.url, false )
//           if ( obj !== null && object.hostName == obj.hostName ) {
//             browserListView.model.get(i).object.icon = object.icon
//           }
//           else {
//             // Do nothing
//           }
//         }
//       }
//       else {
//         // Do nothing
//       }
//     }
//     else {
//       // Do nothing
//     }
//   }
//   
//   function shareUnmounted() {
//     if ( browserListView.model.count != 0 ) {
//       var object = browserListView.model.get(browserListView.currentIndex).object
//       if ( object.type == 3 /* share */ ) {
//         for ( var i = 0; i < browserListView.model.count; i++ ) {
//           var obj = mounter.find( browserListView.model.get(i).object.url, false )
//           if (  obj !== null ) {
//             if ( !obj.isMounted && object.hostName == obj.hostName ) {
//               browserListView.model.get(i).object.icon = obj.icon
//             }
//             else {
//               // Do nothing
//             }
//           }
//           else {
//             var alt_obj = scanner.find( browserListView.model.get(i).object.url, browserListView.model.get(i).object.type )
//             if ( alt_obj !== null ) {
//               browserListView.model.get(i).object.icon = alt_obj.icon
//             }
//             else {
//               // Do nothing
//             }
//           }
//         }
//       }
//       else {
//         // Do nothing
//       }
//     }
//     else {
//       // Do nothing
//     }
//   }
}

