import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    property bool compactMode: false

    spacing: 4

    LV.IconButton {
        id: sidebarControlButton

        checkable: false
        height: 20
        iconName: "columnIndex"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        visible: !informationBar.compactMode
        width: 20
    }
    LV.IconButton {
        id: profileButton

        checkable: false
        height: 20
        iconName: "loggedInUser"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: syncButton

        checkable: false
        height: 20
        iconName: "syncFiles"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        visible: !informationBar.compactMode
        width: 20
    }
}
