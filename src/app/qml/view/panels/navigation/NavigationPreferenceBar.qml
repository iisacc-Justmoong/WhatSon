import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: preferenceBar

    spacing: 2

    LV.IconButton {
        id: preferenceButton

        height: 20
        iconName: "audioToAudio"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: detailPanelControlButton

        height: 20
        iconName: "columnIndex"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
