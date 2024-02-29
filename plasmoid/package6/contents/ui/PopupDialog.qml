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
    PlasmaComponents.TabBar {
      id: tabBar

      Layout.maximumWidth: parent.width
      Layout.fillWidth: true
      Layout.fillHeight: false

      PlasmaComponents.TabButton {
        id: browserTabButton
        text: i18n("Network Neighborhood")
        icon.name: "network-workgroup-symbolic"
        // width: implicitWidth
      }

      PlasmaComponents.TabButton {
        id: sharesTabButton
        text: i18n("Mounted Shares")
        icon.name: "folder-network-symbolic"
        // width: implicitWidth
      }

      PlasmaComponents.TabButton {
        id: bookmarkTabButton
        text: i18n("Bookmarks")
        icon.name: "bookmarks"
        // width: implicitWidth
      }

      PlasmaComponents.TabButton {
        id: profilesTabButton
        text: i18n("Profiles")
        icon.name: "preferences-system-users"
        // width: implicitWidth
      }

      PlasmaComponents.TabButton {
        id: configurationTabButton
        text: i18n("Configuration")
        icon.name: "configure"
        // width: implicitWidth
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
