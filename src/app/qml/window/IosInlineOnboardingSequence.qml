pragma ComponentBehavior: Bound

import QtQuick
import LVRS 1.0 as LV
import "." as WindowView

Item {
    id: root

    property color canvasColor: LV.Theme.panelBackground01
    property var hostWindow: null
    property var hubSessionController: null
    property color panelColor: LV.Theme.panelBackground06
    property color sidePanelColor: LV.Theme.panelBackground10
    property string versionText: "Version: 1.0.0"

    signal dismissRequested
    signal viewHookRequested

    clip: true

    Rectangle {
        anchors.fill: parent
        color: root.canvasColor
    }

    WindowView.OnboardingContent {
        anchors.fill: parent
        autoCompleteOnHubLoaded: false
        hostWindow: root.hostWindow
        hubSessionController: root.hubSessionController
        panelColor: root.panelColor
        sidePanelColor: root.sidePanelColor
        standaloneMode: false
        versionText: root.versionText

        onDismissRequested: root.dismissRequested()
        onViewHookRequested: root.viewHookRequested()
    }
}
