import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import LVRS 1.0 as LV

Window {
    id: root

    property var hostWindow: null
    readonly property string visualizeModes: "clip,batches,changes,overdraw"

    color: LV.Theme.panelBackground03
    height: LV.Theme.gap20 * 16
    minimumHeight: LV.Theme.gap20 * 10
    minimumWidth: LV.Theme.gap20 * 18
    title: "WhatSon Scene Debug"
    visible: false
    width: LV.Theme.gap20 * 28

    Rectangle {
        anchors.fill: parent
        color: root.color
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: LV.Theme.gap6
        spacing: LV.Theme.gap4

        LV.Label {
            Layout.fillWidth: true
            text: "Scene graph visualization enabled"
            style: header3
        }
        LV.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "QSG_VISUALIZE=" + root.visualizeModes
        }
        LV.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "QSG_RHI_BACKEND=opengl"
        }
        LV.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: root.hostWindow ? "Main window connected" : "Main window not connected"
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: LV.Theme.gap2
            color: LV.Theme.panelBackground10

            LV.Label {
                anchors.fill: parent
                anchors.margins: LV.Theme.gap4
                wrapMode: Text.WordWrap
                text: "Look at the main WhatSon window. Qt Quick should now overlay rendering diagnostics for clipping, batches, changes, and overdraw."
            }
        }
    }
}
