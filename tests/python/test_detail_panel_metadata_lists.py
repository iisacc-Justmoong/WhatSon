from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class DetailPanelMetadataListsTests(unittest.TestCase):
    def test_detail_contents_uses_lvrs_active_rows_for_metadata_lists(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/panels/detail/DetailContents.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("readonly property int activeFolderIndex:", qml_text)
        self.assertIn("readonly property int activeTagIndex:", qml_text)
        self.assertIn("function setMetadataListActiveIndex(listKind, index)", qml_text)
        self.assertIn("function deleteActiveMetadataItem(listKind)", qml_text)
        self.assertIn("function beginFolderCreation()", qml_text)
        self.assertIn("function commitFolderCreation(rawText)", qml_text)
        self.assertIn("component DetailListRow: LV.HierarchyItem", qml_text)
        self.assertIn("LV.InputField {", qml_text)
        self.assertIn("rowSelected: index === listSection.activeIndex", qml_text)
        self.assertIn("enabled: listSection.deleteEnabled", qml_text)
        self.assertIn("enabled: listSection.addEnabled", qml_text)
        self.assertIn("detailPanelViewModel.removeActiveFolder()", qml_text)
        self.assertIn("detailPanelViewModel.removeActiveTag()", qml_text)
        self.assertIn("detailPanelViewModel.assignFolderByName(normalizedText)", qml_text)
        self.assertIn('detailContents.setMetadataListActiveIndex("folders", index);', qml_text)
        self.assertIn('detailContents.setMetadataListActiveIndex("tags", index);', qml_text)
        self.assertIn('inlineInputPlaceholderText: "Assign or create folder"', qml_text)


if __name__ == "__main__":
    unittest.main()
