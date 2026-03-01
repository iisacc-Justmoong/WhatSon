import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: statusBar

    property int compactBottomInset: compactHorizontalInset
    property int compactContainerRadius: LV.Theme.radiusXl * 2
    property color compactFieldColor: LV.Theme.panelBackground10
    property int compactFieldHeight: LV.Theme.gap18
    property int compactFieldRadius: Math.round(compactFieldHeight / 2)
    property int compactHorizontalInset: Math.max(LV.Theme.gap12, Math.min(LV.Theme.gap24, Math.round(width * 0.04)))
    property bool compactMode: false
    property int compactNewFileSlotWidth: LV.Theme.controlHeightMd
    property int compactToolbarHeight: LV.Theme.gap20
    property string compactToolbarText: ""
    readonly property int effectivePanelHeight: compactMode ? (compactBottomInset + compactToolbarHeight) : panelHeight
    property color panelColor: LV.Theme.panelBackground06
    property int panelHeight: LV.Theme.controlHeightMd
    readonly property color searchFieldColor: LV.Theme.panelBackground10
    readonly property int searchFieldHeight: LV.Theme.gap18
    readonly property int searchFieldHorizontalInset: LV.Theme.gap24
    readonly property int searchFieldMaxWidth: 541
    readonly property int searchFieldMinWidth: 220
    property int searchFieldRadius: 5
    readonly property int searchFieldWidth: {
        var availableWidth = Math.max(0, width - searchFieldHorizontalInset * 2);
        if (availableWidth < searchFieldMinWidth)
            return availableWidth;
        return Math.min(searchFieldMaxWidth, availableWidth);
    }
    readonly property color searchHintColor: LV.Theme.descriptionColor
    readonly property color searchInputColor: LV.Theme.titleHeaderColor
    property string searchText: ""

    signal searchSubmitted(string text)
    signal searchTextEdited(string text)
    signal windowMoveRequested

    Layout.fillWidth: true
    Layout.preferredHeight: statusBar.effectivePanelHeight
    clip: true
    color: statusBar.compactMode ? LV.Theme.accentTransparent : statusBar.panelColor

    Component.onCompleted: {
        if (!statusBar.compactMode)
            return;
        if (statusBar.searchText.length === 0 && statusBar.compactToolbarText.length > 0)
            statusBar.searchText = statusBar.compactToolbarText;
    }

    Rectangle {
        id: searchBarTextField

        anchors.centerIn: parent
        color: statusBar.searchFieldColor
        height: statusBar.searchFieldHeight
        radius: statusBar.searchFieldRadius
        visible: !statusBar.compactMode
        width: statusBar.searchFieldWidth

        LV.InputField {
            id: searchBarInput

            anchors.fill: parent
            backgroundColor: statusBar.searchFieldColor
            backgroundColorDisabled: statusBar.searchFieldColor
            backgroundColorFocused: statusBar.searchFieldColor
            backgroundColorHover: statusBar.searchFieldColor
            backgroundColorPressed: statusBar.searchFieldColor
            clearButtonVisible: true
            cornerRadius: statusBar.searchFieldRadius
            fieldMinHeight: statusBar.searchFieldHeight
            insetHorizontal: LV.Theme.gap7
            insetVertical: LV.Theme.gap3
            mode: searchMode
            selectByMouse: true
            sideSpacing: LV.Theme.gap5
            text: statusBar.searchText
            textColor: statusBar.searchInputColor

            onAccepted: function (text) {
                var nextText = typeof text === "string" ? text : searchBarInput.text;
                statusBar.searchText = nextText;
                statusBar.searchSubmitted(nextText);
            }
            onTextEdited: function (text) {
                var nextText = typeof text === "string" ? text : searchBarInput.text;
                statusBar.searchText = nextText;
                statusBar.searchTextEdited(nextText);
            }
        }
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
        color: LV.Theme.accentTransparent
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
            spacing: LV.Theme.gapNone

            Rectangle {
                id: compactSearchTextField

                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                color: statusBar.compactFieldColor
                height: statusBar.compactFieldHeight
                radius: statusBar.compactFieldRadius

                LV.InputField {
                    id: compactSearchInput

                    anchors.fill: parent
                    backgroundColor: statusBar.compactFieldColor
                    backgroundColorDisabled: statusBar.compactFieldColor
                    backgroundColorFocused: statusBar.compactFieldColor
                    backgroundColorHover: statusBar.compactFieldColor
                    backgroundColorPressed: statusBar.compactFieldColor
                    clearButtonVisible: true
                    cornerRadius: statusBar.compactFieldRadius
                    fieldMinHeight: statusBar.compactFieldHeight
                    insetHorizontal: LV.Theme.gap7
                    insetVertical: LV.Theme.gap3
                    mode: searchMode
                    selectByMouse: true
                    sideSpacing: LV.Theme.gap5
                    text: statusBar.searchText
                    textColor: statusBar.searchInputColor

                    onAccepted: function (text) {
                        var nextText = typeof text === "string" ? text : compactSearchInput.text;
                        statusBar.searchText = nextText;
                        statusBar.searchSubmitted(nextText);
                    }
                    onTextEdited: function (text) {
                        var nextText = typeof text === "string" ? text : compactSearchInput.text;
                        statusBar.searchText = nextText;
                        statusBar.searchTextEdited(nextText);
                    }
                }
            }
            Item {
                id: newFileButtonSlot

                Layout.fillHeight: true
                Layout.preferredWidth: statusBar.compactNewFileSlotWidth

                LV.IconButton {
                    id: newFileButton

                    anchors.centerIn: parent
                    checkable: false
                    height: LV.Theme.gap20
                    iconName: "addFile"
                    iconSize: LV.Theme.iconSm
                    tone: LV.AbstractButton.Borderless
                    width: LV.Theme.gap20
                }
            }
        }
    }
}
