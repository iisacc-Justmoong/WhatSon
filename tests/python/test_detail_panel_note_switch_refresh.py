from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class DetailPanelNoteSwitchRefreshContractTests(unittest.TestCase):
    def test_detail_panel_vm_must_reload_header_on_note_id_change(self) -> None:
        view_model_cpp = (
            REPO_ROOT / "src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "&DetailCurrentNoteContextBridge::currentNoteIdChanged",
            view_model_cpp,
        )
        self.assertIn(
            "m_projectSelectionSourceViewModel.setNoteId(noteId);",
            view_model_cpp,
        )
        self.assertIn(
            "m_bookmarkSelectionSourceViewModel.setNoteId(noteId);",
            view_model_cpp,
        )
        self.assertIn(
            "m_progressSelectionSourceViewModel.setNoteId(noteId);",
            view_model_cpp,
        )
        self.assertIn(
            "reloadCurrentHeader(false);",
            view_model_cpp,
        )

    def test_detail_panel_cpp_test_must_cover_stable_directory_note_switch_case(self) -> None:
        cpp_test_text = (
            REPO_ROOT / "tests/app/test_detail_panel_viewmodel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "detailSelectors_mustReloadCurrentHeaderWhenCurrentNoteIdChangesWithStableDirectoryPath",
            cpp_test_text,
        )
        self.assertIn(
            "noteListModel.setCurrentNoteId(secondNoteId);",
            cpp_test_text,
        )
        self.assertIn(
            'QCOMPARE(propertiesViewModel->property("folderItems").toStringList(), QStringList{QStringLiteral("Beta")});',
            cpp_test_text,
        )
        self.assertIn(
            'QCOMPARE(propertiesViewModel->property("tagItems").toStringList(), QStringList{QStringLiteral("second")});',
            cpp_test_text,
        )

    def test_detail_panel_doc_must_document_note_id_reload_guard(self) -> None:
        doc_text = (
            REPO_ROOT
            / "docs/src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp.md"
        ).read_text(encoding="utf-8")

        self.assertIn("currentNoteIdChanged", doc_text)
        self.assertIn("reloadCurrentHeader", doc_text)


if __name__ == "__main__":
    unittest.main()
