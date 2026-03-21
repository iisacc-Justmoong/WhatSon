from __future__ import annotations

import unittest
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]


class HubSyncControllerTests(unittest.TestCase):
    def test_runtime_sync_controller_is_wired_into_app_bootstrap(self) -> None:
        main_text = (REPO_ROOT / "src/app/main.cpp").read_text(encoding="utf-8")
        controller_header_text = (
            REPO_ROOT / "src/app/sync/WhatSonHubSyncController.hpp"
        ).read_text(encoding="utf-8")
        controller_impl_text = (
            REPO_ROOT / "src/app/sync/WhatSonHubSyncController.cpp"
        ).read_text(encoding="utf-8")
        library_header_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
        ).read_text(encoding="utf-8")
        bookmarks_header_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn('#include "sync/WhatSonHubSyncController.hpp"', main_text)
        self.assertIn("WhatSonHubSyncController hubSyncController;", main_text)
        self.assertIn("hubSyncController.attachToApplication(&app);", main_text)
        self.assertIn("hubSyncController.setReloadCallback(loadHubIntoRuntime);", main_text)
        self.assertIn("&LibraryHierarchyViewModel::hubFilesystemMutated", main_text)
        self.assertIn("&BookmarksHierarchyViewModel::hubFilesystemMutated", main_text)
        self.assertIn("hubSyncController.setCurrentHubPath(hubPath);", main_text)
        self.assertIn("hubSyncController.setCurrentHubPath(startupHubPath);", main_text)
        self.assertIn("Hub sync failed:", main_text)

        self.assertIn("class WhatSonHubSyncController final : public QObject", controller_header_text)
        self.assertIn("void requestSyncHint();", controller_header_text)
        self.assertIn("void acknowledgeLocalMutation();", controller_header_text)
        self.assertIn("void syncReloaded(const QString& hubPath);", controller_header_text)
        self.assertIn("void syncFailed(const QString& errorMessage);", controller_header_text)
        self.assertIn("QFileSystemWatcher m_fileSystemWatcher;", controller_header_text)
        self.assertIn("QTimer m_periodicTimer;", controller_header_text)
        self.assertIn("QTimer m_debounceTimer;", controller_header_text)

        self.assertIn("constexpr int kDefaultPeriodicIntervalMs = 5000;", controller_impl_text)
        self.assertIn("constexpr int kDefaultDebounceIntervalMs = 350;", controller_impl_text)
        self.assertIn("QDirIterator iterator(", controller_impl_text)
        self.assertIn("QCryptographicHash::Sha256", controller_impl_text)
        self.assertIn("QEvent::TouchBegin", controller_impl_text)
        self.assertIn("QEvent::MouseButtonPress", controller_impl_text)
        self.assertIn("emit syncReloaded(m_currentHubPath);", controller_impl_text)
        self.assertIn("emit syncFailed(", controller_impl_text)
        self.assertIn("if (m_localMutationPending)", controller_impl_text)

        self.assertIn("void hubFilesystemMutated();", library_header_text)
        self.assertIn("void hubFilesystemMutated();", bookmarks_header_text)

        self.assertIn("WhatSonHubSyncController", readme_text)
        self.assertIn("filesystem resync timer", readme_text)
        self.assertIn("WhatSonHubSyncController", architecture_text)
        self.assertIn("filesystem watcher", architecture_text)


if __name__ == "__main__":
    unittest.main()
