import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV
import "navigation" as NavigationView

Rectangle {
    id: root

    property color panelColor: LV.Theme.panelBackground06
    property int panelHeight: 36

    Layout.fillWidth: true
    Layout.preferredHeight: root.panelHeight
    clip: true
    color: root.panelColor

    Item {
        anchors.bottomMargin: 2
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 2

        LV.HStack {
            anchors.fill: parent
            spacing: 0

            NavigationView.NavigationInformationBar {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 20
            }
            Item {
                Layout.fillWidth: true
            }
            NavigationView.NavigationApplicationContentsBar {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 20
            }
        }
    }
}
