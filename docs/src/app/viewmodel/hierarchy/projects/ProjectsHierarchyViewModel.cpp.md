# `src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.cpp`

## Responsibility

This file implements the dedicated projects hierarchy viewmodel. It parses and mutates
`ProjectLists.wsproj`, builds the project sidebar rows, and keeps project selection stable across
runtime snapshot updates.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now treats watcher-driven updates conservatively.

- The current selection is captured by a stable project row key before any mutation.
- The incoming folder-depth entries are compared with the currently rendered project hierarchy.
- If the project hierarchy source changed, the viewmodel rebuilds project rows and restores the
  previous selection by key instead of dropping the user back to the implicit default state.
- Regardless of whether the hierarchy rows changed, the function re-indexes project note
  membership from live `.wsnhead` files so unchanged snapshots cannot keep stale note projection.
- `requestViewModelHook()` now also performs the same project note re-index path. This is used by
  sidebar entry/event hooks to force synchronization when the user re-enters the projects view.

Projects now exposes the same expansion hooks used by the shared sidebar footer. When a projects
snapshot eventually contains expandable rows, single-row and bulk expansion both mutate the
viewmodel-owned `expanded` flags and then rebuild the model so the footer menu can drive
`Expand All` / `Collapse All` without pushing that state into QML. If the current projects data is
flat, the footer menu stays disabled because no row advertises `showChevron: true`.

## Mutation Flow

- `loadFromWshub(...)` parses `ProjectLists.wsproj` into `WhatSonProjectsHierarchyStore` and also
  indexes `Library.wslibrary` so the projects domain has its own note-list projection.
- `renameItem(...)`, `createFolder()`, `deleteSelectedFolder()`, and reorder/move helpers mutate the
  store-backed folder entries and then rebuild the model.
- `setItemExpanded(...)` and `setAllItemsExpanded(...)` mutate only in-memory expansion state. They do
  not rewrite `Projects.wsproj`, because fold state is a sidebar presentation concern.
- `itemsFromProjectEntries(...)` is the translation boundary from persisted folder-depth records to
  sidebar rows.

## Note List Projection

- `noteListModel()` now returns a `LibraryNoteListModel` owned by the projects viewmodel instead of
  the inherited null default.
- Project-filtered note rows no longer pass the full note body into that shared note-list model.
  The model only receives preview/search metadata, while the editor fetches the selected note body lazily after
  selection changes.
- The note list is rebuilt from indexed `LibraryNoteRecord` entries, but project membership is
  synchronized again by reading each note header file before filtering.
- This keeps `.wsnhead <project>` as the only source of truth for Projects filtering, even when
  `index.wsnindex` still carries stale project labels.
- The same header synchronization runs at each note-list refresh, so switching project selection
  after an external `.wsnhead` edit cannot keep a stale project member visible.
- A view-triggered hook (`requestViewModelHook`) reuses the same refresh path, so projects note
  projection can be re-synced on sidebar entry without waiting for a new runtime snapshot.
- If an index row cannot be mapped to a readable note header, its project label is ignored for the
  Projects projection so index-only ghost rows do not leak into a selected project.
- When the hierarchy has visible rows, a negative or invalid selected index is normalized to the first visible row
  before the note list is rebuilt. Projects therefore enter with the same first-project filter that the sidebar marks
  as active, instead of falling back to an unfiltered list.
- `reloadNoteMetadataForNoteId(...)` now re-reads a single note document from disk and rebuilds the
  filtered projection immediately, so project assignment writes do not require a later hub reload
  before the projects note list catches up.
- `applyPersistedBodyStateForNote(...)` now mirrors normalized body text, RAW source text, first-line
  preview, and `lastModifiedAt` directly into the cached `m_allNotes` record, then refreshes the
  selected projects note list in memory. Structured correction paths use this to avoid an immediate
  second disk-backed metadata reload for the same note.
- Those body-only refresh paths now call `refreshNoteListForSelection(false)`, which reuses the
  already-cached per-note project labels instead of rescanning every `.wsnhead` file. Full header
  synchronization remains enabled for ordinary selection changes and full snapshot refreshes where
  project membership may actually have changed.
- That same lightweight apply path now also emits `hubFilesystemMutated()`, aligning Projects with the
  existing mutation notification contract used by other note-list-backed hierarchies.
- `requestTrackedStatisticsRefreshForNote(...)` now owns the `.wsnbody` scan previously triggered by the editor
  selection bridge for project-scoped note opens, then reuses `reloadNoteMetadataForNoteId(...)` to rehydrate the
  updated header fields back into the projects projection.
- When `reloadNoteMetadataForNoteId(...)` detects that a note's project label actually changed
  (for example `Untitled -> <empty>`), it triggers one extra full re-index pass from
  `ProjectLists.wsproj`/`Library.wslibrary` to guarantee that hierarchy badges and selected-project
  note projection stay consistent even if the previous in-memory note snapshot carried stale paths.
- `noteDirectoryPathForNoteId(...)` now resolves directory paths canonically by falling back to the
  readable `.wsnhead` location when the indexed directory path is missing or stale.
- If `reloadNoteMetadataForNoteId(...)` cannot re-read the note header anymore (for example the
  header file was removed), the viewmodel immediately clears that note's project membership in the
  in-memory projection and refreshes the note list, preventing stale project rows from lingering in
  the sidebar/list panel.

## Hierarchy Count Badge

- `depthItems()` now includes a numeric `count` value for each project row.
- The count is derived from the current in-memory indexed notes (`m_allNotes`) by case-insensitive
  match against the project label.
- Any runtime note reindex path (`refreshIndexedNotesFromWshub(...)`,
  `refreshIndexedNotesFromProjectsFilePath(...)`, `reloadNoteMetadataForNoteId(...)`) now emits
  `hierarchyModelChanged()` after refreshing note membership so sidebar count badges refresh
  without app restart.

## Invariants

- Selection is semantic and should survive runtime snapshot churn.
- A project snapshot that does not change the rendered hierarchy must not reset the sidebar state.
- If rows still exist after a rebuild, the effective selection must remain a visible row rather than an implicit
  "no filter" state.
