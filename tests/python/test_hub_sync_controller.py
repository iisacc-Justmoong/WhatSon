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
        runtime_loader_text = (
            REPO_ROOT / "src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp"
        ).read_text(encoding="utf-8")
        runtime_snapshots_text = (
            REPO_ROOT / "src/app/runtime/threading/WhatSonRuntimeDomainSnapshots.cpp"
        ).read_text(encoding="utf-8")
        library_header_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.hpp"
        ).read_text(encoding="utf-8")
        library_impl_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")
        bookmarks_header_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp"
        ).read_text(encoding="utf-8")
        bookmarks_impl_text = (
            REPO_ROOT / "src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp"
        ).read_text(encoding="utf-8")
        readme_text = (REPO_ROOT / "README.md").read_text(encoding="utf-8")
        architecture_text = (REPO_ROOT / "docs/APP_ARCHITECTURE.md").read_text(encoding="utf-8")

        self.assertIn('#include "sync/WhatSonHubSyncController.hpp"', main_text)
        self.assertIn("WhatSonHubSyncController hubSyncController;", main_text)
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
        self.assertIn("struct HubObservation final", controller_header_text)
        self.assertIn("QStringList m_lastObservedWatchPaths;", controller_header_text)
        self.assertIn("QStringList m_appliedWatchPaths;", controller_header_text)

        self.assertIn("constexpr int kDefaultPeriodicIntervalMs = 5000;", controller_impl_text)
        self.assertIn("constexpr int kDefaultDebounceIntervalMs = 350;", controller_impl_text)
        self.assertIn("QDirIterator iterator(", controller_impl_text)
        self.assertIn("QCryptographicHash::Sha256", controller_impl_text)
        self.assertIn("inspectHub(", controller_impl_text)
        self.assertIn("normalizeWatchPaths", controller_impl_text)
        self.assertIn("if (watchPaths == m_appliedWatchPaths)", controller_impl_text)
        self.assertNotIn("computeHubSignature", controller_impl_text)
        self.assertNotIn("collectWatchPaths", controller_impl_text)
        self.assertNotIn("attachToApplication", controller_header_text)
        self.assertNotIn("eventFilter(", controller_header_text)
        self.assertNotIn("handleApplicationStateChanged", controller_header_text)
        self.assertNotIn("QEvent::TouchBegin", controller_impl_text)
        self.assertNotIn("QEvent::MouseButtonPress", controller_impl_text)
        self.assertIn("shouldIgnoreObservedRelativePath", controller_impl_text)
        self.assertIn('QStringLiteral(".whatson/write-lease.json")', controller_impl_text)
        self.assertIn("emit syncReloaded(m_currentHubPath);", controller_impl_text)
        self.assertIn("emit syncFailed(", controller_impl_text)
        self.assertIn("if (m_localMutationPending)", controller_impl_text)

        self.assertIn("void hubFilesystemMutated();", library_header_text)
        self.assertIn("void hubFilesystemMutated();", bookmarks_header_text)
        self.assertIn("WhatSonLibraryIndexedState", library_header_text)
        self.assertIn("m_noteListItemCache", library_header_text)
        self.assertIn("m_indexedState.setIndexedNotes", library_impl_text)
        self.assertIn("buildFolderScopedNoteListItems", library_impl_text)
        self.assertIn("invalidateNoteListItemCache", library_impl_text)
        self.assertIn("WhatSonLibraryIndexedState", bookmarks_impl_text)
        self.assertIn("collectBookmarkedNotes", bookmarks_impl_text)

        self.assertIn("deriveBookmarksFromLibrary", runtime_loader_text)
        self.assertIn("buildBookmarks(librarySnapshot.allNotes)", runtime_loader_text)
        self.assertIn("WhatSonLibraryIndexedState indexedState;", runtime_snapshots_text)
        self.assertIn("buildBookmarks(", runtime_snapshots_text)

        self.assertIn("WhatSonHubSyncController", readme_text)
        self.assertIn("filesystem watcher", readme_text)
        self.assertIn("without UI interaction hints", readme_text)
        self.assertIn("single recursive observation pass", readme_text)
        self.assertIn("derive bookmarks from the shared library snapshot", readme_text)
        self.assertIn("WhatSonHubSyncController", architecture_text)
        self.assertIn("filesystem watcher", architecture_text)
        self.assertIn("single recursive observation pass", architecture_text)


if __name__ == "__main__":
    unittest.main()
