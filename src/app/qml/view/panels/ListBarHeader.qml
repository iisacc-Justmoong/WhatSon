import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Item {
    id: listBarHeader

    property color inlineFieldBackgroundColor: "transparent"
    readonly property int actionButtonSize: LV.Theme.gap20
    property int frameMinHeight: LV.Theme.gap24
    readonly property int inlineFieldHeight: LV.Theme.gap18
    readonly property int inlineFieldHorizontalInset: LV.Theme.gap7
    readonly property int inlineFieldTextHeight: LV.Theme.gap12
    readonly property int inlineFieldVerticalInset: LV.Theme.gap3
    property int outerHorizontalInset: LV.Theme.gap2
    property int outerVerticalInset: LV.Theme.gap2
    readonly property int shapeCylinder: searchField.shapeCylinder
    readonly property int shapeRoundRect: searchField.shapeRoundRect
    readonly property int resolvedInputTextHeight: Math.max(listBarHeader.inlineFieldTextHeight, Number(searchField && searchField.inputItem && searchField.inputItem.contentHeight !== undefined ? searchField.inputItem.contentHeight : listBarHeader.inlineFieldTextHeight) || listBarHeader.inlineFieldTextHeight)
    property int searchFieldShapeStyle: listBarHeader.shapeCylinder
    property string searchText: ""
    property bool sortActionVisible: true
    property bool visibilityActionVisible: true

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal sortActionRequested
    signal visibilityActionRequested

    implicitHeight: Math.max(listBarHeader.frameMinHeight, headerRow.implicitHeight + listBarHeader.outerVerticalInset * 2)
    implicitWidth: headerRow.implicitWidth + listBarHeader.outerHorizontalInset * 2

    RowLayout {
        id: headerRow

        anchors.fill: parent
        anchors.bottomMargin: listBarHeader.outerVerticalInset
        anchors.leftMargin: listBarHeader.outerHorizontalInset
        anchors.rightMargin: listBarHeader.outerHorizontalInset
        anchors.topMargin: listBarHeader.outerVerticalInset
        spacing: LV.Theme.gapNone

        LV.InputField {
            id: searchField

            readonly property int resolvedInputTextHeight: listBarHeader.resolvedInputTextHeight

            Layout.fillWidth: true
            Layout.minimumWidth: LV.Theme.gapNone
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
            shapeStyle: listBarHeader.searchFieldShapeStyle
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
        Binding {
            property: "height"
            target: searchField.inputItem
            value: searchField.resolvedInputTextHeight
        }
        Binding {
            property: "y"
            target: searchField.inputItem
            value: Math.max(0, Math.floor((searchField.height - searchField.resolvedInputTextHeight) / 2))
        }
        LV.IconButton {
            Layout.preferredHeight: listBarHeader.actionButtonSize
            Layout.preferredWidth: listBarHeader.actionButtonSize
            iconName: "cwmPermissionView"
            visible: listBarHeader.visibilityActionVisible

            onClicked: listBarHeader.visibilityActionRequested()
        }
        LV.IconButton {
            Layout.preferredHeight: listBarHeader.actionButtonSize
            Layout.preferredWidth: listBarHeader.actionButtonSize
            iconName: "sortByType"
            visible: listBarHeader.sortActionVisible

            onClicked: listBarHeader.sortActionRequested()
        }
    }
}
