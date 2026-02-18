import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: root

    signal createFileRequested
    signal selectFileRequested

    Rectangle {
        anchors.fill: parent
        color: "#1f242d"
    }
    Item {
        anchors.centerIn: parent
        height: onboardingColumn.implicitHeight
        width: Math.min(parent.width - LV.Theme.gap24 * 2, 560)

        LV.VStack {
            id: onboardingColumn

            anchors.fill: parent
            spacing: LV.Theme.gap10

            LV.Label {
                style: title
                text: "Onboarding"
                width: parent ? parent.width : implicitWidth
            }
            LV.Label {
                style: description
                text: "Choose one option to start: create a new file or select an existing file."
                width: parent ? parent.width : implicitWidth
                wrapMode: Text.WordWrap
            }
            LV.AppCard {
                Layout.fillWidth: true
                subtitle: "Start from an empty template"
                title: "Create File"

                LV.LabelButton {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "Create File"
                    tone: LV.AbstractButton.Primary

                    onClicked: root.createFileRequested()
                }
            }
            LV.AppCard {
                Layout.fillWidth: true
                subtitle: "Open an existing document"
                title: "Select File"

                LV.LabelButton {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    text: "Select File"
                    tone: LV.AbstractButton.Default

                    onClicked: root.selectFileRequested()
                }
            }
        }
    }
}
