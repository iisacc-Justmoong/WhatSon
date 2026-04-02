from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorPagePrintMarginsTests(unittest.TestCase):
    def test_page_and_print_text_must_follow_print_guide_insets(self) -> None:
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "insetHorizontal: contentsView.showPrintEditorLayout",
            display_view_text,
        )
        self.assertIn(
            "? contentsView.printGuideHorizontalInset",
            display_view_text,
        )
        self.assertIn(
            "insetVertical: contentsView.showPrintEditorLayout",
            display_view_text,
        )
        self.assertIn(
            "? contentsView.printGuideVerticalInset",
            display_view_text,
        )
        self.assertIn(
            "(Number(printEditorPage.x) || 0) + contentsView.printGuideHorizontalInset",
            display_view_text,
        )
        self.assertIn(
            "(Number(printEditorPage.y) || 0) + contentsView.printGuideVerticalInset",
            display_view_text,
        )
        self.assertIn(
            "pageWidth - contentsView.printGuideHorizontalInset * 2",
            display_view_text,
        )

    def test_print_mode_must_keep_guides_and_page_mode_must_hide_guides(self) -> None:
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "readonly property bool showPrintEditorLayout: contentsView.showPageEditorLayout || contentsView.showPrintModeActive",
            display_view_text,
        )
        self.assertIn(
            "readonly property bool showPrintMarginGuides: contentsView.showPrintModeActive",
            display_view_text,
        )
        self.assertIn(
            "visible: contentsView.showPrintMarginGuides",
            display_view_text,
        )

    def test_docs_must_describe_page_print_margin_behavior(self) -> None:
        doc_text = (
            REPO_ROOT / "docs/src/app/qml/view/content/editor/ContentsDisplayView.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn("printGuideHorizontalInset", doc_text)
        self.assertIn("printGuideVerticalInset", doc_text)
        self.assertIn("Page", doc_text)
        self.assertIn("Print", doc_text)
        self.assertIn("guide", doc_text.lower())


if __name__ == "__main__":
    unittest.main()
