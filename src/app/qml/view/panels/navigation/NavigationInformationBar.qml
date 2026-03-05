import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    property bool compactMode: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationInformationBar") : null

    spacing: 4

    LV.IconButton {
        id: sidebarControlButton

        checkable: false
        iconName: "columnIndex"
        visible: !informationBar.compactMode
    }
    LV.IconButton {
        id: profileButton

        checkable: false
        iconName: "loggedInUser"
    }
    LV.IconButton {
        id: syncButton

        checkable: false
        iconName: "syncFiles"
        visible: !informationBar.compactMode
    }
}
