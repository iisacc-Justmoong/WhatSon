import QtQuick
import QtQuick.Controls as Controls
import LVRS 1.0 as LV

LV.HStack {
    id: editorViewBar

    readonly property var activeEditorViewModeViewModel: editorViewModeViewModel && editorViewModeViewModel.activeViewModeViewModel !== undefined ? editorViewModeViewModel.activeViewModeViewModel : null
    readonly property string activeViewText: activeEditorViewModeViewModel && activeEditorViewModeViewModel.editorViewName !== undefined ? activeEditorViewModeViewModel.editorViewName : "Plain"
    readonly property int comboLabelRightInset: 20
    readonly property var editorViewMenuItems: [
        {
            label: "Plain",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 0
        },
        {
            label: "Page",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 1
        },
        {
            label: "Print",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 2
        },
        {
            label: "Web",
            selected: editorViewModeViewModel && editorViewModeViewModel.activeViewMode === 3
        }
    ]
    property var editorViewModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationEditorViewBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }
    function toggleEditorViewMenu() {
        if (editorViewContextMenu.opened) {
            editorViewContextMenu.close();
            return;
        }
        editorViewContextMenu.openFor(editorViewComboFrame, 0, editorViewComboFrame.height + 2);
        editorViewBar.requestViewHook("open-editor-view-menu");
    }

    spacing: 8

    Text {
        color: Qt.rgba(1, 1, 1, 0.8)
        font.family: "Pretendard"
        font.pixelSize: 12
        font.weight: 500
        lineHeight: 12
        lineHeightMode: Text.FixedHeight
        text: "View"
    }
    Item {
        id: editorViewComboFrame

        height: 20
        implicitHeight: 20
        implicitWidth: 97
        width: 97

        LV.ComboBox {
            id: editorViewCombo

            anchors.fill: parent
            arrow: editorViewContextMenu.opened ? LV.Stepper.Up : LV.Stepper.Down
            tone: LV.ComboBox.Borderless

            onClicked: editorViewBar.toggleEditorViewMenu()
        }
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            anchors.left: parent.left
            anchors.leftMargin: 1
            anchors.right: parent.right
            anchors.rightMargin: editorViewBar.comboLabelRightInset
            anchors.top: parent.top
            anchors.topMargin: 1
            color: editorViewCombo.resolvedBackgroundColor
            radius: LV.Theme.radiusBase
        }
        Text {
            anchors.left: parent.left
            anchors.leftMargin: 8
            anchors.right: parent.right
            anchors.rightMargin: editorViewBar.comboLabelRightInset
            anchors.verticalCenter: parent.verticalCenter
            color: LV.Theme.accentWhite
            elide: Text.ElideRight
            font.family: "Pretendard"
            font.pixelSize: 12
            font.weight: 500
            lineHeight: 12
            lineHeightMode: Text.FixedHeight
            text: editorViewBar.activeViewText
        }
    }
    LV.ContextMenu {
        id: editorViewContextMenu

        autoCloseOnTrigger: true
        closePolicy: Controls.Popup.CloseOnPressOutside | Controls.Popup.CloseOnPressOutsideParent | Controls.Popup.CloseOnEscape
        itemWidth: 108
        items: editorViewBar.editorViewMenuItems
        modal: false
        parent: Controls.Overlay.overlay
        selectedIndex: editorViewModeViewModel && editorViewModeViewModel.activeViewMode !== undefined ? editorViewModeViewModel.activeViewMode : 0

        onItemTriggered: function (index) {
            if (editorViewBar.editorViewModeViewModel && editorViewBar.editorViewModeViewModel.requestViewModeChange !== undefined)
                editorViewBar.editorViewModeViewModel.requestViewModeChange(index);
            editorViewBar.requestViewHook("select-editor-view-mode");
        }
    }
}
