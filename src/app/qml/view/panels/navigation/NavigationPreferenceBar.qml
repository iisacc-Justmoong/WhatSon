import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 2

    LV.IconButton {
        height: 20
        iconName: "audioToAudio"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "columnIndex"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
