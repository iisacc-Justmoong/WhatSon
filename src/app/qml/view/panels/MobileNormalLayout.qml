import QtQuick
import "../mobile/pages" as MobilePageView

Item {
    id: mobileNormalLayout

    property color canvasColor
    property color controlSurfaceColor
    property var editorViewModeViewModel
    property string hierarchySearchText
    property var navigationModeViewModel
    readonly property var panelViewModel: panelViewModelRegistry ? panelViewModelRegistry.panelViewModel("MobileNormalLayout") : null
    required property var sidebarHierarchyViewModel
    property string statusPlaceholderText
    property string statusSearchText
    property var toolbarIconNames
    property var windowInteractions

    signal viewHookRequested

    function requestViewHook(reason) {
        const hookReason = reason !== undefined ? String(reason) : "manual";
        if (panelViewModel && panelViewModel.requestViewModelHook)
            panelViewModel.requestViewModelHook(hookReason);
        viewHookRequested();
    }

    MobilePageView.MobileHierarchyPage {
        anchors.fill: parent
        canvasColor: mobileNormalLayout.canvasColor
        controlSurfaceColor: mobileNormalLayout.controlSurfaceColor
        editorViewModeViewModel: mobileNormalLayout.editorViewModeViewModel
        hierarchySearchText: mobileNormalLayout.hierarchySearchText
        navigationModeViewModel: mobileNormalLayout.navigationModeViewModel
        sidebarHierarchyViewModel: mobileNormalLayout.sidebarHierarchyViewModel
        statusPlaceholderText: mobileNormalLayout.statusPlaceholderText
        statusSearchText: mobileNormalLayout.statusSearchText
        toolbarIconNames: mobileNormalLayout.toolbarIconNames
        windowInteractions: mobileNormalLayout.windowInteractions

        onViewHookRequested: mobileNormalLayout.requestViewHook("mobile-hierarchy-page")
    }
}
