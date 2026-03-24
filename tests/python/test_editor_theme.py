from __future__ import annotations

import re
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorThemeTests(unittest.TestCase):
    def test_lvrs_surface_aliases_follow_low_luminance_figma_scale(self) -> None:
        lvrs_theme_text = Path("/Users/ymy/.local/LVRS/src/LVRS/qml/Theme.qml").read_text(encoding="utf-8")
        lvrs_theme_doc_text = Path("/Users/ymy/.local/LVRS/src/LVRS/docs/theme.md").read_text(encoding="utf-8")

        self.assertIn("readonly property color panelBackground01: \"#1B1B1C\"", lvrs_theme_text)
        self.assertIn("readonly property color windowAlt: panelBackground01", lvrs_theme_text)
        self.assertIn("readonly property color subSurface: panelBackground02", lvrs_theme_text)
        self.assertIn("readonly property color surfaceSolid: panelBackground03", lvrs_theme_text)
        self.assertIn("readonly property color surfaceAlt: panelBackground04", lvrs_theme_text)
        self.assertIn("- `windowAlt -> panelBackground01`", lvrs_theme_doc_text)
        self.assertIn("- `subSurface -> panelBackground02`", lvrs_theme_doc_text)
        self.assertIn("- `surfaceSolid -> panelBackground03`", lvrs_theme_doc_text)
        self.assertIn("- `surfaceAlt -> panelBackground04`", lvrs_theme_doc_text)

    def test_editor_surface_tokens_match_figma_contract(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        body_layout_text = (REPO_ROOT / "src/app/qml/view/panels/BodyLayout.qml").read_text(encoding="utf-8")
        content_layout_text = (REPO_ROOT / "src/app/qml/view/panels/ContentViewLayout.qml").read_text(encoding="utf-8")
        navigation_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/NavigationBarLayout.qml"
        ).read_text(encoding="utf-8")
        status_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/StatusBarLayout.qml"
        ).read_text(encoding="utf-8")
        list_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml"
        ).read_text(encoding="utf-8")
        hierarchy_layout_text = (
            REPO_ROOT / "src/app/qml/view/panels/HierarchySidebarLayout.qml"
        ).read_text(encoding="utf-8")
        hierarchy_view_text = (
            REPO_ROOT / "src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"
        ).read_text(encoding="utf-8")
        detail_layout_text = (
            REPO_ROOT / "src/app/qml/view/panels/DetailPanelLayout.qml"
        ).read_text(encoding="utf-8")
        right_panel_text = (
            REPO_ROOT / "src/app/qml/view/panels/detail/RightPanel.qml"
        ).read_text(encoding="utf-8")
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")
        gutter_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsGutterLayer.qml"
        ).read_text(encoding="utf-8")

        self.assertIn('readonly property color desktopPanelSurfaceColor: "transparent"', main_text)
        self.assertIn("panelColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertIn("contentsDisplayColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertIn("drawerColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertIn("listViewColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertIn("rightPanelColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertIn("sidebarColor: applicationWindow.desktopPanelSurfaceColor", main_text)
        self.assertNotIn("readonly property color contentPanelColor", main_text)

        self.assertIn('property color contentsDisplayColor: "transparent"', body_layout_text)
        self.assertIn('property color drawerColor: "transparent"', body_layout_text)
        self.assertIn('property color gutterColor: "transparent"', body_layout_text)
        self.assertIn('property color listViewColor: "transparent"', body_layout_text)
        self.assertIn('property color rightPanelColor: "transparent"', body_layout_text)
        self.assertIn('property color sidebarColor: "transparent"', body_layout_text)
        self.assertNotIn("property color contentPanelColor", body_layout_text)

        self.assertIn('property color displayColor: "transparent"', content_layout_text)
        self.assertIn('property color drawerColor: "transparent"', content_layout_text)
        self.assertIn('property color gutterColor: "transparent"', content_layout_text)
        self.assertNotIn("property color panelColor", content_layout_text)
        self.assertNotIn("panelColor: contentViewLayout.panelColor", content_layout_text)

        self.assertIn('property color displayColor: "transparent"', display_view_text)
        self.assertIn('property color drawerColor: "transparent"', display_view_text)
        self.assertIn('property color gutterColor: "transparent"', display_view_text)
        self.assertIn("textColor: LV.Theme.bodyColor", display_view_text)
        self.assertIn('readonly property color lineNumberColor: "#4E5157"', display_view_text)
        self.assertIn('readonly property color activeLineNumberColor: "#9DA0A8"', display_view_text)
        self.assertNotIn("property color panelColor", display_view_text)
        self.assertNotIn("color: contentsView.panelColor", display_view_text)

        self.assertIn('property color gutterColor: "transparent"', gutter_text)
        self.assertIn('property color panelColor: "transparent"', navigation_bar_text)
        self.assertIn('property color panelColor: "transparent"', status_bar_text)
        self.assertIn('property color panelColor: "transparent"', list_bar_text)
        self.assertIn('property color panelColor: "transparent"', hierarchy_layout_text)
        self.assertIn('property color panelColor: "transparent"', hierarchy_view_text)
        self.assertIn('property color panelColor: "transparent"', detail_layout_text)
        self.assertIn('property color panelColor: "transparent"', right_panel_text)
        self.assertIn('property color searchFieldBackgroundColor: "transparent"', hierarchy_view_text)
        self.assertIn('readonly property color searchFieldColor: "transparent"', status_bar_text)

        lvrs_hierarchy_item_text = Path(
            "/Users/ymy/.local/LVRS/src/LVRS/qml/components/navigation/HierarchyItem.qml"
        ).read_text(encoding="utf-8")
        self.assertIn('property color rowBackgroundColorInactive: "transparent"', lvrs_hierarchy_item_text)

    def test_editor_theme_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertRegex(
            readme_text,
            re.compile(r"root\s+`LV\.ApplicationWindow`\s+`panelBackground01`\s+canvas"),
        )
        self.assertIn("`windowAlt -> panelBackground01`", readme_text)
        self.assertIn("`subSurface -> panelBackground02`", readme_text)
        self.assertIn("desktop status and sidebar search shells now stay transparent", readme_text)
        self.assertRegex(
            readme_text,
            re.compile(r"gutter fill, and lower drawer now stay transparent"),
        )
        self.assertIn("LV.Theme.bodyColor", readme_text)
        self.assertRegex(
            architecture_text,
            re.compile(
                r"root\s+`ApplicationWindow`\s+`panelBackground01`\s+canvas remains the only broad desktop background surface"
            ),
        )
        self.assertRegex(
            architecture_text,
            re.compile(r"editor theme contract now keeps the broad desktop editor surfaces transparent"),
        )
        self.assertIn("`windowAlt -> panelBackground01`", architecture_text)
        self.assertIn("`surfaceAlt -> panelBackground04`", architecture_text)
        self.assertIn("inactive hierarchy rows and desktop search shells stay transparent", architecture_text)
        self.assertIn("line-number colors", architecture_text)


if __name__ == "__main__":
    unittest.main()
