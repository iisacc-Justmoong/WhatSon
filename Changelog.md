# Standup Changelog

## 2026-03-09 (KST)

### Snapshot

- Commits: 6 (`e92e21c` .. `7e82210`)
- Files touched: 16 unique paths (25 file-change entries)
- Diff volume: +496 / -234 lines
- PR references in commit subjects: none

### Team-Ready Summary (Anchored)

- Added the standup changelog baseline file.
    - Commit: `e92e21c`
    - Files: `Changelog.md`, `src/app/CMakeLists.txt`, `src/app/qml/view/content/editor/ContentsDisplayView.qml`
- Fixed repeated QML binding and property-source regressions in editor/list panel views.
    - Commits: `9da8ec1`, `beb81ef`, `3a6a72d`, `7e82210`
    - Files: `src/app/qml/view/content/editor/ContentsDisplayView.qml`,
      `src/app/qml/view/panels/NoteListItem.qml`, `src/app/qml/view/panels/ListBarHeader.qml`
- Split editor surface responsibilities into dedicated layer components and aligned related guard/test docs.
    - Commit: `f98cf69`
    - Files: `src/app/qml/view/content/editor/ContentsGutterLayer.qml`,
      `src/app/qml/view/content/editor/ContentsMinimapLayer.qml`,
      `src/app/qml/view/content/editor/ContentsDrawerSplitter.qml`,
      `tests/app/test_qml_binding_syntax_guard.cpp`, `docs/APP_ARCHITECTURE.md`, `AGENTS.md`
- Updated Apple platform packaging metadata alongside QML integration refactors.
    - Commit: `3a6a72d`
    - Files: `platform/Apple/Info.plist`, `src/app/CMakeLists.txt`

### Full Commit List (Chronological)

- `e92e21c` Add `Changelog.md` with team-ready summaries and totals
- `9da8ec1` Fix bindings in QML components and correct property usage
- `beb81ef` Fix QML property bindings and correct `source` usage
- `f98cf69` Modularize editor components with dedicated layers for gutter, minimap, and drawer splitter
- `3a6a72d` Refactor QML integrations and conditional configurations
- `7e82210` Refactor QML integrations and conditional configurations

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
