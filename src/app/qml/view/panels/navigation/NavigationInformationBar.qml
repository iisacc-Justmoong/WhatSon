import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 4

    LV.IconButton {
        height: 20
        iconName: "columnIndex"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "loggedInUser"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "syncFiles"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
