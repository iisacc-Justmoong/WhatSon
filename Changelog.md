# Standup Changelog

## 2026-03-08 (KST)

### Snapshot

- Commits: 16 (`1b4052e` .. `34bd214`)
- Files touched: 69 unique paths (157 file-change entries)
- Diff volume: +7,628 / -1,981 lines
- PR references in commit subjects: none

### Team-Ready Summary (Anchored)

- Added `Presentation` editor mode and wired navigation updates.
    - Commits: `1b4052e`, `aae1ebb`
    - Files: `src/app/viewmodel/navigationbar/EditorViewModeViewModel.cpp`,
      `src/app/viewmodel/navigationbar/EditorViewState.cpp`,
      `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml`
- Added folder drag-and-drop plus operation validation and hierarchy normalization logic.
    - Commits: `f49021b`, `2e786b9`, `306c0df`
    - Files: `src/app/viewmodel/hierarchy/library/LibraryHierarchyModel.cpp`,
      `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`,
      `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`
- Refined note list rendering and placeholder/resolved date behavior.
    - Commits: `f3e0294`, `3530532`, `6988f7a`
    - Files: `src/app/qml/view/panels/NoteListItem.qml`, `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml`
- Enabled note-list search on parsed note body text and expanded hierarchy/bookmark integration.
    - Commits: `62be9b2`, `ec86f6a`, `4fad98a`, `e99e63a`
    - Files: `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.cpp`,
      `src/app/viewmodel/hierarchy/bookmarks/BookmarksNoteListModel.cpp`,
      `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp`
- Migrated list placeholder behavior into editor content flow and removed duplicated panel logic.
    - Commit: `246fad3`
    - Files: `src/app/qml/view/content/editor/ContentsDisplayView.qml`,
      `src/app/qml/view/panels/ListItemsPlaceholder.qml`, `src/app/qml/view/panels/ContentViewLayout.qml`
- Introduced localized system calendar/date formatting store and connected viewmodels/UI.
    - Commit: `26f187c`
    - Files: `src/app/calendar/SystemCalendarStore.cpp`, `src/app/qml/view/panels/ListBarHeader.qml`,
      `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`
- Added permission-bridge misuse safeguards with dedicated tests.
    - Commit: `34bd214`
    - Files: `src/app/permissions/ApplePermissionBridge.mm`, `tests/app/test_permission_bridge_source_guard.cpp`

### Full Commit List (Chronological)

- `1b4052e` Add `Presentation` mode support to `EditorViewModeViewModel`
- `aae1ebb` Add `Presentation` mode support to `EditorViewModeViewModel`
- `f3e0294` Add placeholder and resolved date support for `NoteListItem.qml`
- `f49021b` Add drag-and-drop support for folders in the library sidebar
- `2e786b9` Add folder operation checks and placeholder date logic
- `3530532` Refactor `NoteListItem.qml` and enhance styling consistency across metadata and primary text
- `6988f7a` Sync toolbar layout and icon specifications to Figma frame standards
- `e9b4061` Add macOS-native menu bar and fix binding issues in `NoteListItem.qml`
- `62be9b2` Enable note-list search against pre-parsed body content and optimize focus-dismiss handling
- `306c0df` Standardize untitled labels, folder operation hooks, path normalization, and content-view sync
- `ec86f6a` Add `BookmarksNoteListModel` and support for bookmarks note search
- `4fad98a` Add `BookmarksNoteListModel` and support for bookmarks note search
- `e99e63a` Enable developer tooling and refactor `LibraryHierarchyViewModel` for text persistence
- `246fad3` Migrate `ListItemsPlaceholder.qml` logic to `ContentsDisplayView.qml`
- `26f187c` Introduce `SystemCalendarStore` for localized date/time formatting metadata
- `34bd214` Add tests and safeguards for `traceSelf(this, ...)` misuse in permission bridges
