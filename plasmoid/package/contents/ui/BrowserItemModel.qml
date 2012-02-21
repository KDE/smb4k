import QtQuick 1.1

ListModel {
  id: listModel
  ListElement {
    name: "WORKGROUP1"
    comment: "Some Workgroup"
    icon: "network-workgroup"
    ip: ""
    url: "smb://WORKGROUP1"
  }
  ListElement {
    name: "WORKGROUP2"
    comment: "Another Workgroup"
    icon: "network-workgroup"
    ip: ""
    url: "smb://WORKGROUP2"
  }
}