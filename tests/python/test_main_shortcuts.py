from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class MainShortcutTests(unittest.TestCase):
    def test_new_shortcut_uses_existing_create_note_hook_path(self) -> None:
        main_qml_text = (REPO_ROOT / "src/app/qml/Main.qml").read_text(encoding="utf-8")
        controller_text = (
            REPO_ROOT / "src/app/qml/MainWindowInteractionController.qml"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("sequence: StandardKey.New", main_qml_text)
        self.assertIn("onActivated: windowInteractions.createNoteFromShortcut()", main_qml_text)
        self.assertIn("readonly property var rootPanelViewModelRegistry: panelViewModelRegistry", main_qml_text)
        self.assertIn("panelViewModelRegistry: applicationWindow.rootPanelViewModelRegistry", main_qml_text)
        self.assertIn("property var panelViewModelRegistry: null", controller_text)
        self.assertIn("property var libraryNoteMutationViewModel: null", controller_text)
        self.assertIn(
            "readonly property var rootLibraryNoteMutationViewModel: libraryNoteMutationViewModel",
            main_qml_text,
        )
        self.assertIn('panelViewModelRegistry.panelViewModel("navigation.NavigationAddNewBar")', controller_text)
        self.assertIn('addNewPanelViewModel.requestViewModelHook("create-note");', controller_text)
        self.assertIn("setActiveHierarchyIndex(interactionController.libraryHierarchyIndex);", controller_text)
        self.assertIn("return Boolean(noteMutationViewModel.createEmptyNote());", controller_text)
        self.assertIn("global platform-native New shortcut", readme_text)
        self.assertIn("existing `create-note` hook path used by the navigation add surfaces", readme_text)
        self.assertIn("Main.qml` binds the platform-native New shortcut", architecture_text)
        self.assertIn("`create-note` hook path instead of duplicating note-creation policy", architecture_text)


if __name__ == "__main__":
    unittest.main()
