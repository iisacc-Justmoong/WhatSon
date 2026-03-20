from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class LvrsApiPurityTests(unittest.TestCase):
    def test_obsolete_lvrs_override_layers_are_removed(self) -> None:
        self.assertFalse((REPO_ROOT / "src/app/qml/view/panels/sidebar/HierarchyListCompat.qml").exists())
        self.assertFalse((REPO_ROOT / "src/app/qml/view/panels/sidebar/SidebarHierarchyInteractionController.qml").exists())
        self.assertFalse((REPO_ROOT / "src/app/qml/view/panels/navigation/NavigationIconButton.qml").exists())

    def test_registry_and_docs_do_not_reference_deleted_override_layers(self) -> None:
        registry_text = (
            REPO_ROOT / "src/app/viewmodel/panel/PanelViewModelRegistry.cpp"
        ).read_text(encoding="utf-8")
        panel_registry_test_text = (
            REPO_ROOT / "tests/app/test_panel_viewmodel_registry.cpp"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertNotIn("navigation.NavigationIconButton", registry_text)
        self.assertNotIn("NavigationIconButton.qml", panel_registry_test_text)
        self.assertIn("no local hierarchy compat list", readme_text)
        self.assertIn("stock LVRS primitives directly", readme_text)
        self.assertIn("were removed", architecture_text)
        self.assertNotIn("src/app/qml/view/panels/navigation/NavigationIconButton.qml", architecture_text)


if __name__ == "__main__":
    unittest.main()
