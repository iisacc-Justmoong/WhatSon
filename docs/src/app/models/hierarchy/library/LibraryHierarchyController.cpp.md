# `src/app/models/hierarchy/library/LibraryHierarchyController.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds to `ISystemCalendarStore` and its `systemInfoChanged` signal.
- Note-list date formatting behavior is unchanged.
- Note-list `primaryText` now comes from the shared `src/app/models/hierarchy/library/LibraryNotePreviewText.hpp`
  helper, so the library list and calendar note chips read from the same preview-text contract.
- Static `SystemCalendarStore::formatNoteDateForSystem(...)` remains the non-injected fallback helper.
- `indexedNotesSnapshot()` returns the current `m_indexedState.allNotes()` copy so other runtime collaborators such as
  `CalendarBoardStore` can project note lifecycle metadata from the already-loaded library snapshot.
- Local note mutation paths now split into two classes, with persistence mutations disabled:
  - runtime-load / explicit full-snapshot paths still go through `setIndexedStateNotes(...)`,
    `applyIndexedStateSnapshot(...)`, and `loadIndexedStateFromWshub(...)`
  - note-save / metadata-reload / folder-assign / note-create / note-delete / folder-clear paths now fail closed and
    leave indexed note state unchanged
- Those note-distribution mutation paths now also route through
  `refreshNoteListForSelectionAndNotifyHierarchyModel()`, which is the guard against stale sidebar `count` labels
  when only the note-to-folder distribution changed and the hierarchy row vector stayed byte-for-byte identical.
- `setIndexedStateNotes(...)`, `applyIndexedStateSnapshot(...)`, and successful direct index loads now emit
  `indexedNotesSnapshotChanged()`, so calendar/runtime collaborators observe note-snapshot changes directly from the
  controller instead of relying on a later page-open hook.
- `upsertIndexedNote(...)` now invalidates only the affected note-list cache entry and emits `indexedNoteUpserted(...)`
  when the underlying indexed-state mutation actually changed the note payload.
- `activateNoteById(...)` is now the canonical cross-surface note-open path. It first searches the currently visible
  library note list, then clears any active search filter, then falls back to the implicit `All Library` selection
  before selecting the requested note row.
- Library note-list row projection now carries the normalized note body source again.
  The controller still derives `primaryText` / `searchableText` from indexed note metadata, but it no longer clears
  `bodyText` before rows are pushed into the shared note-list model. This keeps the selected-note body available to
  the editor even when the asynchronous lazy-load path is late or temporarily unavailable.
- The note-list row projection now also carries `noteDirectoryPath`, and the controller's internal note-list cache key
  is no longer `noteId` alone.
  Cache invalidation and row reuse now treat `noteId + noteDirectoryPath` as the stable row identity.
- The library runtime snapshot exposes `noteBodySourceTextForNoteId(...)` for read-side consumers that need the already
  loaded indexed note source.
- Note body persistence and tracked-stat refresh are currently disabled at the controller boundary.
- `createFolder()` remains the authoritative library-folder creation path. When a non-protected folder is selected, it
  computes the insertion point after that folder's subtree and increases depth by one, creating the new folder as a
  child of the selected folder without forcing the selected parent open.
- After persistence succeeds, `createFolder()` also moves the primary selected index to the inserted row so the QML
  sidebar can immediately activate and rename the new folder.
- `setDepthItems(...)` preserves expansion by stable hierarchy key before replacing the row vector. External folder
  structure refreshes may add, remove, or reorder rows, but they must not change the expansion state of surviving
  folders.
- Library hierarchy always starts from the hub-independent in-app scaffold: `All Library`, `Drafts`, and `Today`.
  These rows are app-owned system buckets, not hub-authored folder rows. Constructor setup, load-failure recovery,
  empty depth input, and empty-folder runtime snapshots must keep those three fixed rows visible before reporting an
  unchanged hierarchy source.
- `setItemExpanded(...)` delegates the shared row validation/state flip to `IHierarchyController`'s protected expansion
  helper and then updates the shared `WhatSonHierarchyModel` row through `setItemExpanded(...)`. Protected-root policy
  remains a rename/delete guard; it must not block a visible chevron from folding or unfolding its descendants.
- General selection writes still normalize a negative or invalid selected index to the first visible row before the
  controller republishes state. This keeps the library note-list filter aligned with the row the sidebar already renders
  as active.
- `deleteSelectedFolder()` remains the authoritative delete path. It removes the selected folder together with its
  descendant subtree and persists the updated folders store before refreshing sidebar state. Deleting the focused folder
  is the explicit exception to selection normalization: the controller leaves `selectedIndex == -1` so no surviving
  hierarchy row inherits focus just because it is adjacent or last.
- The library sidebar right-click context menu now reuses those two existing methods through
  `HierarchyInteractionBridge`; no separate library-specific CRUD implementation was added for the menu.
- `applyHierarchyMove(...)` remains a targeted move helper. The sidebar drag/drop path normally persists the final
  `LV.Hierarchy.model` snapshot through `applyHierarchyNodes(...)`, while explicit callers can still use the targeted
  helper to resolve one source subtree and persist it through the same folder hierarchy commit path.
- After a folder hierarchy mutation has been persisted, the controller reparses `Folders.wsfolders` and rebuilds the
  live library rows from that file-backed tree. The staged drag/drop vector is only a mutation proposal; the view model
  mirrors the persisted `.wsfolders` result so store-side normalization, migration, or filtered rows cannot leave a
  stale sidebar shape.
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
  - body saves must be able to mirror normalized body text into the in-memory library note immediately without forcing
    the same save path to rescan every `.wsnbody` in the hub
  - create/delete/folder-clear mutation flows must prefer single-note upsert/remove and only fall back to full
    snapshot replacement when the service result cannot resolve the target note
  - library folder/system-bucket sidebar counters must refresh immediately after folder assignment, note create,
    note delete, folder clear, or one-note metadata reload even when the hierarchy rows themselves did not rebuild
  - folder-structure reloads and folder creation must not expand or collapse existing library rows unless the user
    explicitly performed an expansion command
  - constructor setup, load-failure recovery, empty depth input, and an empty-folder runtime snapshot from a new hub
    must still publish `All Library`, `Drafts`, and `Today` before any user-authored folders exist
  - accent root folders with visible chevrons must accept targeted `setItemExpanded(...)` changes even though the same
    rows remain protected from rename/delete mutations
  - `activateNoteById(...)` must select the requested note when it is already visible in the current library list.
  - An active library search filter must be cleared automatically when it hides the requested note.
  - A folder-scoped library selection must fall back to `All Library` before the activation path reports failure.
  - Failed activation must not switch the current note to an unrelated item.
  - The shared library note-list model must keep the selected note body available through `BodyTextRole` /
    `currentBodyText` so editor selection never collapses to an empty document after runtime snapshot refreshes.
  - The shared library note-list model must also export the selected row's `noteDirectoryPath` for identity
    disambiguation.
  - Direct note-package resolution is currently disabled, so editor bridges must not depend on it.
  - A folder label such as `Marketing/Sales` must remain one hierarchy item after parse/load/save cycles.
  - The same literal-slash folder label must not surface as `Marketing\\/Sales` in note-list folder presentation.
  - Dragging one library folder onto another through the LVRS move event must persist exactly one nested subtree entry
    and must not leave a duplicate top-level sibling in the controller model.
  - After any hierarchy commit, the live library model must be rebuilt from the persisted `Folders.wsfolders` tree,
    not from transient staged rows that the folder store did not serialize.
