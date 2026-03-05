import QtQuick
import LVRS 1.0 as LV

LV.IconButton {
    id: navigationIconButton

    property string iconToken: ""
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationIconButton") : null

    checkable: false
    height: 20
    iconName: navigationIconButton.iconToken
    iconSize: 16
    tone: LV.AbstractButton.Borderless
    width: 20
}
