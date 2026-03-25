import QtQuick
import LVRS 1.0 as LV

QtObject {
    id: interactionController

    property var activePageRouter: null
    readonly property string addNewPanelKey: "navigation.NavigationAddNewBar"
    property bool adaptiveDesktopLayout: false
    property string adaptiveLayoutProfile: ""
    property bool adaptiveMobileLayout: false
    property string adaptiveNavigationMode: ""
    property var focusRetainedUiTokens: ["button", "combobox", "checkbox", "radiobutton", "switch", "slider", "spinbox", "dial", "textinput", "textedit", "inputfield", "editor", "menu", "popup", "tooltip", "mousearea", "taphandler", "flickable", "listview", "scrollview", "tableview", "scrollbar", "hierarchy", "contextmenu", "itemdelegate", "notelistitem"]
    property var hostWindow: null
    property int libraryHierarchyIndex: 0
    property string libraryNoteMutationViewId: ""
    readonly property string libraryNoteMutationViewModelKey: "libraryNoteMutationViewModel"
    property string navigationModeViewId: ""
    readonly property string navigationModeViewModelKey: "navigationModeViewModel"
    property var panelViewModelRegistry: null
    property bool resizeDrWasSuspended: false
    property bool resizeInProgress: false
    property int resizeRenderGuardDebounceMs: 220
    property bool resizeRenderGuardEnabled: true
    property string sidebarHierarchyViewId: ""
    readonly property string sidebarHierarchyViewModelKey: "sidebarHierarchyViewModel"

    function applyRenderQualityPolicy(source) {
        if (!interactionController.hostWindow || (!interactionController.hostWindow.isDesktopPlatform && !interactionController.hostWindow.isMobilePlatform))
            return;

        console.log("[whatson:debug][render.policy][" + source + "] platform=" + interactionController.hostWindow.platform + " action=resizeSuspendResumeGuard dynamicResolutionEnabled=" + LV.RenderQuality.dynamicResolutionEnabled);
    }
    function clearActiveFocus(reason) {
        let current = interactionController.hostWindow ? interactionController.hostWindow.activeFocusItem : null;
        let depthGuard = 0;

        while (current && depthGuard < 32) {
            if (current.focus !== undefined)
                current.focus = false;
            current = current.parent;
            depthGuard += 1;
        }
    }
    function resolveOwnedWritableViewModel(viewId, viewModelKey) {
        const normalizedViewId = viewId === undefined || viewId === null ? "" : String(viewId).trim();
        const normalizedViewModelKey = viewModelKey === undefined || viewModelKey === null ? "" : String(viewModelKey).trim();
        if (normalizedViewId.length === 0 || normalizedViewModelKey.length === 0)
            return null;
        if (!LV.ViewModels.canWrite(normalizedViewId))
            return null;
        const ownedViewModel = LV.ViewModels.getForView(normalizedViewId);
        if (ownedViewModel !== null && ownedViewModel !== undefined)
            return ownedViewModel;
        return LV.ViewModels.get(normalizedViewModelKey);
    }
    function resolvePanelViewModel(panelKey) {
        const normalizedPanelKey = panelKey === undefined || panelKey === null ? "" : String(panelKey).trim();
        if (normalizedPanelKey.length === 0
                || !interactionController.panelViewModelRegistry
                || interactionController.panelViewModelRegistry.panelViewModel === undefined)
            return null;
        return interactionController.panelViewModelRegistry.panelViewModel(normalizedPanelKey);
    }
    function resolveLibraryNoteCreationViewModel() {
        return interactionController.resolveOwnedWritableViewModel(
                    interactionController.libraryNoteMutationViewId,
                    interactionController.libraryNoteMutationViewModelKey);
    }
    function createNoteFromShortcut() {
        const addNewPanelViewModel = interactionController.resolvePanelViewModel(interactionController.addNewPanelKey);
        if (addNewPanelViewModel && addNewPanelViewModel.requestViewModelHook !== undefined) {
            addNewPanelViewModel.requestViewModelHook("create-note");
            return true;
        }
        const noteMutationViewModel = interactionController.resolveLibraryNoteCreationViewModel();
        if (!noteMutationViewModel || noteMutationViewModel.createEmptyNote === undefined)
            return false;
        const sidebarHierarchyViewModel = interactionController.resolveOwnedWritableViewModel(
                    interactionController.sidebarHierarchyViewId,
                    interactionController.sidebarHierarchyViewModelKey);
        if (sidebarHierarchyViewModel && sidebarHierarchyViewModel.setActiveHierarchyIndex !== undefined)
            sidebarHierarchyViewModel.setActiveHierarchyIndex(interactionController.libraryHierarchyIndex);
        return Boolean(noteMutationViewModel.createEmptyNote());
    }
    function cycleNavigationModeFromShortcut() {
        if (interactionController.hasFocusedTextInput())
            return;
        const navigationModeViewModel = interactionController.resolveOwnedWritableViewModel(
                    interactionController.navigationModeViewId,
                    interactionController.navigationModeViewModelKey);
        if (navigationModeViewModel && navigationModeViewModel.requestNextMode !== undefined)
            navigationModeViewModel.requestNextMode();
    }
    function finalizeResizeRenderQualityPolicy() {
        if (!interactionController.resizeRenderGuardEnabled || !interactionController.hostWindow || (!interactionController.hostWindow.isMobilePlatform && !interactionController.hostWindow.isDesktopPlatform))
            return;

        interactionController.resizeInProgress = false;
        if (!interactionController.resizeDrWasSuspended) {
            console.log("[whatson:debug][render.policy][resizeEnd] platform=" + interactionController.hostWindow.platform + " action=resumeSkipped");
            return;
        }

        LV.RenderQuality.dynamicResolutionEnabled = false;
        LV.RenderQuality.dynamicResolutionEnabled = true;
        interactionController.resizeDrWasSuspended = false;
        console.log("[whatson:debug][render.policy][resizeEnd] platform=" + interactionController.hostWindow.platform + " action=resumeWithMaxScaleReset");
    }
    function handleResizeForRenderQuality(source) {
        if (!interactionController.resizeRenderGuardEnabled || !interactionController.hostWindow || (!interactionController.hostWindow.isMobilePlatform && !interactionController.hostWindow.isDesktopPlatform))
            return;

        if (!interactionController.resizeInProgress) {
            interactionController.resizeInProgress = true;
            interactionController.resizeDrWasSuspended = LV.RenderQuality.dynamicResolutionEnabled;

            if (interactionController.resizeDrWasSuspended) {
                LV.RenderQuality.dynamicResolutionEnabled = false;
                console.log("[whatson:debug][render.policy][" + source + "] platform=" + interactionController.hostWindow.platform + " action=resizeBegin suspendDynamicResolution=true");
            } else {
                console.log("[whatson:debug][render.policy][" + source + "] platform=" + interactionController.hostWindow.platform + " action=resizeBegin suspendDynamicResolution=false");
            }
        }
    }
    function hasFocusedTextInput() {
        let current = interactionController.hostWindow ? interactionController.hostWindow.activeFocusItem : null;
        while (current) {
            const isTextEditingItem = current.text !== undefined && current.cursorPosition !== undefined && current.selectedText !== undefined;
            if (isTextEditingItem)
                return true;
            current = current.parent;
        }
        return false;
    }
    function reportLayoutBranch(source) {
        const currentPath = interactionController.activePageRouter && interactionController.activePageRouter.currentPath !== undefined ? String(interactionController.activePageRouter.currentPath) : "<none>";
        console.log("[whatson:debug][main.layout][" + source + "] platform=" + (interactionController.hostWindow ? interactionController.hostWindow.platform : "") + " adaptiveLayoutProfile=" + interactionController.adaptiveLayoutProfile + " adaptiveNavigationMode=" + interactionController.adaptiveNavigationMode + " adaptiveMobileLayout=" + interactionController.adaptiveMobileLayout + " adaptiveDesktopLayout=" + interactionController.adaptiveDesktopLayout + " currentPath=" + currentPath);
    }
    function shouldRetainFocusForUiHit(uiData) {
        if (!uiData || uiData.insideWindow === false)
            return true;

        const className = uiData.className === undefined || uiData.className === null ? "" : String(uiData.className).toLowerCase();
        const objectName = uiData.objectName === undefined || uiData.objectName === null ? "" : String(uiData.objectName).toLowerCase();
        const path = uiData.path === undefined || uiData.path === null ? "" : String(uiData.path).toLowerCase();
        const text = uiData.text === undefined || uiData.text === null ? "" : String(uiData.text).trim();
        const label = uiData.label === undefined || uiData.label === null ? "" : String(uiData.label).trim();
        const title = uiData.title === undefined || uiData.title === null ? "" : String(uiData.title).trim();
        const searchable = className + " " + objectName + " " + path;

        for (let i = 0; i < interactionController.focusRetainedUiTokens.length; ++i) {
            const token = String(interactionController.focusRetainedUiTokens[i]).toLowerCase();
            if (token.length > 0 && searchable.indexOf(token) >= 0)
                return true;
        }

        if (text.length > 0 || label.length > 0 || title.length > 0)
            return true;

        return !(className.indexOf("rectangle") >= 0 || (className.indexOf("item") >= 0 && objectName === "unnamed"));
    }
}
