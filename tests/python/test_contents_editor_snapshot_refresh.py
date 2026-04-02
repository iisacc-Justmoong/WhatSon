from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class ContentsEditorSnapshotRefreshContractTests(unittest.TestCase):
    def test_contents_display_view_must_poll_selected_note_snapshot(self) -> None:
        display_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "readonly property int noteSnapshotRefreshIntervalMs: 1200",
            display_view_text,
        )
        self.assertIn(
            "readonly property bool noteSnapshotRefreshEnabled: contentsView.visible",
            display_view_text,
        )
        self.assertIn("&& contentsView.hasSelectedNote", display_view_text)
        self.assertIn("&& !contentsView.showDedicatedResourceViewer", display_view_text)
        self.assertIn("&& !contentsView.showFormattedTextRenderer", display_view_text)
        self.assertIn("&& contentsView.noteSelectionContractAvailable", display_view_text)
        self.assertIn("function pollSelectedNoteSnapshot()", display_view_text)
        self.assertIn("selectionBridge.refreshSelectedNoteSnapshot();", display_view_text)
        self.assertIn("contentsView.scheduleGutterRefresh(2);", display_view_text)
        self.assertIn("id: noteSnapshotRefreshTimer", display_view_text)
        self.assertIn("running: contentsView.noteSnapshotRefreshEnabled", display_view_text)
        self.assertIn("onTriggered: contentsView.pollSelectedNoteSnapshot()", display_view_text)

    def test_selection_bridge_must_reload_selected_note_metadata(self) -> None:
        bridge_header_text = (
            REPO_ROOT / "src/app/viewmodel/content/ContentsEditorSelectionBridge.hpp"
        ).read_text(encoding="utf-8")
        bridge_cpp_text = (
            REPO_ROOT / "src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "Q_INVOKABLE bool refreshSelectedNoteSnapshot();",
            bridge_header_text,
        )
        self.assertIn(
            'constexpr auto kReloadNoteMetadataForNoteIdSignature = "reloadNoteMetadataForNoteId(QString)";',
            bridge_cpp_text,
        )
        self.assertIn(
            "bool ContentsEditorSelectionBridge::refreshSelectedNoteSnapshot()",
            bridge_cpp_text,
        )
        self.assertIn(
            "QMetaObject::invokeMethod(",
            bridge_cpp_text,
        )
        self.assertIn(
            '"reloadNoteMetadataForNoteId"',
            bridge_cpp_text,
        )
        self.assertIn("refreshNoteSelectionState();", bridge_cpp_text)
        self.assertIn("refreshNoteCountState();", bridge_cpp_text)

    def test_editor_docs_must_document_periodic_snapshot_polling(self) -> None:
        display_view_doc_text = (
            REPO_ROOT / "docs/src/app/qml/view/content/editor/ContentsDisplayView.qml.md"
        ).read_text(encoding="utf-8")
        bridge_header_doc_text = (
            REPO_ROOT / "docs/src/app/viewmodel/content/ContentsEditorSelectionBridge.hpp.md"
        ).read_text(encoding="utf-8")
        bridge_cpp_doc_text = (
            REPO_ROOT / "docs/src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp.md"
        ).read_text(encoding="utf-8")

        self.assertIn("noteSnapshotRefreshTimer", display_view_doc_text)
        self.assertIn("refreshSelectedNoteSnapshot()", display_view_doc_text)
        self.assertIn("refreshSelectedNoteSnapshot()", bridge_header_doc_text)
        self.assertIn("refreshSelectedNoteSnapshot()", bridge_cpp_doc_text)


if __name__ == "__main__":
    unittest.main()
