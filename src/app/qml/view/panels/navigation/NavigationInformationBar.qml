import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: informationBar

    property bool compactMode: false
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationInformationBar") : null

    spacing: 4

    LV.IconButton {
        id: sidebarControlButton

        iconName: "columnIndex"
        visible: !informationBar.compactMode
    }
    LV.IconButton {
        id: profileButton

        iconName: "loggedInUser"
    }
    LV.IconButton {
        id: syncButton

        iconName: "syncFiles"
        visible: !informationBar.compactMode
    }
}
