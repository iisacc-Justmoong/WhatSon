from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorDrawerPanelPartitionTests(unittest.TestCase):
    def test_editor_and_drawer_must_use_complementary_height_contract(self) -> None:
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("id: drawerView", display_view_text)
        self.assertIn("readonly property real effectiveDrawerHeight:", display_view_text)
        self.assertIn("readonly property real drawerReservedHeight:", display_view_text)
        self.assertIn("readonly property real availableDisplayHeight:", display_view_text)
        self.assertIn(
            "drawerView.effectiveDrawerHeight + contentsView.splitterThickness",
            display_view_text,
        )
        self.assertIn(
            "Layout.minimumHeight: contentsView.minDisplayHeight",
            display_view_text,
        )
        self.assertIn(
            "Layout.preferredHeight: drawerView.availableDisplayHeight",
            display_view_text,
        )
        self.assertIn(
            "Layout.preferredHeight: drawerView.effectiveDrawerHeight",
            display_view_text,
        )

    def test_docs_must_describe_drawer_editor_partition_contract(self) -> None:
        doc_text = (
            REPO_ROOT / "docs/src/app/qml/view/content/editor/ContentsDisplayView.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn("effectiveDrawerHeight", doc_text)
        self.assertIn("drawerReservedHeight", doc_text)
        self.assertIn("availableDisplayHeight", doc_text)
        self.assertIn("overlay", doc_text.lower())


if __name__ == "__main__":
    unittest.main()
