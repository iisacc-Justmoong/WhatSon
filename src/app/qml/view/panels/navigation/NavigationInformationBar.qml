import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 4

    NavigationIconButton {
        iconToken: "generalprojectStructure"
    }
    NavigationIconButton {
        iconToken: "loggedInUser"
    }
    NavigationIconButton {
        iconToken: "syncFiles"
    }
}
