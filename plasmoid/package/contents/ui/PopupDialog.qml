/*
    SPDX-FileCopyrightText: 2017-2019 Alexander Reinholdt
    <alexander.reinholdt@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

FocusScope {
  id: popupDialog
  anchors.fill: parent
  
  ColumnLayout {
    anchors.fill: parent
    
    //
    // Tab bar
    //
    PlasmaComponents.TabBar {
      id: tabBar
      
      Layout.maximumWidth: parent.width
      Layout.fillWidth: true
      Layout.fillHeight: false
          
      PlasmaComponents.TabButton {
        id: browserTabButton
        text: i18n("Network Neighborhood")
        iconSource: "network-workgroup-symbolic"
        tab: networkBrowserPage
      }
          
      PlasmaComponents.TabButton {
        id: sharesTabButton
        text: i18n("Mounted Shares")
        iconSource: "folder-network-symbolic"
        tab: sharesViewPage
      }
        
      PlasmaComponents.TabButton {
        id: bookmarkTabButton
        text: i18n("Bookmarks")
        iconSource: "bookmarks"
        tab: bookmarksPage
      }
        
      PlasmaComponents.TabButton {
        id: profilesTabButton
        text: i18n("Profiles")
        iconSource: "format-list-unordered"
        tab: profilesPage
      }
      
      PlasmaComponents.TabButton {
        id: configurationTabButton
        text: i18n("Configuration")
        iconSource: "configure"
        tab: configurationPage
      }
    }
      
    //
    // Tab group
    //
    PlasmaComponents.TabGroup {
      id: tabGroup
      
      Layout.fillWidth: true
      Layout.fillHeight: true
      
      currentTab: networkBrowserPage
        
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
    
//     //
//     // Information
//     //
//     PlasmaComponents.Label {
//       id: infoLabel
//       
//       Layout.fillWidth: true
//       Layout.fillHeight: false
//       
//       color: "red"
//       text: "lalala"
//     }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onBusy: busy()
    onIdle: idle()
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
