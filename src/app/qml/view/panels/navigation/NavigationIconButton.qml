import QtQuick
import LVRS 1.0 as LV

LV.IconButton {
    id: root

    property string iconToken: ""

    height: 20
    iconName: root.iconToken
    iconSize: 16
    tone: LV.AbstractButton.Borderless
    width: 20
}
