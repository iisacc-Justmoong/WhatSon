from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class NavigationPanelToggleTests(unittest.TestCase):
    def test_navigation_edge_buttons_toggle_sidebar_and_detail_panel(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        body_text = (REPO_ROOT / "src/app/qml/view/panels/BodyLayout.qml").read_text(encoding="utf-8")
        navigation_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/NavigationBarLayout.qml"
        ).read_text(encoding="utf-8")
        properties_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationPropertiesBar.qml"
        ).read_text(encoding="utf-8")
        information_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationInformationBar.qml"
        ).read_text(encoding="utf-8")
        preference_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationPreferenceBar.qml"
        ).read_text(encoding="utf-8")
        control_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/control/NavigationApplicationControlBar.qml"
        ).read_text(encoding="utf-8")
        edit_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/edit/NavigationApplicationEditBar.qml"
        ).read_text(encoding="utf-8")
        view_text = (
            REPO_ROOT / "src/app/qml/view/panels/navigation/view/NavigationApplicationViewBar.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("property bool hideSidebar: false", main_text)
        self.assertIn("property bool hideRightPanel: false", main_text)
        self.assertIn("readonly property int sidebarWidth: hideSidebar ? 0 : Math.max(minSidebarWidth, preferredSidebarWidth)", main_text)
        self.assertIn("readonly property int rightPanelWidth: hideRightPanel ? 0 : Math.max(minRightPanelWidth, preferredRightPanelWidth)", main_text)
        self.assertIn("function toggleSidebarVisibility()", main_text)
        self.assertIn("function toggleDetailPanelVisibility()", main_text)
        self.assertIn("applicationWindow.hideSidebar = !applicationWindow.hideSidebar;", main_text)
        self.assertIn("applicationWindow.hideRightPanel = !applicationWindow.hideRightPanel;", main_text)
        self.assertIn("sidebarCollapsed: applicationWindow.hideSidebar", main_text)
        self.assertIn("detailPanelCollapsed: applicationWindow.hideRightPanel", main_text)
        self.assertIn("onToggleSidebarRequested: applicationWindow.toggleSidebarVisibility()", main_text)
        self.assertIn("onToggleDetailPanelRequested: applicationWindow.toggleDetailPanelVisibility()", main_text)

        self.assertIn("signal toggleSidebarRequested", navigation_bar_text)
        self.assertIn("signal toggleDetailPanelRequested", navigation_bar_text)
        self.assertIn("sidebarCollapsed: navigationBar.sidebarCollapsed", navigation_bar_text)
        self.assertIn("detailPanelCollapsed: navigationBar.detailPanelCollapsed", navigation_bar_text)
        self.assertIn("onToggleSidebarRequested: navigationBar.toggleSidebarRequested()", navigation_bar_text)
        self.assertIn("onToggleDetailPanelRequested: navigationBar.toggleDetailPanelRequested()", navigation_bar_text)

        self.assertIn("property bool sidebarCollapsed: false", properties_text)
        self.assertIn("signal toggleSidebarRequested", properties_text)
        self.assertIn("sidebarCollapsed: propertiesBar.sidebarCollapsed", properties_text)
        self.assertIn("onToggleSidebarRequested: propertiesBar.toggleSidebarRequested()", properties_text)

        self.assertIn("signal toggleSidebarRequested", information_text)
        self.assertIn("informationBar.toggleSidebarRequested();", information_text)
        self.assertIn('informationBar.requestViewHook(informationBar.sidebarCollapsed ? "expand-sidebar" : "collapse-sidebar");', information_text)

        self.assertIn("signal toggleDetailPanelRequested", preference_text)
        self.assertIn("preferenceBar.toggleDetailPanelRequested();", preference_text)
        self.assertIn('preferenceBar.requestViewHook(preferenceBar.detailPanelCollapsed ? "expand-detail-panel" : "collapse-detail-panel");', preference_text)

        self.assertIn("signal toggleDetailPanelRequested", control_text)
        self.assertIn("detailPanelCollapsed: applicationControlBar.detailPanelCollapsed", control_text)
        self.assertIn("onToggleDetailPanelRequested: applicationControlBar.toggleDetailPanelRequested()", control_text)

        self.assertIn("signal toggleDetailPanelRequested", edit_text)
        self.assertIn("detailPanelCollapsed: applicationEditBar.detailPanelCollapsed", edit_text)
        self.assertIn("onToggleDetailPanelRequested: applicationEditBar.toggleDetailPanelRequested()", edit_text)

        self.assertIn("signal toggleDetailPanelRequested", view_text)
        self.assertIn("detailPanelCollapsed: applicationViewBar.detailPanelCollapsed", view_text)
        self.assertIn("onToggleDetailPanelRequested: applicationViewBar.toggleDetailPanelRequested()", view_text)

        self.assertIn("readonly property bool sidebarVisible: hStack.sidebarWidth > 0", body_text)
        self.assertIn("Layout.minimumWidth: hStack.sidebarVisible ? hStack.effectiveMinSidebarWidth : 0", body_text)
        self.assertIn("Layout.preferredWidth: hStack.sidebarVisible ? hStack.sidebarWidth : 0", body_text)
        self.assertIn("searchFieldVisible: true", body_text)
        self.assertIn("visible: hStack.sidebarVisible", body_text)
        self.assertIn("Layout.minimumWidth: hStack.rightVisible ? hStack.minRightPanelWidth : 0", body_text)
        self.assertIn("Layout.preferredWidth: hStack.rightVisible ? hStack.rightPanelWidth : 0", body_text)
        self.assertIn("var width = hStack.sidebarVisible ? hStack.splitterThickness : 0;", body_text)
        self.assertIn("height: Math.max(1, hStack.splitterThickness)", body_text)
        self.assertIn("color: hStack.splitterColor", body_text)

    def test_navigation_panel_toggle_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("left/right edge buttons", readme_text)
        self.assertIn("hierarchy sidebar", readme_text)
        self.assertIn("detail panel", readme_text)
        self.assertIn("toggle the hierarchy sidebar and detail panel", architecture_text)
        self.assertIn("preserving the stored preferred widths", architecture_text)


if __name__ == "__main__":
    unittest.main()
