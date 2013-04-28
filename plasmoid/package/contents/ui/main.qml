/***************************************************************************
    main.qml - The main file for Smb4K's plasmoid
                             -------------------
    begin                : Sa Feb 11 2012
    copyright            : (C) 2012 by Alexander Reinholdt
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
import org.kde.qtextracomponents 0.1


PlasmaExtras.App {
  id: mainwindow
  property int minimumWidth: 400
  property int minimumHeight: 300
  property string parent_item: ""
  property int parent_type: 0 // Unknown
  property url parent_url
  
  Mounter {
    id: mounter
    onMounted: {
      addMountedShares()
    }
    onUnmounted: {
      removeUnmountedShares()
    }
  }
  
  Print {
    id: printer
  }
  
  //
  // The widgets
  //
  PlasmaComponents.TabBar {
    id: tabBar
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    PlasmaComponents.TabButton {
      id: browserTabButton
      text: i18n( "Network Neighborhood" )
      iconSource: "network-workgroup"
      tab: browserPage
    }
    
    PlasmaComponents.TabButton {
      id: sharesTabButton
      text: i18n( "Mounted Shares" )
      iconSource: "folder-remote"
      tab: sharesPage
    }
    
    PlasmaComponents.TabButton {
      id: bookmarkTabButton
      text: i18n( "Bookmarks" )
      iconSource: "favorites"
      tab: bookmarksPage
    }
  }
      
  PlasmaComponents.TabGroup {
    id: tabGroup
    currentTab: browserPage
    anchors {
      top: tabBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
      
    //
    // The browser widget
    //
    BrowserPage {
      id: browserPage
      
      // Do not connect signals here, because this leads
      // to a stack overflow. Use Component.onCompleted
      // below.
    }
    
    //
    // The mounter widget
    //
    PlasmaComponents.Page {
      id: sharesPage
      
      PlasmaExtras.ScrollArea {
        id: sharesViewScrollArea
        anchors.fill: parent
        GridView {
          id: sharesView
          cellWidth: 120
          cellHeight: 80
          delegate: SharesViewItemDelegate {
            id: sharesViewDelegate
            onOpenClicked: {
              sharesView.currentIndex = index
              mountedShareClicked()
            }
            onUnmountClicked: {
              sharesView.currentIndex = index
              unmountShare()
            }
          }
          model: ListModel {}
          focus: true
          highlightRangeMode: GridView.StrictlyEnforceRange
        }
      }
    }
    
    //
    // The bookmarks page
    //
    PlasmaComponents.Page {
      id: bookmarksPage
    }
  }
  
  PlasmaComponents.BusyIndicator {
    id: busyIndicator
    running: false
    visible: false
    opacity: 0.5
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter
  }
  
  Component.onCompleted: {
    //
    // The browser
    //
    browserPage.instance.start()
    browserPage.busy.connect(busy)
    browserPage.idle.connect(idle)
    
    mounter.start()
    printer.start()
  }
  
  //
  // Add mounted shares to the shares view
  //
  function addMountedShares() {
    // We know that at least one share is mounted, so we do not need 
    // to check that the list of mounted shares is empty.
    for ( var i = 0; i < mounter.mountedShares.length; i++ ) {

      var have_item = false

      if ( sharesView.model.count != 0 ) {
        for ( var j = 0; j < sharesView.model.count; j++ ) {
          if ( sharesView.model.get( j ).itemURL == mounter.mountedShares[i].url ) {
            have_item = true
            break
          }
          else {
            // Do nothing
          }
        }
      }

      if ( !have_item ) {
        sharesView.model.append( {
                          "itemName": mounter.mountedShares[i].shareName,
                          "itemHost": mounter.mountedShares[i].hostName,
                          "itemIcon": mounter.mountedShares[i].icon,
                          "itemURL": mounter.mountedShares[i].url,
                          "itemPath": mounter.mountedShares[i].mountpoint } )
      }
      else {
        // Do nothing
      }
    }
    
    // Now modify the icon in the browser
    if ( parent_type == 2 ) {
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var object = mounter.find( browserListView.model.get( i ).itemURL, false )
        
        if ( object && parent_item == object.hostName ) {
          browserListView.model.get( i ).itemIcon = object.icon
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
  // Remove unmounted shares from the shares view
  //
  function removeUnmountedShares() {
    // Remove obsolete shares.
    if ( sharesView.model.count != 0 ) {
      obsolete_shares = new Array()

      for ( var i = 0; i < sharesView.model.count; i++ ) {
        var object = mounter.find( sharesView.model.get( i ).itemURL )
        
        if ( !object || !object.isMounted ) {
          obsolete_shares.push( sharesView.model.get( i ).itemURL.toString() )
        }
        else {
          // Do nothing
        }
      }

      if ( obsolete_shares.length != 0 ) {
        for ( var i = 0; i < obsolete_shares.length; i++ ) {
          for ( var j = 0; j < sharesView.model.count; j++ ) {
            if ( obsolete_shares[i] == sharesView.model.get( j ).itemURL.toString() ) {
              sharesView.model.remove( j )
              break
            }
            else {
              continue
            }
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
    
    // Now modify the icon in the browser
    if ( parent_type == 2 ) {
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var object = mounter.find( browserListView.model.get( i ).itemURL, false )
        
        if ( object ) {
          if ( !object.isMounted && parent_item == object.hostName ) {
            browserListView.model.get( i ).itemIcon = object.icon
          }
          else {
            // Do nothing
          }
        }
        else {
          var obj = browserPage.instance.find( browserListView.model.get( i ).itemURL, browserListView.model.get( i ).itemType )
          
          if ( obj ) {
            browserListView.model.get( i ).itemIcon = obj.icon
          }
          else {
            // Do nothing
          }
        }
      }
    }
    else {
      // Do nothing
    }
  }
  
  //
  // A mounted share was clicked
  //
  function mountedShareClicked() {
    Qt.openUrlExternally( sharesView.model.get( sharesView.currentIndex ).itemPath )
  }
  
  //
  // A mounted share is to be unmounted
  //
  function unmountShare() {
    mounter.unmount( sharesView.model.get( sharesView.currentIndex ).itemPath )
  }
  
  //
  // Abort any actions performed by the core
  //
  function abort() {
    mounter.abortAll
    printer.abortAll
  }

  
  //
  // The application is busy
  //
  function busy() {
    print( "indicating busy state" )
    busyIndicator.visible = true
    busyIndicator.running = true
  }
  
  //
  // The application has become idle
  //
  function idle() {
    print( "indicating idle state" )
    busyIndicator.running = false
    busyIndicator.visible = false
  }
}

