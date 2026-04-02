from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class NoteListViewportPreservationTests(unittest.TestCase):
    def test_list_bar_preserves_viewport_across_model_reset(self) -> None:
        list_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml"
        ).read_text(encoding="utf-8")
        doc_text = (
            REPO_ROOT / "docs/src/app/qml/view/panels/ListBarLayout.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn("property real preservedNoteListContentY: 0", list_bar_text)
        self.assertIn("property bool noteListViewportRestorePending: false", list_bar_text)
        self.assertIn("function captureNoteListViewport()", list_bar_text)
        self.assertIn(
            "listBarLayout.preservedNoteListContentY = listBarLayout.quantizedNoteListContentY(noteListView.contentY);",
            list_bar_text,
        )
        self.assertIn("function restoreNoteListViewport()", list_bar_text)
        self.assertIn(
            "listBarLayout.applyNoteListViewportStep(listBarLayout.preservedNoteListContentY);",
            list_bar_text,
        )
        self.assertIn("if (listBarLayout.noteListViewportRestorePending) {", list_bar_text)
        self.assertIn("function onModelAboutToBeReset()", list_bar_text)
        self.assertIn("listBarLayout.captureNoteListViewport();", list_bar_text)
        self.assertIn("function onModelReset()", list_bar_text)
        self.assertIn("listBarLayout.restoreNoteListViewport();", list_bar_text)
        self.assertIn(
            "if (listBarLayout.syncingNoteListViewport || listBarLayout.noteListViewportRestorePending)",
            list_bar_text,
        )

        self.assertIn("caches `contentY` before note-model reset cycles", doc_text)

    def test_list_bar_delegate_active_state_falls_back_to_committed_note_id(self) -> None:
        list_bar_text = (
            REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml"
        ).read_text(encoding="utf-8")
        doc_text = (
            REPO_ROOT / "docs/src/app/qml/view/panels/ListBarLayout.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "readonly property string committedNoteId: listBarLayout.currentNoteIdFromModel()",
            list_bar_text,
        )
        self.assertIn(
            "const bridgeCurrentIndex = Number(noteListContractBridge.currentIndex);",
            list_bar_text,
        )
        self.assertIn("function currentNoteIdFromModel()", list_bar_text)
        self.assertIn(
            "const bridgeNoteId = noteListContractBridge.currentNoteId;",
            list_bar_text,
        )
        self.assertIn("function isDelegateActive(index, noteId)", list_bar_text)
        self.assertIn(
            "active: listBarLayout.isDelegateActive(noteItemDelegate.index, noteItemDelegate.noteId)",
            list_bar_text,
        )
        self.assertIn(
            "fallback contract: committed `currentNoteId` equality",
            doc_text,
        )


if __name__ == "__main__":
    unittest.main()
