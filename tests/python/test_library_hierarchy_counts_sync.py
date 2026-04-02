from __future__ import annotations

import re
import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class LibraryHierarchyCountSyncContractTests(unittest.TestCase):
    def test_note_index_mutations_must_resync_hierarchy_model_for_count_projection(self) -> None:
        source_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")

        self.assertRegex(
            source_text,
            re.compile(
                r"bool LibraryHierarchyViewModel::assignNoteToFolder\(int index, const QString& noteId\).*?"
                r"setIndexedStateNotes\([^;]+\);\s*"
                r"syncModel\(\);\s*"
                r"refreshNoteListForSelection\(\);",
                re.DOTALL,
            ),
        )
        self.assertRegex(
            source_text,
            re.compile(
                r"bool LibraryHierarchyViewModel::clearNoteFoldersById\(const QString& noteId\).*?"
                r"setIndexedStateNotes\([^;]+\);\s*"
                r"syncModel\(\);\s*"
                r"refreshNoteListForSelection\(\);",
                re.DOTALL,
            ),
        )
        self.assertRegex(
            source_text,
            re.compile(
                r"bool LibraryHierarchyViewModel::reloadNoteMetadataForNoteId\(const QString& noteId\).*?"
                r"setIndexedStateNotes\([^;]+\);\s*"
                r"syncModel\(\);\s*"
                r"refreshNoteListForSelection\(\);",
                re.DOTALL,
            ),
        )


if __name__ == "__main__":
    unittest.main()
