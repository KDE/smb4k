/***************************************************************************
    sharesviewpage.qml - The shares view of Smb4K's plasmoid
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
  id: sharesPage
  
  property NetworkObject currentObject: NetworkObject{}
  
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
          currentObject = sharesView.model.get( index ).object
          mountedShareClicked()
        }
        onUnmountClicked: {
          currentObject = sharesView.model.get( index ).object
          unmountShare()
        }
      }
      model: ListModel {}
      focus: true
      highlightRangeMode: GridView.StrictlyEnforceRange
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: mounter
    onMounted: addMountedShares()
    onUnmounted: removeUnmountedShares()
  }
  
  //
  // Add mounted shares to the shares view
  //
  function addMountedShares() {
    if ( mounter.mountedShares.length != 0 ) {
      for ( var i = 0; i < mounter.mountedShares.length; ++i ) {
        var have_item = false
        if ( sharesView.model.count != 0 ) {
          for ( var j = 0; j < sharesView.model.count; ++j ) {
            if ( sharesView.model.get( j ).object.url == mounter.mountedShares[i].url ) {
              have_item = true
              break
            }
            else {
              // Do nothing
            }
          }
        }

        if ( !have_item ) {
          sharesView.model.append( { "object": mounter.mountedShares[i] } )
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
        var object = mounter.find( sharesView.model.get( i ).object.url )
        
        if ( !object || !object.isMounted ) {
          obsolete_shares.push( sharesView.model.get( i ).object.url.toString() )
        }
        else {
          // Do nothing
        }
      }

      if ( obsolete_shares.length != 0 ) {
        for ( var i = 0; i < obsolete_shares.length; i++ ) {
          for ( var j = 0; j < sharesView.model.count; j++ ) {
            if ( obsolete_shares[i] == sharesView.model.get( j ).object.url.toString() ) {
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
  }
  
  //
  // A mounted share was clicked
  //
  function mountedShareClicked() {
    Qt.openUrlExternally( currentObject.mountpoint )
  }
  
  //
  // A mounted share is to be unmounted
  //
  function unmountShare() {
    mounter.unmount( currentObject.mountpoint )
  }
}
