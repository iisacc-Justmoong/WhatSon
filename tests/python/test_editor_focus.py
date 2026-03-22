from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class EditorFocusTests(unittest.TestCase):
    def test_new_note_creation_emits_focus_signal_and_editor_consumes_it(self) -> None:
        library_header_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
        ).read_text(encoding="utf-8")
        library_cpp_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")
        body_layout_text = (REPO_ROOT / "src/app/qml/view/panels/BodyLayout.qml").read_text(encoding="utf-8")
        content_layout_text = (
            REPO_ROOT / "src/app/qml/view/panels/ContentViewLayout.qml"
        ).read_text(encoding="utf-8")
        editor_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("void emptyNoteCreated(const QString& noteId);", library_header_text)
        self.assertIn("emit emptyNoteCreated(noteId);", library_cpp_text)
        self.assertIn("if (createdNoteIndex < 0 && !m_noteListModel.searchText().trimmed().isEmpty())", library_cpp_text)
        self.assertIn("m_noteListModel.setSearchText(QString());", library_cpp_text)
        self.assertIn("libraryHierarchyViewModel: libraryHierarchyViewModel", body_layout_text)
        self.assertIn("property var libraryHierarchyViewModel: null", content_layout_text)
        self.assertIn("libraryHierarchyViewModel: contentViewLayout.libraryHierarchyViewModel", content_layout_text)
        self.assertIn('property string pendingEditorFocusNoteId: ""', editor_view_text)
        self.assertIn("function scheduleEditorFocusForNote(noteId) {", editor_view_text)
        self.assertIn("function focusEditorForPendingNote() {", editor_view_text)
        self.assertIn("contentsView.focusEditorForPendingNote();", editor_view_text)
        self.assertIn("function onEmptyNoteCreated(noteId) {", editor_view_text)
        self.assertIn("target: contentsView.libraryHierarchyViewModel", editor_view_text)
        self.assertIn("ignoreUnknownSignals: true", editor_view_text)
        self.assertIn("contentEditor.forceActiveFocus();", editor_view_text)
        self.assertIn("contentEditor.editorItem.forceActiveFocus();", editor_view_text)
        self.assertIn("contentEditor.cursorPosition = cursorPosition;", editor_view_text)
        self.assertIn("contentEditor.editorItem.cursorPosition = cursorPosition;", editor_view_text)
        self.assertIn("New note creation now emits a dedicated runtime signal", readme_text)
        self.assertIn("active note search would hide the freshly created note", readme_text)
        self.assertIn("immediate body typing works after creation", architecture_text)
        self.assertIn("current note-list search filter would hide the new note", architecture_text)

    def test_selected_editor_session_keeps_local_body_authority(self) -> None:
        editor_session_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsEditorSession.qml"
        ).read_text(encoding="utf-8")
        editor_view_text = (
            REPO_ROOT / "src/app/qml/view/content/editor/ContentsDisplayView.qml"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("property bool localEditorAuthority: false", editor_session_text)
        self.assertIn("function markLocalEditorAuthority()", editor_session_text)
        self.assertIn("function shouldAcceptModelBodyText(noteId, text)", editor_session_text)
        self.assertIn("return !editorSession.localEditorAuthority;", editor_session_text)
        self.assertIn(
            "editorSession.shouldAcceptModelBodyText(contentsView.selectedNoteId, contentsView.selectedNoteBodyText)",
            editor_view_text,
        )
        self.assertIn("editorSession.markLocalEditorAuthority();", editor_view_text)
        self.assertIn("editorSession.scheduleEditorPersistence();", editor_view_text)
        self.assertIn("live editor buffer becomes the source of truth", readme_text)
        self.assertIn("tracks per-note local editor authority", architecture_text)
        self.assertIn("source of truth for that note until selection changes", architecture_text)


if __name__ == "__main__":
    unittest.main()
