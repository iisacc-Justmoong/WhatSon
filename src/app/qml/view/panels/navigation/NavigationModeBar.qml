import QtQuick
import QtQuick.Layouts
import LVRS 1.0 as LV

LV.HStack {
    id: modeBar

    readonly property string activeModeText: activeNavigationModeViewModel && activeNavigationModeViewModel.modeName !== undefined ? activeNavigationModeViewModel.modeName : "Control"
    readonly property var activeNavigationModeViewModel: navigationModeViewModel && navigationModeViewModel.activeModeViewModel !== undefined ? navigationModeViewModel.activeModeViewModel : null
    property var navigationModeViewModel: null
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.NavigationModeBar") : null

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
        text: "Mode"
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
                text: modeBar.activeModeText
            }
            Item {
                Layout.fillWidth: true
            }
            LV.Stepper {
                Layout.alignment: Qt.AlignVCenter
                arrow: LV.Stepper.UpDown
                tone: LV.AbstractButton.Primary

                onClicked: {
                    if (modeBar.navigationModeViewModel && modeBar.navigationModeViewModel.requestNextMode !== undefined)
                        modeBar.navigationModeViewModel.requestNextMode();
                    modeBar.requestViewHook("next-navigation-mode");
                }
            }
        }
    }
}
