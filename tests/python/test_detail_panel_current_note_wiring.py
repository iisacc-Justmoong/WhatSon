from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class DetailPanelCurrentNoteWiringTests(unittest.TestCase):
    def test_main_tracks_sidebar_active_note_context_for_detail_panel(self) -> None:
        main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")

        self.assertIn("const auto syncDetailPanelCurrentNoteContext =", main_text)
        self.assertIn(
            "detailPanelViewModel.setCurrentNoteListModel(sidebarHierarchyViewModel.activeNoteListModel());",
            main_text,
        )
        self.assertIn(
            "detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(sidebarHierarchyViewModel.activeHierarchyViewModel());",
            main_text,
        )
        self.assertIn(
            "&SidebarHierarchyViewModel::activeNoteListModelChanged",
            main_text,
        )
        self.assertIn(
            "&SidebarHierarchyViewModel::activeHierarchyViewModelChanged",
            main_text,
        )
        self.assertNotIn(
            "detailPanelViewModel.setCurrentNoteListModel(libraryHierarchyViewModel.noteListModel());",
            main_text,
        )
        self.assertNotIn(
            "detailPanelViewModel.setCurrentNoteDirectorySourceViewModel(&libraryHierarchyViewModel);",
            main_text,
        )


if __name__ == "__main__":
    unittest.main()
