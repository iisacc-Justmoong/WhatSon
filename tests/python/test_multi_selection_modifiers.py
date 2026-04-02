from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class MultiSelectionModifierContractsTests(unittest.TestCase):
    def test_list_bar_must_route_selection_through_modifier_handler(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/panels/ListBarLayout.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("property var selectedNoteIndices: []", qml_text)
        self.assertIn("function requestNoteSelection(index, noteId, modifiers)", qml_text)
        self.assertIn("function selectionRangeModifierPressed(modifiers)", qml_text)
        self.assertIn("function selectionToggleModifierPressed(modifiers)", qml_text)
        self.assertIn("Qt.MetaModifier", qml_text)
        self.assertIn("Qt.ControlModifier", qml_text)
        self.assertIn("Qt.ShiftModifier", qml_text)
        self.assertIn("const applicationModifiers = Qt.application && Qt.application.keyboardModifiers !== undefined", qml_text)
        self.assertIn("return eventModifiers | applicationModifiers;", qml_text)
        self.assertIn("if (listBarLayout.selectionToggleModifierPressed(normalizedModifiers))", qml_text)
        self.assertIn("for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)", qml_text)
        self.assertNotIn("Number(Qt.MetaModifier)", qml_text)
        self.assertNotIn("Number(Qt.ControlModifier)", qml_text)
        self.assertNotIn("Number(Qt.ShiftModifier)", qml_text)
        self.assertIn("listBarLayout.requestNoteSelection(", qml_text)
        self.assertIn("mouse.modifiers", qml_text)
        self.assertIn("eventPoint.modifiers", qml_text)

    def test_sidebar_hierarchy_must_support_modifier_multi_selection(self) -> None:
        qml_text = (
            REPO_ROOT / "src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml"
        ).read_text(encoding="utf-8")

        self.assertIn("property var selectedHierarchyIndices: []", qml_text)
        self.assertIn("function requestHierarchySelection(item, resolvedIndex, modifiers)", qml_text)
        self.assertIn("function hierarchySelectionRangeModifierPressed(modifiers)", qml_text)
        self.assertIn("function hierarchySelectionToggleModifierPressed(modifiers)", qml_text)
        self.assertIn("const applicationModifiers = Qt.application && Qt.application.keyboardModifiers !== undefined", qml_text)
        self.assertIn("return eventModifiers | applicationModifiers;", qml_text)
        self.assertIn("if (sidebarHierarchyView.hierarchySelectionToggleModifierPressed(normalizedModifiers))", qml_text)
        self.assertIn("for (let selectionIndex = 0; selectionIndex < rangeSelection.length; ++selectionIndex)", qml_text)
        self.assertNotIn("Number(Qt.MetaModifier)", qml_text)
        self.assertNotIn("Number(Qt.ControlModifier)", qml_text)
        self.assertNotIn("Number(Qt.ShiftModifier)", qml_text)
        self.assertIn("const activationModifiers = Qt.application.keyboardModifiers;", qml_text)
        self.assertIn(
            "sidebarHierarchyView.requestHierarchySelection(item, resolvedActivationIndex, activationModifiers);",
            qml_text,
        )
        self.assertIn("id: hierarchySelectionOverlayLayer", qml_text)

    def test_docs_must_describe_modifier_selection_contracts(self) -> None:
        list_doc = (
            REPO_ROOT / "docs/src/app/qml/view/panels/ListBarLayout.qml.md"
        ).read_text(encoding="utf-8")
        hierarchy_doc = (
            REPO_ROOT / "docs/src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml.md"
        ).read_text(encoding="utf-8")

        self.assertIn("Cmd/Ctrl", list_doc)
        self.assertIn("Shift", list_doc)
        self.assertIn("Cmd/Ctrl", hierarchy_doc)
        self.assertIn("Shift", hierarchy_doc)


if __name__ == "__main__":
    unittest.main()
