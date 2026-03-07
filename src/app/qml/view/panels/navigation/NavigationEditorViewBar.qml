import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: editorViewBar

    readonly property var activeEditorViewModeViewModel: editorViewModeViewModel && editorViewModeViewModel.activeViewModeViewModel !== undefined ? editorViewModeViewModel.activeViewModeViewModel : null
    readonly property string activeViewText: activeEditorViewModeViewModel && activeEditorViewModeViewModel.editorViewName !== undefined ? activeEditorViewModeViewModel.editorViewName : "Plain"
    property var editorViewModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationEditorViewBar") : null

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
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
        height: 20
        implicitHeight: 20
        implicitWidth: 97
        width: 97

        Rectangle {
            anchors.fill: parent
            color: LV.Theme.panelBackground10
            radius: 5
        }
        RowLayout {
            anchors.bottomMargin: 1
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 1
            anchors.topMargin: 1
            spacing: 0

            Text {
                Layout.alignment: Qt.AlignVCenter
                color: LV.Theme.accentWhite
                font.family: "Pretendard"
                font.pixelSize: 12
                font.weight: 500
                lineHeight: 12
                lineHeightMode: Text.FixedHeight
                text: editorViewBar.activeViewText
            }
            Item {
                Layout.fillWidth: true
            }
            LV.Stepper {
                Layout.alignment: Qt.AlignVCenter
                arrow: LV.Stepper.UpDown
                tone: LV.AbstractButton.Borderless

                onClicked: {
                    if (editorViewBar.editorViewModeViewModel && editorViewBar.editorViewModeViewModel.requestNextViewMode !== undefined)
                        editorViewBar.editorViewModeViewModel.requestNextViewMode();
                    editorViewBar.requestViewHook("next-editor-view-mode");
                }
            }
        }
    }
}
