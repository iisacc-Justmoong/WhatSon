import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: statusBar

    property int compactBottomInset: compactHorizontalInset
    property int compactContainerRadius: 32
    property color compactFieldColor: LV.Theme.panelBackground10
    property int compactFieldHeight: 18
    property int compactFieldRadius: 5
    property int compactHorizontalInset: Math.max(12, Math.min(24, Math.round(width * 0.04)))
    property bool compactMode: false
    property int compactNewFileSlotWidth: 36
    property int compactToolbarHeight: 20
    property string compactToolbarText: ""
    readonly property int effectivePanelHeight: compactMode ? (compactBottomInset + compactToolbarHeight) : panelHeight
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
    property string searchPlaceholder: "Search"

    signal windowMoveRequested

    Layout.fillWidth: true
    Layout.preferredHeight: statusBar.effectivePanelHeight
    clip: true
    color: statusBar.compactMode ? "transparent" : statusBar.panelColor

    Component.onCompleted: {
        if (!statusBar.compactMode)
            return;
        compactSearchTextField.cursorPosition = compactSearchTextField.text.length;
    }

    LV.InputField {
        id: searchBarTextField

        anchors.centerIn: parent
        backgroundColor: statusBar.searchFieldColor
        backgroundColorDisabled: statusBar.searchFieldColor
        backgroundColorFocused: statusBar.searchFieldColor
        clearButtonVisible: true
        cornerRadius: statusBar.searchFieldRadius
        fieldMinHeight: statusBar.searchFieldHeight
        height: statusBar.searchFieldHeight
        insetHorizontal: 7
        insetVertical: 3
        mode: searchMode
        placeholder: statusBar.searchPlaceholder
        placeholderColor: statusBar.searchHintColor
        sideSpacing: 6
        textColor: statusBar.searchInputColor
        visible: !statusBar.compactMode
        width: statusBar.searchFieldWidth
    }
    Item {
        id: windowControlsHitArea

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: searchBarTextField.left
        anchors.top: parent.top
        visible: !statusBar.compactMode

        MouseArea {
            acceptedButtons: Qt.LeftButton
            anchors.fill: parent

            onPressed: function (mouse) {
                if (mouse.button === Qt.LeftButton)
                    statusBar.windowMoveRequested();
            }
        }
    }
    Item {
        id: windowControlIconsHitArea

        anchors.bottom: parent.bottom
        anchors.left: searchBarTextField.right
        anchors.right: parent.right
        anchors.top: parent.top
        visible: !statusBar.compactMode

        MouseArea {
            acceptedButtons: Qt.LeftButton
            anchors.fill: parent

            onPressed: function (mouse) {
                if (mouse.button === Qt.LeftButton)
                    statusBar.windowMoveRequested();
            }
        }
    }
    Rectangle {
        id: compactStatusBar

        anchors.fill: parent
        clip: true
        color: "transparent"
        radius: statusBar.compactContainerRadius
        visible: statusBar.compactMode

        LV.HStack {
            id: compactSearchBar

            anchors.bottom: parent.bottom
            anchors.bottomMargin: statusBar.compactBottomInset
            anchors.left: parent.left
            anchors.leftMargin: statusBar.compactHorizontalInset
            anchors.right: parent.right
            anchors.rightMargin: statusBar.compactHorizontalInset
            anchors.top: parent.top
            spacing: 0

            LV.InputField {
                id: compactSearchTextField

                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                backgroundColor: statusBar.compactFieldColor
                backgroundColorDisabled: statusBar.compactFieldColor
                backgroundColorFocused: statusBar.compactFieldColor
                clearButtonVisible: true
                cornerRadius: statusBar.compactFieldRadius
                fieldMinHeight: statusBar.compactFieldHeight
                height: statusBar.compactFieldHeight
                insetHorizontal: 7
                insetVertical: 3
                placeholder: ""
                sideSpacing: 6
                text: statusBar.compactToolbarText
            }
            Item {
                id: newFileButtonSlot

                Layout.fillHeight: true
                Layout.preferredWidth: statusBar.compactNewFileSlotWidth

                LV.IconButton {
                    id: newFileButton

                    anchors.centerIn: parent
                    height: 20
                    iconName: "addFile"
                    iconSize: 16
                    tone: LV.AbstractButton.Borderless
                    width: 20
                }
            }
        }
    }
}
