import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.AppCard {
    id: root

    property var entries: []

    LV.VStack {
        spacing: LV.Theme.gap6
        width: parent ? parent.width : implicitWidth

        Repeater {
            model: root.entries

            delegate: LV.ListItem {
                required property var modelData

                detail: ""
                enabled: false
                iconName: ""
                label: String(modelData)
                listBackgroundColor: LV.Theme.surfaceGhost
                rowHeight: LV.Theme.controlHeightMd
                showChevron: false
            }
        }
        LV.Spacer {
        }
    }
}
