import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: listBarHeader

    readonly property color inlineFieldBackgroundColor: "transparent"
    readonly property int inlineFieldHeight: 18
    readonly property int inlineFieldHorizontalInset: 7
    readonly property int inlineFieldTextHeight: 12
    readonly property int inlineFieldVerticalInset: 3
    property string searchText: ""

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal sortActionRequested
    signal visibilityActionRequested

    implicitHeight: 24
    implicitWidth: 198

    RowLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 0

        LV.InputField {
            id: searchField

            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.preferredHeight: listBarHeader.inlineFieldHeight
            backgroundColor: listBarHeader.inlineFieldBackgroundColor
            backgroundColorDisabled: listBarHeader.inlineFieldBackgroundColor
            backgroundColorFocused: listBarHeader.inlineFieldBackgroundColor
            backgroundColorHover: listBarHeader.inlineFieldBackgroundColor
            backgroundColorPressed: listBarHeader.inlineFieldBackgroundColor
            centeredTextHeight: listBarHeader.inlineFieldTextHeight
            clearButtonVisible: false
            fieldMinHeight: listBarHeader.inlineFieldHeight
            insetHorizontal: listBarHeader.inlineFieldHorizontalInset
            insetVertical: listBarHeader.inlineFieldVerticalInset
            mode: searchMode
            selectByMouse: true
            shapeStyle: shapeCylinder
            text: listBarHeader.searchText

            onAccepted: function (text) {
                const nextText = typeof text === "string" ? text : searchField.text;
                if (listBarHeader.searchText !== nextText)
                    listBarHeader.searchText = nextText;
                listBarHeader.searchSubmitted(nextText);
            }
            onTextEdited: function (text) {
                const nextText = typeof text === "string" ? text : searchField.text;
                if (listBarHeader.searchText !== nextText)
                    listBarHeader.searchText = nextText;
                listBarHeader.searchTextEdited(nextText);
            }
        }
        LV.IconButton {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 20
            iconName: "cwmPermissionView"

            onClicked: listBarHeader.visibilityActionRequested()
        }
        LV.IconButton {
            Layout.preferredHeight: 20
            Layout.preferredWidth: 20
            iconName: "sortByType"

            onClicked: listBarHeader.sortActionRequested()
        }
    }
}
