import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: statusBar

    property int compactBottomInset: LV.Theme.gapNone
    property int compactContainerRadius: LV.Theme.radiusXl * 2
    property color compactFieldColor: LV.Theme.panelBackground10
    property int compactFieldHeight: LV.Theme.gap18
    property int compactFieldRadius: LV.Theme.radiusControl
    property int compactHorizontalInset: LV.Theme.gapNone
    property bool compactMode: false
    property int compactNewFileSlotWidth: LV.Theme.controlHeightMd
    property int compactToolbarHeight: LV.Theme.gap20
    property string compactToolbarText: ""
    readonly property int effectivePanelHeight: compactMode ? compactToolbarHeight : panelHeight
    property color panelColor: "transparent"
    property int panelHeight: LV.Theme.controlHeightMd
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("StatusBarLayout") : null
    readonly property color searchFieldColor: "transparent"
    readonly property int searchFieldHeight: LV.Theme.gap18
    readonly property int searchFieldHorizontalInset: LV.Theme.gap24
    readonly property int searchFieldMaxWidth: LV.Theme.scaffoldBlobPrimarySize + LV.Theme.gap20 + Math.round(LV.Theme.strokeThin)
    readonly property int searchFieldMinWidth: LV.Theme.inputWidthMd + LV.Theme.gap14
    property int searchFieldRadius: LV.Theme.radiusControl
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
    signal createNoteRequested
    signal viewHookRequested
    signal windowMoveRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    Layout.fillWidth: true
    Layout.preferredHeight: statusBar.effectivePanelHeight
    clip: true
    color: statusBar.compactMode ? LV.Theme.accentTransparent : statusBar.panelColor

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
            mode: searchMode
            selectByMouse: true
            text: statusBar.searchText

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

            anchors.fill: parent
            spacing: LV.Theme.gapNone

            LV.InputField {
                id: compactSearchInput

                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                Layout.minimumWidth: LV.Theme.gapNone
                Layout.preferredHeight: statusBar.compactFieldHeight
                backgroundColor: statusBar.compactFieldColor
                backgroundColorDisabled: statusBar.compactFieldColor
                backgroundColorFocused: statusBar.compactFieldColor
                backgroundColorHover: statusBar.compactFieldColor
                backgroundColorPressed: statusBar.compactFieldColor
                centeredTextHeight: LV.Theme.gap12
                fieldMinHeight: statusBar.compactFieldHeight
                insetHorizontal: LV.Theme.gap7
                insetVertical: LV.Theme.gap3
                mode: searchMode
                placeholderText: statusBar.compactToolbarText
                selectByMouse: true
                shapeStyle: shapeCylinder
                text: statusBar.searchText

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
            Item {
                id: newFileButtonSlot

                Layout.fillHeight: true
                Layout.preferredWidth: statusBar.compactNewFileSlotWidth

                LV.IconButton {
                    id: newFileButton

                    anchors.centerIn: parent
                    iconName: "addFile"
                    horizontalPadding: LV.Theme.gap2
                    tone: LV.AbstractButton.Borderless
                    verticalPadding: LV.Theme.gap2

                    onClicked: {
                        statusBar.createNoteRequested();
                    }
                }
            }
        }
    }
}
