from __future__ import annotations

import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorFormatShortcutTests(unittest.TestCase):
    def test_editor_shortcuts_include_highlight_and_selection_cache_fallbacks(self) -> None:
        editor_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("readonly property bool editorInputFocused: {", editor_view_text)
        self.assertIn("contentEditor.editorItem.activeFocus !== undefined", editor_view_text)
        self.assertIn("contentEditor.editorItem.inputItem.activeFocus !== undefined", editor_view_text)
        self.assertIn(
            "showCurrentLineMarker: contentsView.hasSelectedNote || contentsView.editorText.length > 0 || contentsView.editorInputFocused",
            editor_view_text,
        )
        self.assertIn("property int cachedEditorSelectionStart: -1", editor_view_text)
        self.assertIn("property int cachedEditorSelectionEnd: -1", editor_view_text)
        self.assertIn("property int contextMenuSelectionStart: -1", editor_view_text)
        self.assertIn("property int contextMenuSelectionEnd: -1", editor_view_text)
        self.assertIn("function resetEditorSelectionCache() {", editor_view_text)
        self.assertIn("function cachedEditorSelectionRange() {", editor_view_text)
        self.assertIn("function contextMenuEditorSelectionRange() {", editor_view_text)
        self.assertIn("function syncCachedEditorSelectionRange() {", editor_view_text)
        self.assertIn("function handleInlineFormatShortcutKeyPress(event) {", editor_view_text)
        self.assertIn("const commandPressed = (modifiers & Qt.MetaModifier) || (modifiers & Qt.ControlModifier);", editor_view_text)
        self.assertIn("Keys.onPressed: function(event)", editor_view_text)
        self.assertIn("contentsView.handleInlineFormatShortcutKeyPress(event);", editor_view_text)
        self.assertIn("textFormatRenderer.normalizeInlineStyleAliasesForEditor(source);", editor_view_text)
        self.assertIn("sequence: StandardKey.Bold", editor_view_text)
        self.assertIn("sequence: StandardKey.Italic", editor_view_text)
        self.assertIn("sequence: StandardKey.Underline", editor_view_text)
        self.assertIn('sequence: "Meta+Shift+X"', editor_view_text)
        self.assertIn('sequence: "Ctrl+Shift+X"', editor_view_text)
        self.assertIn('sequence: "Meta+Shift+H"', editor_view_text)
        self.assertIn('sequence: "Ctrl+Shift+H"', editor_view_text)
        self.assertNotIn("&& contentsView.editorInputFocused", editor_view_text)
        self.assertIn('wrapSelectedEditorTextWithTag("bold")', editor_view_text)
        self.assertIn('wrapSelectedEditorTextWithTag("italic")', editor_view_text)
        self.assertIn('wrapSelectedEditorTextWithTag("underline")', editor_view_text)
        self.assertIn('wrapSelectedEditorTextWithTag("strikethrough")', editor_view_text)
        self.assertIn('wrapSelectedEditorTextWithTag("highlight")', editor_view_text)
        self.assertIn("readonly property var editorSelectionContextMenuItems: [", editor_view_text)
        self.assertIn('"eventName": "editor.format.bold"', editor_view_text)
        self.assertIn('"eventName": "editor.format.italic"', editor_view_text)
        self.assertIn('"eventName": "editor.format.underline"', editor_view_text)
        self.assertIn('"eventName": "editor.format.strikethrough"', editor_view_text)
        self.assertIn('"eventName": "editor.format.highlight"', editor_view_text)
        self.assertIn("function openEditorSelectionContextMenu(localX, localY) {", editor_view_text)
        self.assertIn("selectionRange = contentsView.cachedEditorSelectionRange();", editor_view_text)
        self.assertIn("function wrapSelectedEditorTextWithTag(tagName, explicitSelectionRange) {", editor_view_text)
        self.assertIn("selectionRange = contentsView.contextMenuEditorSelectionRange();", editor_view_text)
        self.assertIn("editorSelectionContextMenu.openFor(editorViewport", editor_view_text)
        self.assertIn('"<strong style=\\"font-weight:700;\\">"', editor_view_text)
        self.assertIn("if (editorItem.text !== undefined)", editor_view_text)
        self.assertIn("contentsView.editorText = nextText;", editor_view_text)
        self.assertIn("showRenderedOutput: true", editor_view_text)
        self.assertIn("TapHandler {", editor_view_text)
        self.assertIn("acceptedButtons: Qt.RightButton", editor_view_text)
        self.assertIn("id: editorSelectionContextMenu", editor_view_text)
        self.assertIn("const contextSelectionRange = contentsView.contextMenuEditorSelectionRange();", editor_view_text)
        self.assertIn("onItemEventTriggered: function(eventName, payload, index, item)", editor_view_text)

    def test_editor_shortcut_documentation_mentions_highlight_and_selection_cache(self) -> None:
        docs_text = (
            REPO_ROOT / "docs/src/app/qml/view/content/editor/ContentsDisplayView.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn("Shift+Cmd/Ctrl+H", docs_text)
        self.assertIn("background-color:#8A4B00", docs_text)
        self.assertIn("editorInputFocused", docs_text)
        self.assertIn("editorItem.inputItem.activeFocus", docs_text)
        self.assertIn("cachedEditorSelectionRange()", docs_text)
        self.assertIn("contextMenuEditorSelectionRange()", docs_text)
        self.assertIn("non-empty selection range", docs_text)
        self.assertIn("Right-clicking a non-empty editor selection opens an `LV.ContextMenu`", docs_text)
        self.assertIn("<strong style=\\\"font-weight:700;\\\">", docs_text)
        self.assertIn("Bold", docs_text)
        self.assertIn("Italic", docs_text)
        self.assertIn("Underline", docs_text)
        self.assertIn("Strikethrough", docs_text)
        self.assertIn("Highlight", docs_text)


if __name__ == "__main__":
    unittest.main()
