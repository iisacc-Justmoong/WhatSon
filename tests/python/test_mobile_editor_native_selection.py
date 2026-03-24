from __future__ import annotations

import unittest
from pathlib import Path


class MobileEditorNativeSelectionTests(unittest.TestCase):
    def test_lvrs_text_editor_keeps_native_selection_gestures_enabled(self) -> None:
        lvrs_text_editor_text = Path(
            "/Users/ymy/.local/LVRS/src/LVRS/qml/components/control/input/TextEditor.qml"
        ).read_text(encoding="utf-8")
        lvrs_text_editor_doc_text = Path(
            "/Users/ymy/.local/LVRS/src/LVRS/docs/components/control/TextEditor.md"
        ).read_text(encoding="utf-8")

        self.assertIn("property bool preferNativeGestures: Theme.mobileTarget", lvrs_text_editor_text)
        self.assertIn(
            "interactive: contentHeight > height && (!control.preferNativeGestures || !control.focused)",
            lvrs_text_editor_text,
        )
        self.assertIn("enabled: control.enabled", lvrs_text_editor_text)
        self.assertIn("if (control.autoFocusOnPress)", lvrs_text_editor_text)
        self.assertIn("control.forceEditorFocus()", lvrs_text_editor_text)
        self.assertIn("mouse.accepted = false", lvrs_text_editor_text)
        self.assertIn("focus-assist `MouseArea` remains enabled on mobile", lvrs_text_editor_doc_text)
        self.assertIn("triple-tap", lvrs_text_editor_doc_text)
        self.assertIn("keyboard selection gestures", lvrs_text_editor_doc_text)


if __name__ == "__main__":
    unittest.main()
