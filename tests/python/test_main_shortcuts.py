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
        app_main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn("sequence: StandardKey.New", main_qml_text)
        self.assertIn("onActivated: windowInteractions.createNoteFromShortcut()", main_qml_text)
        self.assertIn("readonly property var rootPanelViewModelRegistry: panelViewModelRegistry", main_qml_text)
        self.assertIn("panelViewModelRegistry: applicationWindow.rootPanelViewModelRegistry", main_qml_text)
        self.assertIn("property var panelViewModelRegistry: null", controller_text)
        self.assertIn('readonly property string addNewPanelKey: "navigation.NavigationAddNewBar"', controller_text)
        self.assertIn(
            'readonly property string libraryNoteMutationViewModelKey: "libraryNoteMutationViewModel"',
            controller_text,
        )
        self.assertIn(
            'readonly property string sidebarHierarchyViewModelKey: "sidebarHierarchyViewModel"',
            controller_text,
        )
        self.assertIn(
            "function resolveOwnedWritableViewModel(viewId, viewModelKey)",
            controller_text,
        )
        self.assertIn("function resolveLibraryNoteCreationViewModel()", controller_text)
        self.assertIn("libraryNoteMutationViewId: applicationWindow.libraryNoteMutationViewId", main_qml_text)
        self.assertIn('addNewPanelViewModel.requestViewModelHook("create-note");', controller_text)
        self.assertIn("setActiveHierarchyIndex(interactionController.libraryHierarchyIndex);", controller_text)
        self.assertIn("return Boolean(noteMutationViewModel.createEmptyNote());", controller_text)
        self.assertIn('if (reason.trimmed() != QStringLiteral("create-note"))', app_main_text)
        self.assertIn('QStringLiteral("navigation.NavigationAddNewBar")', app_main_text)
        self.assertIn('QStringLiteral("navigation.NavigationApplicationControlBar")', app_main_text)
        self.assertIn('QStringLiteral("navigation.NavigationApplicationViewBar")', app_main_text)
        self.assertIn('QStringLiteral("navigation.NavigationApplicationEditBar")', app_main_text)
        self.assertIn("global platform-native New shortcut", readme_text)
        self.assertIn("desktop-only ownership-aware New shortcut", readme_text)
        self.assertIn("Main.qml` binds the desktop-only New shortcut", architecture_text)
        self.assertIn("owned writable LVRS view ids", architecture_text)


if __name__ == "__main__":
    unittest.main()
