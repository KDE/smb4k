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
  
  //
  // The scanner
  //
  Scanner {
    id: scanner
  }
  
  Connections {
    target: scanner
    onAboutToStart: busy()
    onFinished: idle()
  }
  
  //
  // The mounter
  //
  Mounter {
    id: mounter
  }
  
  Connections {
    target: mounter
    onAboutToStart: busy()
    onFinished: idle()
  }
  
  //
  // The print interface
  //
  Print {
    id: printer
  }
  
  Connections {
    target: printer
    onAboutToStart: busy()
    onFinished: idle()
  }
  
  //
  // The bookmark handler
  //
  BookmarkHandler {
    id: bookmarkHandler
  }
  
  //
  // The custom options manager
  //
  OptionsManager {
    id: optionsManager
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
      tab: sharesViewPage
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
    }
    
    //
    // The mounter widget
    //
    SharesViewPage {
      id: sharesViewPage
    }
    
    //
    // The bookmarks page
    //
    BookmarksPage {
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
    // Start the core classes
    //
    scanner.start()
    mounter.start()
    printer.start();
  }
  
  //
  // The application is busy
  //
  function busy() {
    busyIndicator.visible = true
    busyIndicator.running = true
  }
  
  //
  // The application has become idle
  //
  function idle() {
    busyIndicator.running = false
    busyIndicator.visible = false
  }
}

