from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class NoteListItemImageLayoutTests(unittest.TestCase):
    def test_image_variant_matches_figma_contract(self) -> None:
        note_list_item_text = (
            REPO_ROOT / "src/app/qml/view/panels/NoteListItem.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("readonly property int imagePreviewSize: 48", note_list_item_text)
        self.assertIn("implicitHeight: noteListItem.image ? 126 : 102", note_list_item_text)
        self.assertIn("Layout.alignment: Qt.AlignLeft | Qt.AlignTop", note_list_item_text)
        self.assertIn("horizontalAlignment: Text.AlignLeft", note_list_item_text)
        self.assertIn(
            "Layout.preferredHeight: noteListItem.primaryTextBlockHeight", note_list_item_text
        )

    def test_image_variant_contract_is_documented(self) -> None:
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(
            encoding="utf-8"
        )

        self.assertIn("Figma `126px` frame", readme_text)
        self.assertIn("`48px` `imageBox`", readme_text)
        self.assertIn("top-left of the text column", readme_text)
        self.assertIn("optional image mode: the card expands to `194x126`", architecture_text)
        self.assertIn("Figma `48px` `imageBox`", architecture_text)
        self.assertIn("top-left anchored", architecture_text)


if __name__ == "__main__":
    unittest.main()
