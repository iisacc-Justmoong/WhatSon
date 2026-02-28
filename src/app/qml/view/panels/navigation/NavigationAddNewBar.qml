import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 0

    LV.IconButton {
        height: 20
        iconName: "addFile"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
