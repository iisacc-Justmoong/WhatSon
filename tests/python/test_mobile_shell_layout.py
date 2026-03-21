from __future__ import annotations

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
        self.assertIn('iconName: "generalsettings"', navigation_bar_text)
        self.assertIn("showLabel: false", navigation_bar_text)
        self.assertIn('iconName: "nodeslibraryFolder"', navigation_bar_text)
        self.assertIn("navigationBar.compactAddFolderRequested();", navigation_bar_text)
        self.assertIn("visible: navigationBar.compactMode", navigation_bar_text)

        self.assertIn("property bool showLabel: true", mode_bar_text)
        self.assertIn("visible: modeBar.showLabel", mode_bar_text)

        self.assertIn("signal createNoteRequested", status_bar_text)
        self.assertIn('statusBar.requestViewHook("create-note");', status_bar_text)
        self.assertIn("statusBar.createNoteRequested();", status_bar_text)

        self.assertIn("property bool footerVisible: true", hierarchy_layout_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_layout_text)
        self.assertIn("function requestCreateFolder()", hierarchy_layout_text)
        self.assertIn("property bool footerVisible: true", hierarchy_view_text)
        self.assertIn("property bool searchFieldVisible: false", hierarchy_view_text)
        self.assertIn("PanelView.ListBarHeader", hierarchy_view_text)
        self.assertIn("visibilityActionVisible: false", hierarchy_view_text)
        self.assertIn("sortActionVisible: false", hierarchy_view_text)
        self.assertIn("return clone;", hierarchy_view_text)
        self.assertIn("return projectedModel;", hierarchy_view_text)

        self.assertIn("NavigationBarLayout {", mobile_layout_text)
        self.assertIn("compactMode: true", mobile_layout_text)
        self.assertIn("compactAddFolderVisible: true", mobile_layout_text)
        self.assertIn("HierarchySidebarLayout {", mobile_layout_text)
        self.assertIn("footerVisible: false", mobile_layout_text)
        self.assertIn("searchFieldVisible: true", mobile_layout_text)
        self.assertIn("StatusBarLayout {", mobile_layout_text)
        self.assertIn("onCreateNoteRequested: mobileNormalLayout.requestCreateNote()", mobile_layout_text)
        self.assertIn("windowInteractions.createNoteFromShortcut();", mobile_layout_text)

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
        self.assertIn("collapsed mobile control surface", readme_text)
        self.assertIn("compact mode is now the mobile floating top-bar contract", architecture_text)
        self.assertIn("collapsed menu button", architecture_text)
        self.assertIn("compact mode is now the mobile floating bottom bar", architecture_text)
        self.assertIn("searchFieldVisible", architecture_text)
        self.assertIn("footerVisible", architecture_text)
        self.assertIn("node `174:4986`", agents_text)


if __name__ == "__main__":
    unittest.main()
