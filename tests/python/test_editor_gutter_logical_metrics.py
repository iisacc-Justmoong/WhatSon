from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorGutterLogicalMetricsTests(unittest.TestCase):
    def test_contents_display_view_must_use_logical_text_length_for_gutter_geometry(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("function logicalTextLength()", qml_text)
        self.assertIn("contentsView.logicalTextLength()", qml_text)
        self.assertIn("const logicalLength = contentsView.logicalTextLength();", qml_text)
        self.assertIn("property var logicalLineDocumentYCache: []", qml_text)
        self.assertIn("function ensureLogicalLineDocumentYCache()", qml_text)
        self.assertIn("contentsView.logicalLineDocumentYCacheRevision === refreshRevision", qml_text)
        self.assertNotIn(
            "positionToRectangle(Math.max(0, text.length))",
            qml_text,
        )

    def test_logical_text_bridge_must_normalize_rich_text_to_plain_lines(self) -> None:
        bridge_text = (
            REPO_ROOT / "src/app/viewmodel/content/ContentsLogicalTextBridge.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("#include <QTextDocument>", bridge_text)
        self.assertIn("document.setHtml(text);", bridge_text)
        self.assertIn("document.toPlainText();", bridge_text)
        self.assertIn("m_logicalText.size()", bridge_text)


if __name__ == "__main__":
    unittest.main()
