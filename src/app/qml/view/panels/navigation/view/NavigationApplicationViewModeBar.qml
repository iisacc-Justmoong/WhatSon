import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: modeBar

    readonly property var panelController: panelControllerRegistry ? panelControllerRegistry.panelController("navigation.view.NavigationApplicationViewModeBar") : null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelController && panelController.requestControllerHook)
            panelController.requestControllerHook(hookReason);
        viewHookRequested(hookReason);
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: centerView

        horizontalPadding: LV.Theme.gap2
        iconName: "singleRecordView"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: modeBar.requestViewHook("view-mode-center-view")
    }
    LV.IconButton {
        id: focusMode

        horizontalPadding: LV.Theme.gap2
        iconName: "imagefitContent"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: modeBar.requestViewHook("view-mode-focus")
    }
    LV.IconButton {
        id: presentation

        horizontalPadding: LV.Theme.gap2
        iconName: "runshowCurrentFrame"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: modeBar.requestViewHook("view-mode-presentation")
    }
}
