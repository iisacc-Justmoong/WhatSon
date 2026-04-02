from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class DetailPanelSelectionNoopTests(unittest.TestCase):
    def test_detail_contents_skips_duplicate_selector_write_requests(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/panels/detail/DetailContents.qml"
        ).read_text(encoding="utf-8")

        self.assertIn(
            "const currentIndex = detailContents.resolveHierarchyMenuSelectedIndex(hierarchyViewModel);",
            qml_text,
        )
        self.assertIn("if (currentIndex === normalizedIndex)", qml_text)

    def test_detail_panel_viewmodel_short_circuits_duplicate_selection_index(self) -> None:
        cpp_text = (
            REPO_ROOT / "src/app/viewmodel/detailPanel/DetailPanelViewModel.cpp"
        ).read_text(encoding="utf-8")

        self.assertIn("if (index == selectionSourceViewModel.selectedIndex())", cpp_text)
        self.assertIn("return true;", cpp_text)


if __name__ == "__main__":
    unittest.main()
