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
        tone: LV.AbstractButton.Borderless
        visible: !informationBar.compactMode
    }
    LV.IconButton {
        id: profileButton

        checkable: false
        iconName: "loggedInUser"
        tone: LV.AbstractButton.Borderless
    }
    LV.IconButton {
        id: syncButton

        checkable: false
        iconName: "syncFiles"
        tone: LV.AbstractButton.Borderless
        visible: !informationBar.compactMode
    }
}
