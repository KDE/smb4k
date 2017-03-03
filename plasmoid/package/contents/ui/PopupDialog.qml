/***************************************************************************
 *   Copyright (C) 2017 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.2
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

FocusScope {
  id: popupDialog
  anchors.fill: parent
  
  ColumnLayout {
    //
    // Tab bar
    //
    PlasmaComponents.TabBar {
      id: tabBar
      Layout.maximumWidth: popupDialog.width
        
      PlasmaComponents.TabButton {
        id: browserTabButton
        text: i18n("Network Neighborhood")
        iconSource: "network-workgroup-symbolic"
        tab: networkBrowserPage
      }
        
      PlasmaComponents.TabButton {
        id: sharesTabButton
        text: i18n("Mounted Shares")
        iconSource: "folder-network"
        tab: sharesViewPage
      }
      
      PlasmaComponents.TabButton {
        id: bookmarkTabButton
        text: i18n("Bookmarks")
        iconSource: "favorite"
        tab: bookmarksPage
      }
      
      PlasmaComponents.TabButton {
        id: profilesTabButton
        text: i18n("Profiles")
        iconSource: "format-list-unordered"
        tab: profilesPage
      }
    }
    
    //
    // Tab group
    //
    PlasmaComponents.TabGroup {
      id: tabGroup
      currentTab: networkBrowserPage
      Layout.fillHeight: true
      Layout.fillWidth: true
        
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
    }
  }
}
