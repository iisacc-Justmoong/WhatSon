import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: root

    property color panelColor: "#262728"
    property int panelHeight: 36
    readonly property color searchFieldColor: "#343536"
    readonly property int searchFieldHeight: 18
    readonly property int searchFieldHorizontalInset: 24
    readonly property int searchFieldMaxWidth: 541
    readonly property int searchFieldMinWidth: 220
    readonly property int searchFieldWidth: {
        var availableWidth = Math.max(0, width - searchFieldHorizontalInset * 2);
        if (availableWidth < searchFieldMinWidth)
            return availableWidth;
        return Math.min(searchFieldMaxWidth, availableWidth);
    }
    readonly property color searchHintColor: "#9da0a8"
    readonly property color searchInputColor: Qt.rgba(1, 1, 1, 0.9)
    property string searchPlaceholder: "Placeholder"

    Layout.fillWidth: true
    Layout.preferredHeight: root.panelHeight
    clip: true
    color: root.panelColor

    LV.HStack {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
        }
        LV.InputField {
            id: searchField

            Layout.alignment: Qt.AlignVCenter
            Layout.preferredHeight: root.searchFieldHeight
            Layout.preferredWidth: root.searchFieldWidth
            backgroundColor: root.searchFieldColor
            backgroundColorDisabled: root.searchFieldColor
            backgroundColorFocused: root.searchFieldColor
            clearButtonVisible: false
            cornerRadius: 5
            fieldMinHeight: root.searchFieldHeight
            insetHorizontal: 7
            insetVertical: 3
            mode: defaultMode
            placeholder: root.searchPlaceholder
            placeholderColor: root.searchHintColor
            sideSpacing: 6
            textColor: root.searchInputColor

            trailingItems: LV.Label {
                color: root.searchHintColor
                style: description
                text: "􀁡"
                verticalAlignment: Text.AlignVCenter
            }
        }
        Item {
            Layout.fillWidth: true
        }
    }
}
