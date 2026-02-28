import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 2

    NavigationIconButton {
        iconToken: "pin"
    }
    NavigationIconButton {
        iconToken: "toolwindownotifications"
    }
    NavigationIconButton {
        iconToken: "startTimer"
    }
}
