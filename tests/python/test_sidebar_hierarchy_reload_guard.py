from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class SidebarHierarchyReloadGuardTests(unittest.TestCase):
    def test_sidebar_hierarchy_nodes_change_must_not_reenter_domain_reload_hook(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("function requestHierarchyViewModelReload(reason)", qml_text)
        self.assertIn(
            "const normalizedReason = reason === undefined || reason === null ? \"\" : String(reason).trim();",
            qml_text,
        )
        self.assertIn("if (normalizedReason === \"hierarchy.nodes.changed\")", qml_text)
        self.assertNotIn(
            'sidebarHierarchyView.requestHierarchyViewModelReload("hierarchy.nodes.changed");',
            qml_text,
        )


if __name__ == "__main__":
    unittest.main()
