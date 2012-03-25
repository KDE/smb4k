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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
// import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts
import org.kde.qtextracomponents 0.1


Item {
  id: mainwindow
  property int minimumWidth: 300
  property int minimumHeight: 200
  property string parent_item: ""
  property int parent_type: 0 // Unknown
  property url parent_url
  
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
      busy()
    }
    onFinished: {
      idle()
    }
  }

  Mounter {
    id: mounter
    onMountedSharesChanged: {
      getMountedShares()
    }
  }

  Component {
    id: browserItemDelegate
    PlasmaComponents.ListItem {
      width: browserListView.width
      height: 40
      Row {
        spacing: 5
        Column {
          QIconItem {
            icon: itemIcon
            width: 22
            height: 22
          }
        }
        Column {
          Text { text: itemName }
          Text { text: "<font size=\"-1\">"+itemComment+"</font>" }
        }
      }
      MouseArea {
        anchors.fill: parent
        onClicked: {
          browserListView.currentIndex = index
          itemClicked()
        }
      }
    }
  }

  ListModel {
    id: browserModel
  }

  Item {
    id: browser
    anchors.fill: parent
    
    PlasmaComponents.ToolBar {
      id: toolBar
      clip: true
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }

      PlasmaComponents.ToolBarLayout {
        id: toolBarLayout
      
        PlasmaComponents.ToolButton {
          id: rescanButton
          iconSource: "view-refresh"
          enabled: true
          onClicked: {
            rescan()
          }
        }
      
        PlasmaComponents.ToolButton {
          id: abortButton
          iconSource: "process-stop"
          enabled: false
          onClicked: {
            abort()
          }
        }
        
        PlasmaComponents.ToolButton {
          id: upButton
          iconSource: "go-up"
          enabled: false
          onClicked: {
            up()
          }
        }
        
        PlasmaComponents.BusyIndicator {
          id: busyIndicator
          height: 22
          width: 22
          visible: false
        }
      }
      
      tools: toolBarLayout
      transition: "set"
    }
    
    ListView {
      id: browserListView
      anchors {
        top: toolBar.bottom
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }
      anchors.topMargin: 10
      delegate: browserItemDelegate
      model: browserModel
      focus: true
      snapMode: ListView.SnapToItem
    }
    
     PlasmaComponents.ScrollBar {
       flickableItem: browserListView
       anchors { 
         right: browserListView.right 
         top: browserListView.top
         bottom: browserListView.bottom
      }
    }

    Component.onCompleted: {
      scanner.start()
      mounter.start()
    }
  }

  //
  // Get the workgroups and show them in the list view
  //
  function getWorkgroups() {
    
    if ( parent_type != 0 /* unknown aka entire network */ ) {
      return
    }
    else {
      upButton.enabled = false
    }

    // Remove obsolete workgroups
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var have_item = false
        
        for ( var j = 0; j < scanner.workgroups.length; j++ ) {
          if ( scanner.workgroups[j].workgroupName == browserListView.model.get( i ).itemName ) {
            have_item = true
            break
          }
          else {
            // Do nothing
          }
        }
        
        if ( !have_item ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i].text == browserListView.model.get( j ).itemName ) {
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
                                 "itemType": scanner.workgroups[i].type } )
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
      upButton.enabled = true
    }
    
    // Remove obsolete hosts
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var have_item = false
        
        for ( var j = 0; j < scanner.hosts.length; j++ ) {
          if ( scanner.hosts[j].hostName == browserListView.model.get( i ).itemName ) {
            have_item = true
            break
          }
          else {
            // Do nothing
          }
        }
        
        if ( !have_item ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i].text == browserListView.model.get( j ).itemName ) {
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
                                 "itemType": scanner.hosts[i].type } )
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
      upButton.enabled = true
    }
    
    // Remove obsolete shares
    if ( browserListView.model.count != 0 ) {
      obsolete_items = new Array()
      
      for ( var i = 0; i < browserListView.model.count; i++ ) {
        var have_item = false
        
        for ( var j = 0; j < scanner.shares.length; j++ ) {
          if ( scanner.shares[j].hostName == browserListView.model.get( i ).itemName ) {
            have_item = true
            break
          }
          else {
            // Do nothing
          }
        }
        
        if ( !have_item ) {
          obsolete_items.push( browserListView.model.get( i ).itemName )
        }
        else {
          // Do nothing
        }
      }
      
      if ( obsolete_items.length != 0 ) {
        for ( var i = 0; i < obsolete_items.length; i++ ) {
          for ( var j = 0; j < browserListView.model.count; j++ ) {
            if ( obsolete_items[i].text == browserListView.model.get( j ).itemName ) {
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
                                 "itemType": scanner.shares[i].type } )
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
  // Get the list of mounted shares
  //
  function getMountedShares() {
    print( "Get the list of mounted shares" )
  }
  
  //
  // An item was clicked
  //
  function itemClicked() {
    
    if ( browserListView.model.get( browserListView.currentIndex ).itemType < 3 /* 3 == share */ ) {
      parent_url = browserListView.model.get( browserListView.currentIndex ).itemURL
      parent_item = browserListView.model.get( browserListView.currentIndex ).itemName
      parent_type = browserListView.model.get( browserListView.currentIndex ).itemType
      
      scanner.lookup( parent_url, parent_type )
      
      while ( browserListView.model.count != 0 ) {
        browserListView.model.remove( 0 )
      }
    }
    else {
      mounter.mount( browserListView.model.get( browserListView.currentIndex ).itemURL )
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
  // Abort any actions performed by the core
  //
  function abort() {
    scanner.abortAll
    mounter.abortAll
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
        rescan()
        break
      }
      case 2: {
        var object = scanner.find( parent_url, parent_type )
        parent_url = "smb://"+object.workgroupName
        parent_item = object.workgroupName
        parent_type--
        rescan()
        break
      }
      default: {
        break
      }
    }
 
    while ( browserListView.model.count != 0 ) {
      browserListView.model.remove( 0 )
    }    
  }
  
  //
  // The application is busy
  //
  function busy() {
    if ( scanner.running ) {
      rescanButton.enabled = false
    }
    else {
      // Do nothing
    }
    abortButton.enabled = true
      
    busyIndicator.visible = true
    busyIndicator.running = true
  }
  
  //
  // The application has become idle
  //
  function idle() {
    rescanButton.enabled = true
    
    if ( !scanner.running && !mounter.running ) {
      abortButton.enabled = false
    }
    else {
      // Do nothing
    }
    
    busyIndicator.running = false
    busyIndicator.visible = false
  }
}

