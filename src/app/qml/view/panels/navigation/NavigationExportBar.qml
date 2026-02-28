import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 2

    LV.IconButton {
        height: 20
        iconName: "generalupload"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "generalprint"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        height: 20
        iconName: "mailer"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
