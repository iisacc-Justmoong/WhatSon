import QtQuick
import LVRS 1.0 as LV

LV.HStack {
    id: viewOptionBar

    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("navigation.view.NavigationApplicationViewOptionBar") : null

    signal viewHookRequested(string reason)

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested(hookReason);
    }

    spacing: LV.Theme.gap2

    LV.IconButton {
        id: readOnlyToggle

        horizontalPadding: LV.Theme.gap2
        iconName: "readerMode"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: viewOptionBar.requestViewHook("view-toggle-read-only")
    }
    LV.IconButton {
        id: warpText

        horizontalPadding: LV.Theme.gap2
        iconName: "textAutoGenerate"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: viewOptionBar.requestViewHook("view-toggle-wrap-text")
    }
    LV.IconButton {
        id: centerViewOptionButton

        horizontalPadding: LV.Theme.gap2
        iconName: "recursiveMethod"
        tone: LV.AbstractButton.Borderless
        verticalPadding: LV.Theme.gap2

        onClicked: viewOptionBar.requestViewHook("view-option-center-view")
    }
    LV.IconMenuButton {
        id: textToSpeech

        bottomPadding: LV.Theme.gap2
        iconName: "textToSpeech"
        leftPadding: LV.Theme.gap2
        rightPadding: LV.Theme.gap4
        spacing: LV.Theme.gapNone
        tone: LV.AbstractButton.Borderless
        topPadding: LV.Theme.gap2

        onClicked: viewOptionBar.requestViewHook("view-open-text-to-speech-options")
    }
    LV.IconMenuButton {
        id: paperOption

        bottomPadding: LV.Theme.gap2
        iconName: "fileFormat"
        leftPadding: LV.Theme.gap2
        rightPadding: LV.Theme.gap4
        spacing: LV.Theme.gapNone
        tone: LV.AbstractButton.Borderless
        topPadding: LV.Theme.gap2

        onClicked: viewOptionBar.requestViewHook("view-open-paper-options")
    }
}
