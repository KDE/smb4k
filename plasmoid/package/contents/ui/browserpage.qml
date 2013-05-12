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
  
  property string parent_item: ""
  property int parent_type: 0 // Unknown
  property url parent_url
  
  signal busy()
  signal idle()

  Scanner {
    id: scanner
    onWorkgroupsChanged: {
      getWorkgroups()
    }
    onHostsChanged: {
      getHosts()
    }
    onSharesChanged: {
      getShares()
    }
    onAboutToStart: {
      browserPage.busy()
    }
    onFinished: {
      browserPage.idle()
    }
  }
  
  Print {
    id: printer
    onAboutToStart: {
      browserPage.busy()
    }
    onFinished: {
      browserPage.idle()
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
      browserPage.busy()
    }
    onFinished: {
      browserPage.idle()
    }
  }
  
  BookmarkHandler {
    id: bookmarkHandler
  }
  
  OptionsManager {
    id: optionsManager
  }
          
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
          
      PlasmaComponents.ToolButton {
        id: rescanButton
        text: i18n( "Rescan" )
        iconSource: "view-refresh"
        onClicked: {
          rescan()
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
          up()
        }
      }
    }
        
    tools: browserToolBarLayout
  }
      
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
          browserListView.currentIndex = index
              
          if ( browserListView.model.get( index ).itemType < 3 /*share*/ ) {
            parent_item = browserListView.model.get( index ).itemName
            parent_type = browserListView.model.get( index ).itemType
            parent_url  = browserListView.model.get( index ).itemURL
          } 
          else {
            // Do nothing
          }
          networkItemClicked()
        }
        onBookmarkClicked: {
          bookmarkHandler.addBookmark( browserListView.model.get( index ).itemURL )
        }
        onConfigureClicked: {
          optionsManager.openCustomOptionsDialog(  browserListView.model.get( index ).itemURL  )
        }
      }
      model: ListModel {}
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
    }
  }
  
  Component.onCompleted: {
    scanner.start()
    mounter.start()
    printer.start()
  }
  
  //
  // Get the workgroups and show them in the list view
  //
  function getWorkgroups() {
    
    if ( parent_type != 0 /* unknown aka entire network */ ) {
      return
    }
    else {
    }

    // Remove obsolete workgroups
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var object = scanner.find( browserListView.model.get( i ).itemURL, browserListView.model.get( i ).itemType )
        
        if ( !object ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i] == browserListView.model.get( j ).itemName ) {
              browserListView.model.remove( j )
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
    
    // Add new workgroups
    if ( scanner.workgroups.length != 0 ) {
      for ( var i = 0; i < scanner.workgroups.length; i++ ) {
        var have_item = false
        
        if ( browserListView.model.count != 0 ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( browserListView.model.get( j ).itemName == scanner.workgroups[i].workgroupName ) {
              have_item = true
              break
            }
            else {
              // Do nothing
            }
          }
        }
        
        if ( !have_item ) {
          browserListView.model.append( { 
                                 "itemName": scanner.workgroups[i].workgroupName,
                                 "itemComment": scanner.workgroups[i].comment,
                                 "itemIcon": scanner.workgroups[i].icon, 
                                 "itemURL": scanner.workgroups[i].url,
                                 "itemType": scanner.workgroups[i].type,
                                 "itemIsMounted": scanner.workgroups[i].isMounted,
                                 "itemIsPrinter": scanner.workgroups[i].isPrinter } )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      while ( browserListView.model.count != 0 ) {
        browserListView.model.remove( 0 )
      }
    }
  }

  //
  // Get the hosts and show them in the list view
  //
  function getHosts() {
    
    if ( parent_type != 1 /* workgroup */ ) {
      return
    }
    else {
    }
    
    // Remove obsolete hosts
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var object = scanner.find( browserListView.model.get( i ).itemURL, browserListView.model.get( i ).itemType )
        
        if ( !object ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i] == browserListView.model.get( j ).itemName ) {
              browserListView.model.remove( j )
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
    
    // Add new hosts
    if ( scanner.hosts.length != 0 ) {
      for ( var i = 0; i < scanner.hosts.length; i++ ) {
        
        if ( scanner.hosts[i].workgroupName != parent_item ) {
          continue
        }
        else {
          // Do nothing
        }
        
        var have_item = false
        
        if ( browserListView.model.count != 0 ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( browserListView.model.get( j ).itemName == scanner.hosts[i].hostName ) {
              have_item = true
              break
            }
            else {
              // Do nothing
            }
          }
        }
        
        if ( !have_item ) {
          browserListView.model.append( { 
            "itemName": scanner.hosts[i].hostName,
            "itemComment": scanner.hosts[i].comment,
            "itemIcon": scanner.hosts[i].icon, 
            "itemURL": scanner.hosts[i].url,
            "itemType": scanner.hosts[i].type,
            "itemIsMounted": scanner.hosts[i].isMounted,
            "itemIsPrinter": scanner.hosts[i].isPrinter } )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      while ( browserListView.model.count != 0 ) {
        browserListView.model.remove( 0 )
      }
    }
  }

  //
  // Get the shares and show them in the list view
  //
  function getShares() {
    
    if ( parent_type != 2 /* host */ ) {
      return
    }
    else {
    }
    
    // Remove obsolete shares
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var object = scanner.find( browserListView.model.get( i ).itemURL, browserListView.model.get( i ).itemType )
        
        if ( !object ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i] == browserListView.model.get( j ).itemName ) {
              browserListView.model.remove( j )
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
    
    // Add new shares
    if ( scanner.shares.length != 0 ) {
      for ( var i = 0; i < scanner.shares.length; i++ ) {
        
        if ( scanner.shares[i].hostName != parent_item ) {
          continue
        }
        else {
          // Do nothing
        }
        
        var have_item = false
        
        if ( browserListView.model.count != 0 ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( browserListView.model.get( j ).itemName == scanner.shares[i].shareName ) {
              have_item = true
              break
            }
            else {
              // Do nothing
            }
          }
        }
        
        if ( !have_item ) {
          browserListView.model.append( { 
                                 "itemName": scanner.shares[i].shareName,
                                 "itemComment": scanner.shares[i].comment,
                                 "itemIcon": scanner.shares[i].icon, 
                                 "itemURL": scanner.shares[i].url,
                                 "itemType": scanner.shares[i].type,
                                 "itemIsMounted": scanner.shares[i].isMounted,
                                 "itemIsPrinter": scanner.shares[i].isPrinter } )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      while ( browserListView.model.count != 0 ) {
        browserListView.model.remove( 0 )
      }
    }
  }
  
  //
  // An network item was clicked
  //
  function networkItemClicked() {
    // If the current item is a share, mount it or show the print dialog.
    if ( browserListView.model.get( browserListView.currentIndex ).itemType == 3 ) {
      if ( !browserListView.model.get( browserListView.currentIndex ).itemIsPrinter ) {
        mounter.mount( browserListView.model.get( browserListView.currentIndex ).itemURL )
      }
      else {
        printer.print( browserListView.model.get( browserListView.currentIndex ).itemURL )
      }
    }
    else {
      scanner.lookup( parent_url, parent_type )
      
      while ( browserListView.model.count != 0 ) {
        browserListView.model.remove( 0 )
      }
    }
  }
  
  //
  // A share has been mounted
  //
  function shareMounted() {
    // Modify the icon in the browser
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
  // A share has been unmounted
  //
  function shareUnmounted() {
    // Modify the icon in the browser
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
  // Rescan the network neighborhood
  //
  function rescan() {
    if ( parent_type < 3 /* 3 == share */ ) {
      scanner.lookup( parent_url, parent_type )
    }
    else {
      // Do nothing
    }   
  }
  
  //
  // Abort all actions done by the core classes
  //
  function abort() {
    scanner.abortAll()
    printer.abortAll()
    mounter.abortAll()
  }
  
  //
  // Go one level up
  //
  function up() {
    switch ( parent_type )
    {
      case 1: {
        parent_url = "smb://"
        parent_item = ""
        parent_type--
        while ( browserListView.model.count != 0 ) {
          browserListView.model.remove( 0 )
        }  
        rescan()
        break
      }
      case 2: {
        var object = scanner.find( parent_url, parent_type )
        parent_url = "smb://"+object.workgroupName
        parent_item = object.workgroupName
        parent_type--
        while ( browserListView.model.count != 0 ) {
          browserListView.model.remove( 0 )
        }  
        rescan()
        break
      }
      default: {
        break
      }
    }
  }
}

