import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: root

    spacing: 2

    NavigationIconButton {
        iconToken: "toolwindowtodo"
    }
    NavigationIconButton {
        iconToken: "toolWindowClock"
    }
    NavigationIconButton {
        iconToken: "structureListView"
    }
    NavigationIconButton {
        iconToken: "bookmarksbookmarksList"
    }
    NavigationIconButton {
        iconToken: "generalprojectStructure"
    }
}
