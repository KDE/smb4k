import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

import "plasmapackage:/plugin" as Smb4K


Item {
  id: mainwindow
  property int minimumWidth: 300
  property int minimumHeight: 200
  
  Component {
    id: browserItemDelegate
    Item {
      width: browserListView.width
      height: 40
      Row {
        spacing: 10
        Column {
          Image { source: icon }
        }
        Column {
          Text { text: "<b>"+name+"</b>" }
          Text { text: "<font size=\"-1\">"+comment+"</font>" }
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
  
  function itemClicked() {
      Smb4K.Scanner
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
      
        QGraphicsWidget {}
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
      model: BrowserItemModel {}
      delegate: browserItemDelegate
      focus: true
    }
  }
}