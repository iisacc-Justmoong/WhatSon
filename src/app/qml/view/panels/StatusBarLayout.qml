import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

Rectangle {
    id: statusBar

    property int compactBottomInset: compactHorizontalInset
    property int compactContainerRadius: LV.Theme.radiusXl * 2
    property color compactFieldColor: LV.Theme.panelBackground10
    property int compactFieldHeight: LV.Theme.gap18
    property int compactFieldRadius: searchFieldRadiusOverride
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
    property int searchFieldRadius: searchFieldRadiusOverride
    property int searchFieldRadiusOverride: 5
    readonly property int searchFieldWidth: {
        var availableWidth = Math.max(0, width - searchFieldHorizontalInset * 2);
        if (availableWidth < searchFieldMinWidth)
            return availableWidth;
        return Math.min(searchFieldMaxWidth, availableWidth);
    }
    readonly property color searchHintColor: LV.Theme.descriptionColor
    readonly property color searchInputColor: LV.Theme.titleHeaderColor
    property string searchPlaceholder: "Search"

    signal windowMoveRequested

    Layout.fillWidth: true
    Layout.preferredHeight: statusBar.effectivePanelHeight
    clip: true
    color: statusBar.compactMode ? LV.Theme.accentTransparent : statusBar.panelColor

    Component.onCompleted: {
        if (!statusBar.compactMode)
            return;
        if (compactSearchTextInput.text.length === 0 && statusBar.compactToolbarText.length > 0)
            compactSearchTextInput.text = statusBar.compactToolbarText;
        compactSearchTextInput.cursorPosition = compactSearchTextInput.text.length;
    }

    Rectangle {
        id: searchBarTextField

        anchors.centerIn: parent
        color: statusBar.searchFieldColor
        height: statusBar.searchFieldHeight
        radius: statusBar.searchFieldRadius
        visible: !statusBar.compactMode
        width: statusBar.searchFieldWidth

        TextInput {
            id: searchBarTextInput

            anchors.bottomMargin: LV.Theme.gap3
            anchors.fill: parent
            anchors.leftMargin: LV.Theme.gap7
            anchors.rightMargin: LV.Theme.gap7
            anchors.topMargin: LV.Theme.gap3
            clip: true
            color: statusBar.searchInputColor
            font.pixelSize: LV.Theme.textBody
            font.weight: LV.Theme.textBodyWeight
            renderType: Text.NativeRendering
            selectByMouse: true
            selectionColor: LV.Theme.accentBlue
            verticalAlignment: TextInput.AlignVCenter
        }
        Text {
            anchors.left: searchBarTextInput.left
            anchors.verticalCenter: parent.verticalCenter
            color: statusBar.searchHintColor
            font.pixelSize: LV.Theme.textBody
            font.weight: LV.Theme.textBodyWeight
            text: statusBar.searchPlaceholder
            visible: searchBarTextInput.text.length === 0 && !searchBarTextInput.activeFocus
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

                TextInput {
                    id: compactSearchTextInput

                    anchors.bottomMargin: LV.Theme.gap3
                    anchors.fill: parent
                    anchors.leftMargin: LV.Theme.gap7
                    anchors.rightMargin: LV.Theme.gap7
                    anchors.topMargin: LV.Theme.gap3
                    clip: true
                    color: statusBar.searchInputColor
                    font.pixelSize: LV.Theme.textBody
                    font.weight: LV.Theme.textBodyWeight
                    renderType: Text.NativeRendering
                    selectByMouse: true
                    selectionColor: LV.Theme.accentBlue
                    verticalAlignment: TextInput.AlignVCenter
                }
                Text {
                    anchors.left: compactSearchTextInput.left
                    anchors.verticalCenter: parent.verticalCenter
                    color: statusBar.searchHintColor
                    font.pixelSize: LV.Theme.textBody
                    font.weight: LV.Theme.textBodyWeight
                    text: statusBar.compactToolbarText
                    visible: compactSearchTextInput.text.length === 0 && !compactSearchTextInput.activeFocus
                }
            }
            Item {
                id: newFileButtonSlot

                Layout.fillHeight: true
                Layout.preferredWidth: statusBar.compactNewFileSlotWidth

                LV.IconButton {
                    id: newFileButton

                    anchors.centerIn: parent
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
