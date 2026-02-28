import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: exportBar

    spacing: 2

    LV.IconButton {
        id: exportButton

        height: 20
        iconName: "generalupload"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: printButton

        height: 20
        iconName: "generalprint"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
    LV.IconButton {
        id: mailingButton

        height: 20
        iconName: "mailer"
        iconSize: 16
        tone: LV.AbstractButton.Borderless
        width: 20
    }
}
