from __future__ import annotations

import re
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class MobileShellLayoutTests(unittest.TestCase):
    def test_mobile_shell_reuses_shared_compact_layout_contract(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        mac_menu_bar_text = (
            REPO_ROOT / "src/app/qml/window/MacNativeMenuBar.qml"
        ).read_text(encoding="utf-8")
        mobile_scaffold_text = (
            REPO_ROOT / "src/app/qml/view/mobile/MobilePageScaffold.qml"
        ).read_text(encoding="utf-8")
        mobile_page_text = (
            REPO_ROOT / "src/app/qml/view/mobile/pages/MobileHierarchyPage.qml"
        ).read_text(encoding="utf-8")
        note_creation_coordinator_text = (
            REPO_ROOT / "src/app/qml/view/mobile/MobileNoteCreationCoordinator.qml"
        ).read_text(encoding="utf-8")
        list_bar_header_text = (
            REPO_ROOT / "src/app/qml/view/panels/ListBarHeader.qml"
        ).read_text(encoding="utf-8")
        navigation_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/NavigationBarLayout.qml"
        ).read_text(encoding="utf-8")
        status_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/StatusBarLayout.qml"
        ).read_text(encoding="utf-8")
        hierarchy_layout_text = (
            REPO_ROOT / "src/app/qml/view/panels/HierarchySidebarLayout.qml"
        ).read_text(encoding="utf-8")
        hierarchy_view_text = (
            REPO_ROOT / "src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"
        ).read_text(encoding="utf-8")
        rename_controller_text = (
            REPO_ROOT
            / "src/app/qml/view/panels/sidebar/SidebarHierarchyRenameController.qml"
        ).read_text(encoding="utf-8")
        mode_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationModeBar.qml"
        ).read_text(encoding="utf-8")
        control_bar_text = (
            REPO_ROOT
            / "src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"
        ).read_text(encoding="utf-8")
        view_bar_text = (
            REPO_ROOT
            / "src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml"
        ).read_text(encoding="utf-8")
        edit_bar_text = (
            REPO_ROOT
            / "src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("signal compactAddFolderRequested", navigation_bar_text)
        self.assertIn("property bool compactAddFolderVisible: false", navigation_bar_text)
        self.assertIn("signal compactLeadingActionRequested", navigation_bar_text)
        self.assertIn("property bool compactLeadingActionVisible: false", navigation_bar_text)
        self.assertIn('property string compactLeadingActionIconName: "generalchevronLeft"', navigation_bar_text)
        self.assertIn("readonly property int compactLeftGroupSpacing: LV.Theme.gap4", navigation_bar_text)
        self.assertIn("readonly property int compactRightGroupSpacing: LV.Theme.gap12", navigation_bar_text)
        self.assertIn("iconName: navigationBar.compactLeadingActionIconName", navigation_bar_text)
        self.assertIn('iconName: "settings"', navigation_bar_text)
        self.assertIn("showLabel: false", navigation_bar_text)
        self.assertIn('iconName: "nodesnewFolder"', navigation_bar_text)
        self.assertIn("enabled: navigationBar.compactAddFolderEnabled", navigation_bar_text)
        self.assertIn("navigationBar.compactLeadingActionRequested()", navigation_bar_text)
        self.assertIn("navigationBar.compactAddFolderRequested();", navigation_bar_text)
        self.assertIn("visible: navigationBar.compactMode", navigation_bar_text)
        self.assertIn("property color compactSurfaceColor: LV.Theme.panelBackground10", navigation_bar_text)
        self.assertIn("radius: navigationBar.compactMode ? LV.Theme.radiusXl * 2 : LV.Theme.gapNone", navigation_bar_text)
        compact_add_folder_index = navigation_bar_text.index('iconName: "nodesnewFolder"')
        compact_loader_index = navigation_bar_text.index("Loader {", compact_add_folder_index)
        self.assertLess(compact_add_folder_index, compact_loader_index)

        self.assertIn("property bool showLabel: true", mode_bar_text)
        self.assertIn("visible: modeBar.showLabel", mode_bar_text)
        self.assertIn("LV.IconMenuButton {", control_bar_text)
        self.assertIn('iconName: "toolwindowtodo"', control_bar_text)
        self.assertIn("leftPadding: LV.Theme.gap2", control_bar_text)
        self.assertIn("rightPadding: LV.Theme.gap4", control_bar_text)
        self.assertIn("spacing: LV.Theme.gapNone", control_bar_text)
        self.assertNotIn('iconName: "generalsearch"', control_bar_text)
        self.assertIn("LV.IconMenuButton {", view_bar_text)
        self.assertIn('iconName: "toolwindowtodo"', view_bar_text)
        self.assertIn("applicationViewContextMenu.openFor(", view_bar_text)
        self.assertIn("applicationViewMenuButton.width,", view_bar_text)
        self.assertIn(
            "applicationViewMenuButton.height + applicationViewBar.menuYOffset",
            view_bar_text,
        )
        self.assertIn("LV.IconMenuButton {", edit_bar_text)
        self.assertIn('iconName: "toolwindowtodo"', edit_bar_text)
        self.assertIn("applicationEditContextMenu.openFor(", edit_bar_text)
        self.assertIn("applicationEditMenuButton.width,", edit_bar_text)
        self.assertIn(
            "applicationEditMenuButton.height + applicationEditBar.menuYOffset",
            edit_bar_text,
        )

        self.assertIn("signal createNoteRequested", status_bar_text)
        self.assertIn("property int compactBottomInset: LV.Theme.gapNone", status_bar_text)
        self.assertIn("property int compactHorizontalInset: LV.Theme.gapNone", status_bar_text)
        self.assertIn("property int compactFieldRadius: LV.Theme.radiusControl", status_bar_text)
        self.assertIn("anchors.fill: parent", status_bar_text)
        self.assertIn("fieldMinHeight: statusBar.compactFieldHeight", status_bar_text)
        self.assertIn("backgroundColor: statusBar.compactFieldColor", status_bar_text)
        self.assertIn("backgroundColorFocused: statusBar.compactFieldColor", status_bar_text)
        self.assertIn("placeholderText: statusBar.compactToolbarText", status_bar_text)
        self.assertIn("shapeStyle: shapeCylinder", status_bar_text)
        self.assertNotIn("id: compactSearchTextField", status_bar_text)
        self.assertNotIn('statusBar.requestViewHook("create-note");', status_bar_text)
        self.assertIn("statusBar.createNoteRequested();", status_bar_text)

        self.assertIn("property bool footerVisible: true", hierarchy_layout_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_layout_text)
        self.assertIn("property int searchHeaderMinHeight: LV.Theme.gap24", hierarchy_layout_text)
        self.assertIn("property int searchHeaderTopGap: LV.Theme.gap4", hierarchy_layout_text)
        self.assertIn("property int searchListGap: LV.Theme.gapNone", hierarchy_layout_text)
        self.assertIn("property int searchHeaderVerticalInset: LV.Theme.gap2", hierarchy_layout_text)
        self.assertIn("property int verticalInset: LV.Theme.gap2", hierarchy_layout_text)
        self.assertIn("function requestCreateFolder()", hierarchy_layout_text)
        self.assertIn("signal hierarchyItemActivated(var item, int itemId, int index)", hierarchy_layout_text)
        self.assertIn("hierarchyView.hierarchyItemActivated(item, itemId, index);", hierarchy_layout_text)
        self.assertNotIn("property bool hierarchyEditable: true", hierarchy_layout_text)
        self.assertIn(
            "hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable",
            hierarchy_layout_text,
        )
        self.assertIn("property bool footerVisible: true", hierarchy_view_text)
        self.assertIn("property bool hierarchyEditable: false", hierarchy_view_text)
        self.assertIn("property bool hierarchyExpansionActivationSuppressed: false", hierarchy_view_text)
        self.assertIn("function armHierarchyExpansionActivationSuppression(itemId)", hierarchy_view_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_view_text)
        self.assertIn("property int searchHeaderMinHeight: LV.Theme.gap24", hierarchy_view_text)
        self.assertIn("property int searchHeaderTopGap: LV.Theme.gap4", hierarchy_view_text)
        self.assertIn("property int searchListGap: LV.Theme.gapNone", hierarchy_view_text)
        self.assertIn("property int searchHeaderVerticalInset: LV.Theme.gap2", hierarchy_view_text)
        self.assertIn("property int verticalInset: LV.Theme.gap2", hierarchy_view_text)
        self.assertIn("signal hierarchyItemActivated(var item, int itemId, int index)", hierarchy_view_text)
        self.assertIn("if (sidebarHierarchyView.hierarchyExpansionActivationSuppressed)", hierarchy_view_text)
        self.assertIn(
            "sidebarHierarchyView.armHierarchyExpansionActivationSuppression(itemId);",
            hierarchy_view_text,
        )
        self.assertIn("PanelView.ListBarHeader", hierarchy_view_text)
        self.assertIn("frameMinHeight: sidebarHierarchyView.searchHeaderMinHeight", hierarchy_view_text)
        self.assertIn("outerHorizontalInset: LV.Theme.gapNone", hierarchy_view_text)
        self.assertIn("outerVerticalInset: sidebarHierarchyView.searchHeaderVerticalInset", hierarchy_view_text)
        self.assertIn("searchFieldShapeStyle: hierarchySearchHeader.shapeRoundRect", hierarchy_view_text)
        self.assertIn("visibilityActionVisible: false", hierarchy_view_text)
        self.assertIn("sortActionVisible: false", hierarchy_view_text)
        self.assertIn("return clone;", rename_controller_text)
        self.assertIn("return projectedModel;", rename_controller_text)
        self.assertIn("hierarchySearchHeader.implicitHeight + sidebarHierarchyView.searchListGap", hierarchy_view_text)
        self.assertIn(
            "anchors.topMargin: sidebarHierarchyView.verticalInset + (sidebarHierarchyView.searchFieldVisible ? sidebarHierarchyView.searchHeaderTopGap + hierarchySearchHeader.implicitHeight + sidebarHierarchyView.searchListGap : 0)",
            hierarchy_view_text,
        )
        self.assertNotIn(
            "sidebarHierarchyView.toolbarButtonSize + sidebarHierarchyView.searchHeaderTopGap + hierarchySearchHeader.implicitHeight + sidebarHierarchyView.searchListGap",
            hierarchy_view_text,
        )

        self.assertIn("readonly property int actionButtonSize: LV.Theme.gap20", list_bar_header_text)
        self.assertIn("property int frameMinHeight: LV.Theme.gap24", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldHeight: LV.Theme.gap18", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldTextHeight: LV.Theme.gap12", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldHorizontalInset: LV.Theme.gap7", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldVerticalInset: LV.Theme.gap3", list_bar_header_text)
        self.assertIn("readonly property int shapeCylinder: searchField.shapeCylinder", list_bar_header_text)
        self.assertIn("readonly property int shapeRoundRect: searchField.shapeRoundRect", list_bar_header_text)
        self.assertIn("property int searchFieldShapeStyle: listBarHeader.shapeCylinder", list_bar_header_text)
        self.assertIn("property int outerHorizontalInset: LV.Theme.gap2", list_bar_header_text)
        self.assertIn("property int outerVerticalInset: LV.Theme.gap2", list_bar_header_text)
        self.assertIn("implicitHeight: Math.max(listBarHeader.frameMinHeight, headerRow.implicitHeight + listBarHeader.outerVerticalInset * 2)", list_bar_header_text)
        self.assertIn("anchors.topMargin: listBarHeader.outerVerticalInset", list_bar_header_text)
        self.assertIn("mode: searchMode", list_bar_header_text)
        self.assertIn("shapeStyle: listBarHeader.searchFieldShapeStyle", list_bar_header_text)
        self.assertNotIn("readonly property int inlineFieldSearchIconSize", list_bar_header_text)
        self.assertNotIn("cornerRadius: LV.Theme.radiusControl", list_bar_header_text)
        self.assertNotIn("searchIconVisible: false", list_bar_header_text)
        self.assertNotIn("trailingItems: Item {", list_bar_header_text)

        self.assertIn("readonly property var activePageRouter: bodyRouter", mobile_scaffold_text)
        self.assertIn("readonly property var bodyItem: bodyRouter.currentPageItem", mobile_scaffold_text)
        self.assertIn("readonly property real bodyWidth: bodyRouter.width", mobile_scaffold_text)
        self.assertIn('property string bodyInitialPath: "/"', mobile_scaffold_text)
        self.assertIn("property var bodyRoutes: []", mobile_scaffold_text)
        self.assertIn("readonly property int contentHorizontalPadding: LV.Theme.gap16", mobile_scaffold_text)
        self.assertIn("readonly property int contentTopPadding: LV.Theme.gap24 + LV.Theme.gap24", mobile_scaffold_text)
        self.assertIn("readonly property int contentBottomPadding: LV.Theme.gap16", mobile_scaffold_text)
        self.assertIn("readonly property int sectionSpacing: LV.Theme.gap2", mobile_scaffold_text)
        self.assertIn("spacing: mobilePageScaffold.sectionSpacing", mobile_scaffold_text)
        self.assertIn("PanelView.NavigationBarLayout {", mobile_scaffold_text)
        self.assertIn("property bool compactAddFolderVisible: true", mobile_scaffold_text)
        self.assertIn("property bool compactLeadingActionVisible: false", mobile_scaffold_text)
        self.assertIn('property string compactLeadingActionIconName: "generalchevronLeft"', mobile_scaffold_text)
        self.assertIn("compactAddFolderVisible: mobilePageScaffold.compactAddFolderVisible", mobile_scaffold_text)
        self.assertIn("compactLeadingActionVisible: mobilePageScaffold.compactLeadingActionVisible", mobile_scaffold_text)
        self.assertIn("compactMode: true", mobile_scaffold_text)
        self.assertIn("onCompactAddFolderRequested: mobilePageScaffold.compactAddFolderRequested()", mobile_scaffold_text)
        self.assertIn("onCompactLeadingActionRequested: mobilePageScaffold.compactLeadingActionRequested()", mobile_scaffold_text)
        self.assertIn("LV.PageRouter {", mobile_scaffold_text)
        self.assertIn("id: bodyRouter", mobile_scaffold_text)
        self.assertIn("initialPath: mobilePageScaffold.bodyInitialPath", mobile_scaffold_text)
        self.assertIn("registerAsGlobalNavigator: false", mobile_scaffold_text)
        self.assertIn("routes: mobilePageScaffold.bodyRoutes", mobile_scaffold_text)
        self.assertIn("PanelView.StatusBarLayout {", mobile_scaffold_text)
        self.assertIn("onCreateNoteRequested: mobilePageScaffold.createNoteRequested()", mobile_scaffold_text)
        self.assertNotIn("Loader {", mobile_scaffold_text)

        self.assertIn('import ".." as MobileView', mobile_page_text)
        self.assertIn("MobileView.MobilePageScaffold {", mobile_page_text)
        self.assertIn("readonly property var activeNoteListModel: mobileHierarchyPage.sidebarHierarchyViewModel", mobile_page_text)
        self.assertIn("readonly property var activeContentViewModel: mobileHierarchyPage.sidebarHierarchyViewModel", mobile_page_text)
        self.assertIn("resolvedHierarchyViewModel", mobile_page_text)
        self.assertIn("resolvedNoteListModel", mobile_page_text)
        self.assertIn('readonly property string editorRoutePath: "/mobile/editor"', mobile_page_text)
        self.assertIn('readonly property string hierarchyRoutePath: "/mobile/hierarchy"', mobile_page_text)
        self.assertIn('readonly property string noteListRoutePath: "/mobile/note-list"', mobile_page_text)
        self.assertIn("readonly property var mobileBodyRoutes: [", mobile_page_text)
        self.assertIn('"path": mobileHierarchyPage.hierarchyRoutePath', mobile_page_text)
        self.assertIn('"component": hierarchyBodyComponent', mobile_page_text)
        self.assertIn('"path": mobileHierarchyPage.noteListRoutePath', mobile_page_text)
        self.assertIn('"component": noteListBodyComponent', mobile_page_text)
        self.assertIn('"path": mobileHierarchyPage.editorRoutePath', mobile_page_text)
        self.assertIn('"component": editorBodyComponent', mobile_page_text)
        self.assertIn("readonly property bool backNavigationAvailable: mobileScaffold.activePageRouter", mobile_page_text)
        self.assertIn("property int backSwipeConsumedSessionId: -1", mobile_page_text)
        self.assertIn("property int backSwipeSessionId: -1", mobile_page_text)
        self.assertIn("property int preservedNoteListSelectionIndex: -1", mobile_page_text)
        self.assertIn("property bool routeSelectionSyncSuppressed: false", mobile_page_text)
        self.assertIn("readonly property int backSwipeEdgeWidth: LV.Theme.gap24", mobile_page_text)
        self.assertIn(
            "readonly property string resolvedBodyRoutePath: mobileHierarchyPage.displayedBodyRoutePath()",
            mobile_page_text,
        )
        self.assertIn(
            "readonly property bool editorPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.editorRoutePath",
            mobile_page_text,
        )
        self.assertIn(
            "readonly property bool hierarchyPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.hierarchyRoutePath",
            mobile_page_text,
        )
        self.assertIn(
            "readonly property bool noteListPageActive: mobileHierarchyPage.resolvedBodyRoutePath === mobileHierarchyPage.noteListRoutePath",
            mobile_page_text,
        )
        self.assertIn(
            'panelViewModelRegistry.panelViewModel("mobile.MobileHierarchyPage")',
            mobile_page_text,
        )
        self.assertIn("if (panelViewModel && panelViewModel.requestViewModelHook)", mobile_page_text)
        self.assertIn("panelViewModel.requestViewModelHook(hookReason);", mobile_page_text)
        self.assertIn("compactAddFolderVisible: !mobileHierarchyPage.noteListPageActive && !mobileHierarchyPage.editorPageActive", mobile_page_text)
        self.assertIn("compactNoteListControlsVisible: mobileHierarchyPage.noteListPageActive", mobile_page_text)
        self.assertIn("compactSettingsVisible: mobileHierarchyPage.hierarchyPageActive", mobile_page_text)
        self.assertIn("compactLeadingActionVisible: false", mobile_page_text)
        self.assertIn("bodyInitialPath: mobileHierarchyPage.hierarchyRoutePath", mobile_page_text)
        self.assertIn("bodyRoutes: mobileHierarchyPage.mobileBodyRoutes", mobile_page_text)
        self.assertIn("onCompactAddFolderRequested: mobileHierarchyPage.requestCreateFolder()", mobile_page_text)
        self.assertIn("onCompactLeadingActionRequested: mobileHierarchyPage.requestBackToHierarchy()", mobile_page_text)
        self.assertIn("onCreateNoteRequested: noteCreationCoordinator.requestCreateNote()", mobile_page_text)
        self.assertIn("function beginBackSwipeGesture(eventData)", mobile_page_text)
        self.assertIn("function cancelBackSwipeGesture(eventData)", mobile_page_text)
        self.assertIn("function clearActiveHierarchySelection()", mobile_page_text)
        self.assertIn("function currentHierarchySelectionIndex()", mobile_page_text)
        self.assertIn("function finishBackSwipeGesture(eventData, cancelled)", mobile_page_text)
        self.assertIn("function rememberNoteListSelection(selectionIndex)", mobile_page_text)
        self.assertIn("function requestBackToHierarchy()", mobile_page_text)
        self.assertIn("function requestOpenEditor(noteId, index)", mobile_page_text)
        self.assertIn("function requestOpenNoteList(item, itemId, index)", mobile_page_text)
        self.assertIn("function restoreNoteListSelection(selectionIndex)", mobile_page_text)
        self.assertIn("function routeToHierarchyRoot()", mobile_page_text)
        self.assertIn("function syncRouteSelectionState()", mobile_page_text)
        self.assertIn("function updateBackSwipeGesture(eventData)", mobile_page_text)
        self.assertIn("property string lastObservedRoutePath: hierarchyRoutePath", mobile_page_text)
        self.assertIn(
            "onResolvedBodyRoutePathChanged: mobileHierarchyPage.syncRouteSelectionState()",
            mobile_page_text,
        )
        self.assertIn("sessionId === mobileHierarchyPage.backSwipeConsumedSessionId", mobile_page_text)
        self.assertNotIn("mobileHierarchyPage.backSwipeSessionId < 0 && !mobileHierarchyPage.beginBackSwipeGesture(eventData)", mobile_page_text)
        self.assertIn("Component {", mobile_page_text)
        self.assertIn("id: hierarchyBodyComponent", mobile_page_text)
        self.assertIn("PanelView.HierarchySidebarLayout {", mobile_page_text)
        self.assertIn("footerVisible: false", mobile_page_text)
        self.assertIn("horizontalInset: LV.Theme.gapNone", mobile_page_text)
        self.assertIn("verticalInset: LV.Theme.gapNone", mobile_page_text)
        self.assertIn("searchFieldVisible: true", mobile_page_text)
        self.assertIn("searchHeaderMinHeight: LV.Theme.gap18", mobile_page_text)
        self.assertIn("searchHeaderVerticalInset: LV.Theme.gapNone", mobile_page_text)
        self.assertIn("searchHeaderTopGap: LV.Theme.gap2", mobile_page_text)
        self.assertIn("searchListGap: LV.Theme.gap2", mobile_page_text)
        self.assertIn("toolbarFrameWidth: width", mobile_page_text)
        self.assertIn("onHierarchyItemActivated: function (item, itemId, index) {", mobile_page_text)
        self.assertIn("mobileHierarchyPage.requestOpenNoteList(item, itemId, index);", mobile_page_text)
        self.assertIn("LV.PageTransitionController {", mobile_page_text)
        self.assertIn("router: mobileScaffold.activePageRouter", mobile_page_text)
        self.assertIn("onCommitted: function (state) {", mobile_page_text)
        self.assertIn(
            "mobileHierarchyPage.handleCommittedRouteTransition(state);",
            mobile_page_text,
        )
        self.assertIn("id: backSwipeEdgeZone", mobile_page_text)
        self.assertIn("visible: mobileHierarchyPage.backNavigationAvailable", mobile_page_text)
        self.assertIn("width: visible ? mobileHierarchyPage.backSwipeEdgeWidth : 0", mobile_page_text)
        self.assertIn("id: backSwipeDragHandler", mobile_page_text)
        self.assertIn("DragHandler {", mobile_page_text)
        self.assertIn("acceptedDevices: PointerDevice.TouchScreen", mobile_page_text)
        self.assertIn("signal noteActivated(int index, string noteId)", (REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml").read_text(encoding="utf-8"))
        self.assertIn("listBarLayout.noteActivated(targetIndex, normalizedNoteId);", (REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml").read_text(encoding="utf-8"))
        self.assertIn("onNoteActivated: function (index, noteId) {", mobile_page_text)
        self.assertIn("mobileHierarchyPage.requestOpenEditor(noteId, index);", mobile_page_text)
        self.assertIn("mobileScaffold.activePageRouter.push(mobileHierarchyPage.editorRoutePath);", mobile_page_text)
        self.assertIn("id: editorBodyComponent", mobile_page_text)
        self.assertIn("PanelView.ContentViewLayout {", mobile_page_text)
        self.assertIn("contentViewModel: mobileHierarchyPage.activeContentViewModel", mobile_page_text)
        self.assertIn("displayColor: mobileHierarchyPage.canvasColor", mobile_page_text)
        self.assertIn("drawerVisible: false", mobile_page_text)
        self.assertIn("frameHorizontalInsetOverride: LV.Theme.gapNone", mobile_page_text)
        self.assertIn('gutterColor: "transparent"', mobile_page_text)
        self.assertIn("gutterWidthOverride: LV.Theme.gap20 * 2", mobile_page_text)
        self.assertIn("lineNumberColumnTextWidthOverride: LV.Theme.gap20 + LV.Theme.gap2", mobile_page_text)
        self.assertIn("minimapVisible: false", mobile_page_text)
        self.assertNotIn("editorTopInsetOverride:", mobile_page_text)
        self.assertIn("pageTransitionController.beginBack({", mobile_page_text)
        self.assertIn("pageTransitionController.shouldCommit(", mobile_page_text)
        self.assertIn("pageTransitionController.finish(shouldCommit);", mobile_page_text)
        self.assertIn("pageTransitionController.update(progress, {", mobile_page_text)
        self.assertIn("mobileHierarchyPage.rememberNoteListSelection();", mobile_page_text)
        self.assertIn("const displayedPath = mobileHierarchyPage.displayedBodyRoutePath();", mobile_page_text)
        self.assertIn("if (displayedPath === mobileHierarchyPage.noteListRoutePath)", mobile_page_text)
        self.assertIn("mobileHierarchyPage.routeSelectionSyncSuppressed = true;", mobile_page_text)
        self.assertIn("mobileHierarchyPage.restoreNoteListSelection(preservedSelectionIndex);", mobile_page_text)
        self.assertIn("id: noteListBodyComponent", mobile_page_text)
        self.assertIn("activeToolbarIndex: mobileHierarchyPage.activeToolbarIndex", mobile_page_text)
        self.assertIn("headerVisible: false", mobile_page_text)
        self.assertIn("noteListModel: mobileHierarchyPage.activeNoteListModel", mobile_page_text)
        self.assertIn("searchText: mobileHierarchyPage.statusSearchText", mobile_page_text)
        self.assertIn("onActiveNoteListModelChanged:", mobile_page_text)
        self.assertIn("mobileHierarchyPage.routeToHierarchyRoot();", mobile_page_text)
        self.assertIn("target: mobileScaffold.activePageRouter", mobile_page_text)
        self.assertIn("function onCurrentPathChanged()", mobile_page_text)
        self.assertIn("mobileHierarchyPage.syncRouteSelectionState();", mobile_page_text)
        self.assertIn(
            "const routerCurrentPath = String(mobileScaffold.activePageRouter.currentPath);",
            mobile_page_text,
        )
        self.assertIn("const depth = mobileHierarchyPage.routeStackDepth();", mobile_page_text)
        self.assertIn(
            "if (routerCurrentPath !== mobileHierarchyPage.hierarchyRoutePath || depth > 1)",
            mobile_page_text,
        )
        self.assertIn("mobileHierarchyPage.activeContentViewModel.setHierarchySelectedIndex(-1);", mobile_page_text)
        self.assertIn("if (mobileHierarchyPage.routeSelectionSyncSuppressed)", mobile_page_text)
        self.assertIn("if (mobileScaffold.bodyItem && mobileScaffold.bodyItem.requestCreateFolder !== undefined)", mobile_page_text)
        self.assertIn("mobileScaffold.activePageRouter.push(mobileHierarchyPage.noteListRoutePath);", mobile_page_text)
        self.assertIn("mobileScaffold.activePageRouter.back();", mobile_page_text)
        self.assertIn("mobileScaffold.activePageRouter.setRoot(mobileHierarchyPage.hierarchyRoutePath);", mobile_page_text)
        self.assertIn("const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection(itemId);", mobile_page_text)
        self.assertIn("const preservedSelectionIndex = mobileHierarchyPage.rememberNoteListSelection();", mobile_page_text)
        self.assertIn("mobileHierarchyPage.routeToCanonicalNoteList(preservedSelectionIndex);", mobile_page_text)
        self.assertIn("mobileHierarchyPage.routeToCanonicalEditor(preservedSelectionIndex);", mobile_page_text)
        self.assertIn("MobileView.MobileNoteCreationCoordinator {", mobile_page_text)
        self.assertIn("onCreateNoteRequested: noteCreationCoordinator.requestCreateNote()", mobile_page_text)
        self.assertIn("onActiveContentViewModelChanged: noteCreationCoordinator.routePendingCreatedNoteToEditor()", mobile_page_text)
        self.assertIn("onOpenEditorRequested: function (noteId, index) {", mobile_page_text)
        self.assertIn("windowInteractions: mobileHierarchyPage.windowInteractions", mobile_page_text)
        self.assertIn("function requestCreateNote()", note_creation_coordinator_text)
        self.assertIn("function routePendingCreatedNoteToEditor()", note_creation_coordinator_text)
        self.assertIn("function scheduleCreatedNoteEditorRoute(noteId)", note_creation_coordinator_text)
        self.assertIn("noteCreationCoordinator.windowInteractions.createNoteFromShortcut();", note_creation_coordinator_text)
        self.assertIn("target: noteCreationCoordinator.noteCreationViewModel", note_creation_coordinator_text)
        self.assertIn(
            "function backSwipeGestureEventData(localX, localY, totalDeltaX, totalDeltaY, sessionId)",
            mobile_page_text,
        )
        self.assertIn("id: backSwipeDragHandler", mobile_page_text)
        self.assertIn("DragHandler {", mobile_page_text)
        self.assertIn("acceptedDevices: PointerDevice.TouchScreen", mobile_page_text)
        self.assertIn(
            "grabPermissions: PointerHandler.CanTakeOverFromAnything",
            mobile_page_text,
        )
        self.assertNotIn('trigger: "touchStarted"', mobile_page_text)
        self.assertNotIn('trigger: "touchUpdated"', mobile_page_text)
        self.assertNotIn('trigger: "touchEnded"', mobile_page_text)
        self.assertNotIn('trigger: "touchCancelled"', mobile_page_text)
        self.assertNotIn("LV.EventListener {", mobile_page_text)
        self.assertNotIn("routeMemoryStack", mobile_page_text)
        self.assertNotIn("activeMobilePage", mobile_page_text)
        self.assertNotIn("bodyComponent: hierarchyBodyComponent", mobile_page_text)

        self.assertIn("readonly property var rootNavigationModeViewModel: {", main_text)
        self.assertIn('return LV.ViewModels.get("navigationModeViewModel");', main_text)
        self.assertIn("readonly property var rootSidebarHierarchyViewModel: {", main_text)
        self.assertIn('return LV.ViewModels.get("sidebarHierarchyViewModel");', main_text)
        self.assertIn("navigationModeViewModel: applicationWindow.rootNavigationModeViewModel", main_text)
        self.assertIn("sidebarHierarchyViewModel: applicationWindow.rootSidebarHierarchyViewModel", main_text)
        self.assertIn("toolbarIconNames: applicationWindow.hierarchyToolbarIconNames", main_text)
        self.assertIn("windowInteractions: windowInteractions", main_text)
        self.assertIn('import "view/mobile/pages" as MobilePageView', main_text)
        self.assertIn("MobilePageView.MobileHierarchyPage {", main_text)
        self.assertNotIn("import Qt.labs.platform as Platform", main_text)
        self.assertIn('source: applicationWindow.platform === "osx"', main_text)
        self.assertIn('Qt.resolvedUrl("window/MacNativeMenuBar.qml")', main_text)
        self.assertIn("if (item)", main_text)
        self.assertIn("item.hostWindow = applicationWindow", main_text)

        self.assertIn("import Qt.labs.platform as Platform", mac_menu_bar_text)
        self.assertIn("Platform.MenuBar {", mac_menu_bar_text)
        self.assertIn('title: qsTr("Window")', mac_menu_bar_text)
        self.assertIn("root.hostWindow.showOnboardingWindow();", mac_menu_bar_text)

        self.assertNotIn('iconName: "audioToAudio"', control_bar_text)
        self.assertIn('"keyVisible": false', control_bar_text)
        self.assertEqual(
            control_bar_text.count('"keyVisible": false'),
            control_bar_text.count('"showChevron": false'),
        )
        self.assertIn(
            "applicationControlContextMenu.openFor(",
            control_bar_text,
        )
        self.assertIn('iconName: "toolwindowtodo"', control_bar_text)
        self.assertIn('iconName: "sortByType"', control_bar_text)
        self.assertIn('iconName: "cwmPermissionView"', control_bar_text)
        self.assertIn(
            "noteListApplicationControlContextMenu.openFor(",
            control_bar_text,
        )
        self.assertIn(
            "applicationControlMenuButton.width,",
            control_bar_text,
        )
        self.assertIn(
            "applicationControlMenuButton.height + applicationControlBar.menuYOffset",
            control_bar_text,
        )
        self.assertIn(
            "noteListApplicationControlMenuButton.width,",
            control_bar_text,
        )
        self.assertIn(
            "noteListApplicationControlMenuButton.height + applicationControlBar.menuYOffset",
            control_bar_text,
        )

    def test_mobile_shell_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")
        agents_text = (REPO_ROOT / "AGENTS.md").read_text(encoding="utf-8")

        self.assertRegex(
            readme_text,
            re.compile(
                r"shared `NavigationBarLayout\.qml`\s+\(`compactMode: true`\)"
            ),
        )
        self.assertIn("`MobilePageScaffold.qml`", readme_text)
        self.assertIn("`MobileHierarchyPage.qml`", readme_text)
        self.assertIn("top navigation bar and the bottom status/add-note bar persistent", readme_text)
        self.assertIn("suppresses the compact leading action", readme_text)
        self.assertIn("note-list body", readme_text)
        self.assertIn("`LV.PageRouter`", readme_text)
        self.assertIn("`LV.PageTransitionController`", readme_text)
        self.assertRegex(readme_text, re.compile(r"left-edge touch\s+`DragHandler`"))
        self.assertIn("hides the shared hierarchy footer on mobile", readme_text)
        self.assertIn("emit LVRS-compatible row drag roles (`draggable`, `dragAllowed`, `movable`, `dragLocked`)", readme_text)
        self.assertIn("disables LVRS `usePlatformSafeMargin`", readme_text)
        self.assertRegex(
            readme_text,
            re.compile(
                r"keeps the hierarchy column on the same `panelBackground01` canvas as the\s+mobile root"
            ),
        )
        self.assertRegex(
            readme_text,
            re.compile(
                r"compact control menu anchors\s+from the trigger's bottom-right corner"
            ),
        )
        self.assertRegex(
            readme_text,
            re.compile(
                r"action-only control entries suppress the LVRS shortcut column"
            ),
        )
        self.assertIn("stays `24px` high on `panelBackground10`", architecture_text)
        self.assertRegex(
            architecture_text,
            re.compile(r"mobile shell does\s+not fork `Hierarchy\.editable`"),
        )
        self.assertIn("system buckets now emit `draggable`, `dragAllowed`, `movable`, and `dragLocked`", architecture_text)
        self.assertIn("resolves to a `20px`", architecture_text)
        self.assertIn("`MobilePageScaffold.qml`", architecture_text)
        self.assertIn("`MobileHierarchyPage.qml`", architecture_text)
        self.assertIn("suppresses the compact leading action", architecture_text)
        self.assertIn("note-list body", architecture_text)
        self.assertIn("`LV.PageRouter`", architecture_text)
        self.assertIn("`LV.PageTransitionController`", architecture_text)
        self.assertRegex(
            architecture_text, re.compile(r"left-edge touch\s+`DragHandler`")
        )
        self.assertIn("disables `usePlatformSafeMargin`", architecture_text)
        self.assertRegex(
            architecture_text,
            re.compile(r"keeps the compact navigation bar and compact status bar mounted"),
        )
        self.assertRegex(
            architecture_text,
            re.compile(
                r"`gap2` VStack spacing"
            ),
        )
        self.assertRegex(
            architecture_text,
            re.compile(r"anchors from the trigger's bottom-right point"),
        )
        self.assertRegex(
            architecture_text,
            re.compile(r"disable\w* the\s+default LVRS shortcut placeholder column"),
        )
        self.assertIn("node `174:4987`", agents_text)
        self.assertIn("node `174:5026`", agents_text)


if __name__ == "__main__":
    unittest.main()
