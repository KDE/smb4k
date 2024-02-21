/*
    SPDX-FileCopyrightText: 2017-2024 Alexander Reinholdt <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents

FocusScope {
  id: popupDialog
  anchors.fill: parent
  
  ColumnLayout {
    anchors.fill: parent
    
    //
    // Tab bar
    //
    TabBar {
      id: tabBar
      
      Layout.maximumWidth: parent.width
      Layout.fillWidth: true
      Layout.fillHeight: false
          
      TabButton {
        id: networkBrowserTabButton
        text: i18n("Network Neighborhood")
        icon.name: "network-workgroup-symbolic"
      }
          
      TabButton {
        id: sharesViewTabButton
        text: i18n("Mounted Shares")
        icon.name: "folder-network-symbolic"
      }
        
      TabButton {
        id: bookmarkPageTabButton
        text: i18n("Bookmarks")
        icon.name: "bookmarks"
      }
        
      TabButton {
        id: profilesPageTabButton
        text: i18n("Profiles")
        icon.name: "format-list-unordered"
      }
      
      TabButton {
        id: configurationPageTabButton
        text: i18n("Configuration")
        icon.name: "configure"
      }
    }
      
    //
    // Tab group
    //
    StackLayout {
      id: tabGroup
      
      Layout.fillWidth: true
      Layout.fillHeight: true
      
      currentIndex: tabBar.currentIndex
        
      NetworkBrowserPage {
        id: networkBrowserPage
      }
          
      SharesViewPage {
        id: sharesViewPage
      }
          
      BookmarksPage {
        id: bookmarksPage
      }
          
      ProfilesPage {
        id: profilesPage
      }
      
      ConfigurationPage {
        id: configurationPage
      }
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    function onBusy() { busy() }
    function onIdle() { idle() }
  }
  
  //
  // Busy indicator
  //
  PlasmaComponents.BusyIndicator {
    id: busyIndicator
    running: false
    visible: false
    anchors.verticalCenter: parent.verticalCenter
    anchors.horizontalCenter: parent.horizontalCenter
  }
  
  //
  // Functions
  //
  function busy() {
    busyIndicator.visible = true
    busyIndicator.running = true
  }
  
  function idle() {
    busyIndicator.visible = false
    busyIndicator.running = false
  }
}
