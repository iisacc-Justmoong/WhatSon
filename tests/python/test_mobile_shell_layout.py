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
        mobile_layout_text = (
            REPO_ROOT / "src/app/qml/view/panels/MobileNormalLayout.qml"
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
        mode_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationModeBar.qml"
        ).read_text(encoding="utf-8")
        control_bar_text = (
            REPO_ROOT
            / "src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("signal compactAddFolderRequested", navigation_bar_text)
        self.assertIn("property bool compactAddFolderVisible: false", navigation_bar_text)
        self.assertIn("readonly property int compactLeftGroupSpacing: LV.Theme.gap4", navigation_bar_text)
        self.assertIn("readonly property int compactRightGroupSpacing: LV.Theme.gap12", navigation_bar_text)
        self.assertIn('iconName: "generalsettings"', navigation_bar_text)
        self.assertIn("showLabel: false", navigation_bar_text)
        self.assertIn('iconName: "nodesnewFolder"', navigation_bar_text)
        self.assertIn("navigationBar.compactAddFolderRequested();", navigation_bar_text)
        self.assertIn("visible: navigationBar.compactMode", navigation_bar_text)
        self.assertIn("property color compactSurfaceColor: LV.Theme.panelBackground10", navigation_bar_text)
        self.assertIn("radius: navigationBar.compactMode ? LV.Theme.radiusXl * 2 : LV.Theme.gapNone", navigation_bar_text)

        self.assertIn("property bool showLabel: true", mode_bar_text)
        self.assertIn("visible: modeBar.showLabel", mode_bar_text)

        self.assertIn("signal createNoteRequested", status_bar_text)
        self.assertIn("property int compactBottomInset: LV.Theme.gapNone", status_bar_text)
        self.assertIn("property int compactHorizontalInset: LV.Theme.gapNone", status_bar_text)
        self.assertIn("property int compactFieldRadius: LV.Theme.radiusControl", status_bar_text)
        self.assertIn("anchors.fill: parent", status_bar_text)
        self.assertIn('statusBar.requestViewHook("create-note");', status_bar_text)
        self.assertIn("statusBar.createNoteRequested();", status_bar_text)

        self.assertIn("property bool footerVisible: true", hierarchy_layout_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_layout_text)
        self.assertIn("property int searchHeaderHorizontalInset: LV.Theme.gap2", hierarchy_layout_text)
        self.assertIn("property int searchHeaderMinHeight: LV.Theme.gap24", hierarchy_layout_text)
        self.assertIn("property int searchHeaderTopGap: LV.Theme.gap4", hierarchy_layout_text)
        self.assertIn("property int searchListGap: LV.Theme.gapNone", hierarchy_layout_text)
        self.assertIn("property int searchHeaderVerticalInset: LV.Theme.gap2", hierarchy_layout_text)
        self.assertIn("property int verticalInset: LV.Theme.gap2", hierarchy_layout_text)
        self.assertIn("function requestCreateFolder()", hierarchy_layout_text)
        self.assertNotIn("property bool hierarchyEditable: true", hierarchy_layout_text)
        self.assertIn(
            "hierarchyEditable: hierarchyDragDropBridge.reorderContractAvailable",
            hierarchy_layout_text,
        )
        self.assertIn("property bool footerVisible: true", hierarchy_view_text)
        self.assertIn("property bool hierarchyEditable: false", hierarchy_view_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_view_text)
        self.assertIn("property int searchHeaderHorizontalInset: LV.Theme.gap2", hierarchy_view_text)
        self.assertIn("property int searchHeaderMinHeight: LV.Theme.gap24", hierarchy_view_text)
        self.assertIn("property int searchHeaderTopGap: LV.Theme.gap4", hierarchy_view_text)
        self.assertIn("property int searchListGap: LV.Theme.gapNone", hierarchy_view_text)
        self.assertIn("property int searchHeaderVerticalInset: LV.Theme.gap2", hierarchy_view_text)
        self.assertIn("property int verticalInset: LV.Theme.gap2", hierarchy_view_text)
        self.assertIn("PanelView.ListBarHeader", hierarchy_view_text)
        self.assertIn("frameMinHeight: sidebarHierarchyView.searchHeaderMinHeight", hierarchy_view_text)
        self.assertIn("outerHorizontalInset: sidebarHierarchyView.searchHeaderHorizontalInset", hierarchy_view_text)
        self.assertIn("outerVerticalInset: sidebarHierarchyView.searchHeaderVerticalInset", hierarchy_view_text)
        self.assertIn("visibilityActionVisible: false", hierarchy_view_text)
        self.assertIn("sortActionVisible: false", hierarchy_view_text)
        self.assertIn("return clone;", hierarchy_view_text)
        self.assertIn("return projectedModel;", hierarchy_view_text)
        self.assertIn("hierarchySearchHeader.implicitHeight + sidebarHierarchyView.searchListGap", hierarchy_view_text)

        self.assertIn("readonly property int actionButtonSize: LV.Theme.gap20", list_bar_header_text)
        self.assertIn("property int frameMinHeight: LV.Theme.gap24", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldHeight: LV.Theme.gap18", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldTextHeight: LV.Theme.gap12", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldHorizontalInset: LV.Theme.gap7", list_bar_header_text)
        self.assertIn("readonly property int inlineFieldVerticalInset: LV.Theme.gap3", list_bar_header_text)
        self.assertIn("property int outerHorizontalInset: LV.Theme.gap2", list_bar_header_text)
        self.assertIn("property int outerVerticalInset: LV.Theme.gap2", list_bar_header_text)
        self.assertIn("implicitHeight: Math.max(listBarHeader.frameMinHeight, headerRow.implicitHeight + listBarHeader.outerVerticalInset * 2)", list_bar_header_text)
        self.assertIn("anchors.topMargin: listBarHeader.outerVerticalInset", list_bar_header_text)

        self.assertIn("NavigationBarLayout {", mobile_layout_text)
        self.assertIn("compactMode: true", mobile_layout_text)
        self.assertNotIn("compactAddFolderVisible: true", mobile_layout_text)
        self.assertIn("HierarchySidebarLayout {", mobile_layout_text)
        self.assertNotIn("footerVisible: false", mobile_layout_text)
        self.assertNotIn("hierarchyEditable: false", mobile_layout_text)
        self.assertIn("panelColor: mobileNormalLayout.canvasColor", mobile_layout_text)
        self.assertIn("searchFieldVisible: true", mobile_layout_text)
        self.assertIn("toolbarFrameWidth: width", mobile_layout_text)
        self.assertIn("StatusBarLayout {", mobile_layout_text)
        self.assertIn("onCreateNoteRequested: mobileNormalLayout.requestCreateNote()", mobile_layout_text)
        self.assertIn("windowInteractions.createNoteFromShortcut();", mobile_layout_text)
        self.assertIn("anchors.margins: mobileNormalLayout.contentInset", mobile_layout_text)
        self.assertIn("spacing: mobileNormalLayout.sectionSpacing", mobile_layout_text)

        self.assertIn("navigationModeViewModel: applicationWindow.navigationModeVm", main_text)
        self.assertIn("sidebarHierarchyViewModel: applicationWindow.sidebarHierarchyVm", main_text)
        self.assertIn("toolbarIconNames: applicationWindow.hierarchyToolbarIconNames", main_text)
        self.assertIn("windowInteractions: windowInteractions", main_text)
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

    def test_mobile_shell_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")
        agents_text = (REPO_ROOT / "AGENTS.md").read_text(encoding="utf-8")

        self.assertIn("shared `NavigationBarLayout.qml` (`compactMode: true`)", readme_text)
        self.assertIn("keeps the shared hierarchy footer and editable contract intact", readme_text)
        self.assertIn("emit LVRS-compatible row drag roles (`draggable`, `dragAllowed`, `movable`, `dragLocked`)", readme_text)
        self.assertIn("same `panelBackground01` canvas as the mobile root", readme_text)
        self.assertIn("stays `24px` high on `panelBackground10`", architecture_text)
        self.assertRegex(
            architecture_text,
            re.compile(r"mobile shell does\s+not fork `Hierarchy\.editable`"),
        )
        self.assertIn("system buckets now emit `draggable`, `dragAllowed`, `movable`, and `dragLocked`", architecture_text)
        self.assertIn("resolves to a `20px`", architecture_text)
        self.assertIn("shared `HierarchySidebarLayout.qml` defaults", architecture_text)
        self.assertIn("node `174:4986`", agents_text)


if __name__ == "__main__":
    unittest.main()
