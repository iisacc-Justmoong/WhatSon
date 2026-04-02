from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class ResourcesHierarchySelectionFallbackTests(unittest.TestCase):
    def test_note_list_must_not_fallback_to_all_when_selection_is_empty(self) -> None:
        viewmodel_text = (
            REPO_ROOT
            / "src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "if (selectedItem == nullptr)",
            viewmodel_text,
        )
        self.assertIn(
            "m_noteListModel.setItems({});",
            viewmodel_text,
        )

    def test_docs_must_describe_empty_list_when_no_resource_hierarchy_selection(self) -> None:
        doc_text = (
            REPO_ROOT
            / "docs/src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp.md"
        ).read_text(encoding="utf-8")

        self.assertIn("selectedIndex = -1", doc_text)
        self.assertIn("intentionally empty", doc_text)


if __name__ == "__main__":
    unittest.main()
