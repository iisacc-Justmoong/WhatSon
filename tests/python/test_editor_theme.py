from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorThemeTests(unittest.TestCase):
    def test_editor_surface_tokens_match_figma_contract(self) -> None:
        main_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        body_layout_text = (REPO_ROOT / "src/app/qml/view/panels/BodyLayout.qml").read_text(encoding="utf-8")
        content_layout_text = (REPO_ROOT / "src/app/qml/view/panels/ContentViewLayout.qml").read_text(encoding="utf-8")
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")
        gutter_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsGutterLayer.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("readonly property color contentsDisplayColor: LV.Theme.surfaceAlt", main_text)
        self.assertIn("readonly property color drawerColor: LV.Theme.panelBackground08", main_text)
        self.assertNotIn("readonly property color contentPanelColor", main_text)

        self.assertIn("property color contentsDisplayColor: LV.Theme.surfaceAlt", body_layout_text)
        self.assertIn("property color drawerColor: LV.Theme.panelBackground08", body_layout_text)
        self.assertNotIn("property color contentPanelColor", body_layout_text)

        self.assertIn("property color displayColor: LV.Theme.surfaceAlt", content_layout_text)
        self.assertIn("property color drawerColor: LV.Theme.panelBackground08", content_layout_text)
        self.assertNotIn("property color panelColor", content_layout_text)
        self.assertNotIn("panelColor: contentViewLayout.panelColor", content_layout_text)

        self.assertIn("property color displayColor: LV.Theme.surfaceAlt", display_view_text)
        self.assertIn("readonly property color gutterColor: LV.Theme.subSurface", display_view_text)
        self.assertIn("textColor: LV.Theme.bodyColor", display_view_text)
        self.assertIn('readonly property color lineNumberColor: "#4E5157"', display_view_text)
        self.assertIn('readonly property color activeLineNumberColor: "#9DA0A8"', display_view_text)
        self.assertNotIn("property color panelColor", display_view_text)
        self.assertNotIn("color: contentsView.panelColor", display_view_text)

        self.assertIn("property color gutterColor: LV.Theme.subSurface", gutter_text)

    def test_editor_theme_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("LVRS `subSurface`", readme_text)
        self.assertIn("LV.Theme.surfaceAlt", readme_text)
        self.assertIn("LV.Theme.bodyColor", readme_text)
        self.assertIn("The editor theme contract follows the Figma `ContentsDisplayView` tokens through LVRS aliases", architecture_text)
        self.assertIn("`surfaceAlt` (`panelBackground06`) for the single main editor surface", architecture_text)
        self.assertIn("`panelBackground08`", architecture_text)


if __name__ == "__main__":
    unittest.main()
