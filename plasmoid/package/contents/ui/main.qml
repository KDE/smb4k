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
  }

  Mounter {
    id: mounter
  }
  
  Component {
    id: browserItemDelegate
    Item {
      width: browserListView.width
      height: 30
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
        }
      
        PlasmaComponents.ToolButton {
          id: abortButton
          iconSource: "process-stop"
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
//       highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
      focus: true
    }

    Component.onCompleted: {
      scanner.start()
    }
  }

  function getWorkgroups() {
    if ( browserModel.count != 0 ) {
      for ( var i = 0; i < browserModel.count; i++ ) {
        var have_item = false
        
        for ( var j = 0; j < scanner.workgroups.length; j++ ) {
          if ( scanner.workgroups[j].workgroupName == browserModel.get( i ).itemName ) {
            have_item = true
            break
          }
          else {
            // Do nothing
          }
        }
        
        if ( !have_item ) {
          // FIXME: Is this safe?
          browserModel.remove( i )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    if ( scanner.workgroups.length != 0 ) {
      for ( var i = 0; i < scanner.workgroups.length; i++ ) {
        var have_item = false
        
        if ( browserModel.count != 0 ) {
          for ( var j = 0; j < browserModel.count; j++ ) {
            if ( browserModel.get( j ).itemName == scanner.workgroups[i].workgroupName ) {
              have_item = true
              break
            }
            else {
              // Do nothing
            }
          }
        }
        
        if ( !have_item ) {
          browserModel.append( { "itemName": scanner.workgroups[i].workgroupName,
                                 "itemComment": scanner.workgroups[i].comment,
                                 "itemIcon": scanner.workgroups[i].icon } )
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      browserModel.clear
    }
  }

  function getHosts() {
    if ( scanner.hosts.length != 0 )
    {
      for ( var i = 0; i < scanner.hosts.length; i++ ) {
        print( scanner.hosts[i].hostName )
      }
    }
    else
    {
      // Do nothing
    }
  }

  function getShares() {
    if ( scanner.shares.length != 0 ) {
      for ( var i = 0; i < scanner.shares.length; i++ )
      {
        print( scanner.shares[i].shareName )
      }
    }
    else
    {
      // Do nothing
    }
  }
  
  function itemClicked() {
    print( "Item clicked: "+browserModel.get( browserListView.currentIndex ).itemName )
  }
}

