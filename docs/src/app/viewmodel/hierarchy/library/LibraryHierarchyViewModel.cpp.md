# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds to `ISystemCalendarStore` and its `systemInfoChanged` signal.
- Note-list date formatting behavior is unchanged.
- Note-list `primaryText` now comes from the shared `src/app/file/hierarchy/library/LibraryNotePreviewText.hpp`
  helper, so the library list and calendar note chips read from the same preview-text contract.
- Static `SystemCalendarStore::formatNoteDateForSystem(...)` remains the non-injected fallback helper.
- `indexedNotesSnapshot()` returns the current `m_indexedState.allNotes()` copy so other runtime collaborators such as
  `CalendarBoardStore` can project note lifecycle metadata from the already-loaded library snapshot.
- Local note mutation paths now split into two classes:
  - runtime-load / explicit full-snapshot paths still go through `setIndexedStateNotes(...)`,
    `applyIndexedStateSnapshot(...)`, and `loadIndexedStateFromWshub(...)`
  - note-save / metadata-reload / folder-assign / note-create / note-delete / folder-clear paths now prefer one-note
    `upsertIndexedNote(...)` / `removeIndexedNoteById(...)` updates and only fall back to full snapshot replacement
    when the mutation service does not return a resolvable target note
- Those note-distribution mutation paths now also route through
  `refreshNoteListForSelectionAndNotifyHierarchyModel()`, which is the guard against stale sidebar `count` labels
  when only the note-to-folder distribution changed and the hierarchy row vector stayed byte-for-byte identical.
- `setIndexedStateNotes(...)`, `applyIndexedStateSnapshot(...)`, and successful direct index loads now emit
  `indexedNotesSnapshotChanged()`, so calendar/runtime collaborators observe note-snapshot changes directly from the
  viewmodel instead of relying on a later page-open hook.
- `upsertIndexedNote(...)` now invalidates only the affected note-list cache entry and emits `indexedNoteUpserted(...)`
  when the underlying indexed-state mutation actually changed the note payload.
- `activateNoteById(...)` is now the canonical cross-surface note-open path. It first searches the currently visible
  library note list, then clears any active search filter, then falls back to the implicit `All Library` selection
  before selecting the requested note row.
- Library note-list row projection no longer carries the full note body.
  The viewmodel still derives `primaryText` / `searchableText` from indexed note metadata, but `bodyText` is now
  cleared before rows are pushed into the shared note-list model so large notes do not get duplicated into selection
  state.
- The library runtime snapshot now also exposes `noteBodySourceTextForNoteId(...)`.
  `ContentsEditorSelectionBridge` can therefore recover the selected note source from the already-loaded indexed note
  snapshot when direct package-path resolution fails for one desktop selection turn.
- Editor persistence is now split into two viewmodel-facing phases:
  - `applyPersistedBodyStateForNote(...)` mutates only the in-memory indexed note/body preview immediately after a
    successful direct file-store write.
  - `requestTrackedStatisticsRefreshForNote(...)` later pays the `.wsnbody` scan, rewrites tracked header stats, and
    rehydrates the same note through `reloadNoteMetadataForNoteId(...)`.
- `createFolder()` remains the authoritative library-folder creation path. When a non-protected folder is selected, it
  computes the insertion point after that folder's subtree, increases depth by one, expands the parent, and therefore
  creates the new folder as a child of the selected folder.
- After persistence succeeds, `createFolder()` also moves the primary selected index to the inserted row so the QML
  sidebar can immediately activate and rename the new folder.
- When the hierarchy has visible rows, a negative or invalid selected index is normalized to the first visible row
  before the viewmodel republishes state. This keeps the library note-list filter aligned with the row the sidebar
  already renders as active.
- `deleteSelectedFolder()` remains the authoritative delete path. It removes the selected folder together with its
  descendant subtree and persists the updated folders store before refreshing sidebar state.
- The library sidebar right-click context menu now reuses those two existing methods through
  `HierarchyInteractionBridge`; no separate library-specific CRUD implementation was added for the menu.
- Folder-path normalization now uses the shared escaped-segment semantics from `WhatSonNoteFolderSemantics.hpp`.
  A folder label that literally contains `/` is persisted as one escaped segment (`\/`) and is no longer split into
  fake parent/child hierarchy rows.
- Note-list folder chips/search text now decode those escaped segments back into user-facing text, so the library list
  does not expose persistence escape markers like `\/`.

## Tests
- The maintained C++ regression suite now also covers escaped-slash folder-path semantics and parser migration for
  literal-slash library folder labels.
- Regression checklist:
  - Startup/deferred runtime note loads must emit `indexedNotesSnapshotChanged()` so calendar projections refresh
    before the user manually pokes the calendar surface.
  - local single-note mutations such as `saveBodyTextForNote(...)` and `reloadNoteMetadataForNoteId(...)` must not
    require copying/replacing the full `allNotes` vector
  - editor autosave must be able to mirror normalized body text into the in-memory library note immediately without
    forcing the same save path to rescan every `.wsnbody` in the hub
  - create/delete/folder-clear mutation flows must prefer single-note upsert/remove and only fall back to full
    snapshot replacement when the service result cannot resolve the target note
  - library folder/system-bucket sidebar counters must refresh immediately after folder assignment, note create,
    note delete, folder clear, or one-note metadata reload even when the hierarchy rows themselves did not rebuild
  - `activateNoteById(...)` must select the requested note when it is already visible in the current library list.
  - An active library search filter must be cleared automatically when it hides the requested note.
  - A folder-scoped library selection must fall back to `All Library` before the activation path reports failure.
  - Failed activation must not switch the current note to an unrelated item.
  - Large library notes must not be duplicated into note-list row `bodyText`; the editor must lazy-load the selected
    note body separately.
  - If direct note-package resolution is temporarily unavailable, the indexed library snapshot must still be able to
    provide the selected note's body source to the editor bridge.
  - A folder label such as `Marketing/Sales` must remain one hierarchy item after parse/load/save cycles.
  - The same literal-slash folder label must not surface as `Marketing\\/Sales` in note-list folder presentation.
