import QtQuick
import LVRS 1.0 as LV

LV.AppCard {
    id: root

    property string hint: ""
    property string value: ""

    implicitHeight: 148
    subtitle: root.hint

    LV.Label {
        color: LV.Theme.textPrimary
        style: title
        text: root.value
        width: parent ? parent.width : implicitWidth
    }
}
