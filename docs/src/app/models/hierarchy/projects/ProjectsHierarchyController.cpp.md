# `src/app/models/hierarchy/projects/ProjectsHierarchyController.cpp`

## Responsibility

This file implements the dedicated projects hierarchy controller. It parses and mutates
`ProjectLists.wsproj`, builds the project sidebar rows, and keeps project selection stable across
runtime snapshot updates.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now treats watcher-driven updates conservatively.

- The current selection is captured by a stable project row key before any mutation.
- The incoming folder-depth entries are compared with the currently rendered project hierarchy.
- If the project hierarchy source changed, the controller rebuilds project rows and restores the
  previous selection by key instead of dropping the user back to the implicit default state.
- Regardless of whether the hierarchy rows changed, the function re-indexes project note
  membership from live `.wsnhead` files so unchanged snapshots cannot keep stale note projection.
- `requestControllerHook()` now also performs the same project note re-index path. This is used by
  sidebar entry/event hooks to force synchronization when the user re-enters the projects view.

Projects now exposes the same expansion hooks used by the shared sidebar footer. When a projects
snapshot eventually contains expandable rows, single-row and bulk expansion both mutate the
controller-owned `expanded` flags and then rebuild the model so the footer menu can drive
`Expand All` / `Collapse All` without pushing that state into QML. If the current projects data is
flat, the footer menu stays disabled because no row advertises `showChevron: true`.

## Mutation Flow

- `loadFromWshub(...)` parses `ProjectLists.wsproj` into `WhatSonProjectsHierarchyStore` and also
  indexes `Library.wslibrary` so the projects domain has its own note-list projection.
- `renameItem(...)`, `createFolder()`, `deleteSelectedFolder()`, and reorder/move helpers mutate the
  store-backed folder entries and then rebuild the model.
- `applyHierarchyMove(...)` remains available for explicit targeted project-folder moves. The sidebar's ordinary LVRS
  drag/drop commit persists the final `LV.Hierarchy.model` snapshot through full-node replay.
- `setItemExpanded(...)` and `setAllItemsExpanded(...)` delegate shared chevron validation/state flips to
  `IHierarchyController`'s protected helpers, then sync only the in-memory model. They do not rewrite
  `Projects.wsproj`, because fold state is a sidebar presentation concern.
- `itemsFromProjectEntries(...)` is the translation boundary from persisted folder-depth records to
  sidebar rows.

## Note List Projection

- `noteListModel()` now returns a `LibraryNoteListModel` owned by the projects controller instead of
  the inherited null default.
- Project-filtered note rows now keep the selected note's RAW/source snapshot in `LibraryNoteListItem::bodyText`.
  The visible list surface still renders preview/search metadata, but the editor selection bridge can bootstrap RAW
  body text immediately from the current project row before the async note-body refresh path completes.
- The note list is rebuilt from indexed `LibraryNoteRecord` entries, but project membership is
  synchronized again by reading each note header file before filtering.
- This keeps `.wsnhead <project>` as the only source of truth for Projects filtering, even when
  `index.wsnindex` still carries stale project labels.
- The same header synchronization runs at each note-list refresh, so switching project selection
  after an external `.wsnhead` edit cannot keep a stale project member visible.
- A view-triggered hook (`requestControllerHook`) reuses the same refresh path, so projects note
  projection can be re-synced on sidebar entry without waiting for a new runtime snapshot.
- If an index row cannot be mapped to a readable note header, its project label is ignored for the
  Projects projection so index-only ghost rows do not leak into a selected project.
- When the hierarchy has visible rows, a negative or invalid selected index is normalized to the first visible row
  before the note list is rebuilt. Projects therefore enter with the same first-project filter that the sidebar marks
  as active, instead of falling back to an unfiltered list.
- Project note projection now stays metadata-only. Body-state apply paths and editor-triggered statistic refresh APIs
  were removed with the note editor/save boundary.
- Full header synchronization remains enabled for ordinary selection changes and full snapshot refreshes where project
  membership may actually have changed.
- `noteDirectoryPathForNoteId(...)` now resolves directory paths canonically by falling back to the
  readable `.wsnhead` location when the indexed directory path is missing or stale.

## Hierarchy Count Badge

- `depthItems()` now includes a numeric `count` value for each project row.
- The count is derived from the current in-memory indexed notes (`m_allNotes`) by case-insensitive
  match against the project label.
- Any runtime note reindex path (`refreshIndexedNotesFromWshub(...)`,
  `refreshIndexedNotesFromProjectsFilePath(...)`) now emits
  `hierarchyModelChanged()` after refreshing note membership so sidebar count badges refresh
  without app restart.

## Invariants

- Selection is semantic and should survive runtime snapshot churn.
- A project snapshot that does not change the rendered hierarchy must not reset the sidebar state.
- If rows still exist after a rebuild, the effective selection must remain a visible row rather than an implicit
  "no filter" state.
