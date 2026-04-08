# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds to `ISystemCalendarStore` and its `systemInfoChanged` signal.
- Note-list date formatting behavior is unchanged.
- Note-list `primaryText` now comes from the shared `src/app/file/hierarchy/library/LibraryNotePreviewText.hpp`
  helper, so the library list and calendar note chips read from the same preview-text contract.
- Static `SystemCalendarStore::formatNoteDateForSystem(...)` remains the non-injected fallback helper.
- `indexedNotesSnapshot()` returns the current `m_indexedState.allNotes()` copy so other runtime collaborators such as
  `CalendarBoardStore` can project note lifecycle metadata from the already-loaded library snapshot.
- `setIndexedStateNotes(...)`, `applyIndexedStateSnapshot(...)`, and successful direct index loads now emit
  `indexedNotesSnapshotChanged()`, so calendar/runtime collaborators observe note-snapshot changes directly from the
  viewmodel instead of relying on a later page-open hook.
- `activateNoteById(...)` is now the canonical cross-surface note-open path. It first searches the currently visible
  library note list, then clears any active search filter, then falls back to the implicit `All Library` selection
  before selecting the requested note row.
- `createFolder()` remains the authoritative library-folder creation path. When a non-protected folder is selected, it
  computes the insertion point after that folder's subtree, increases depth by one, expands the parent, and therefore
  creates the new folder as a child of the selected folder.
- After persistence succeeds, `createFolder()` also moves the primary selected index to the inserted row so the QML
  sidebar can immediately activate and rename the new folder.
- `deleteSelectedFolder()` remains the authoritative delete path. It removes the selected folder together with its
  descendant subtree and persists the updated folders store before refreshing sidebar state.
- The library sidebar right-click context menu now reuses those two existing methods through
  `HierarchyInteractionBridge`; no separate library-specific CRUD implementation was added for the menu.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - Startup/deferred runtime note loads must emit `indexedNotesSnapshotChanged()` so calendar projections refresh
      before the user manually pokes the calendar surface.
    - `activateNoteById(...)` must select the requested note when it is already visible in the current library list.
    - An active library search filter must be cleared automatically when it hides the requested note.
    - A folder-scoped library selection must fall back to `All Library` before the activation path reports failure.
    - Failed activation must not switch the current note to an unrelated item.
