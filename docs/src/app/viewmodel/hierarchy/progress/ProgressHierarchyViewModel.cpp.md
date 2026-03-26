# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.cpp`

## Responsibility

This implementation turns `Progress.wsprogress` state labels into the sidebar rows for the progress
domain and materializes the matching note list from `.wsnhead` metadata.

## Runtime Data Flow

- `loadFromWshub(...)` resolves the active `Progress.wsprogress`, parses its state list, and then
  indexes the hub library through `LibraryAll`.
- `applyRuntimeSnapshot(...)` refreshes the same state list from the startup snapshot and reindexes
  notes by resolving the enclosing `.wshub` from the snapshot file path.
- `setProgressState(...)` sanitizes the state labels, falls back to the default four-state set when
  needed, rebuilds the visible rows, and immediately reapplies note filtering.

## Note List Semantics

- `noteListModel()` exposes a `LibraryNoteListModel` so the progress domain can drive the shared
  note list and editor selection bridge.
- `selectedIndex` is treated as the canonical progress enum value because the row order now mirrors
  the order defined in `Progress.wsprogress`.
- `refreshNoteListForSelection()` keeps only notes whose `.wsnhead` `progress` integer matches the
  selected progress value. When no row is selected, all indexed notes remain visible.
- `saveBodyTextForNote(...)`, `saveCurrentBodyText(...)`, and `noteDirectoryPathForNoteId(...)`
  keep the progress domain compatible with existing note editing and detail-panel current-note
  wiring.

## Intentional Constraints

- Rename, create, delete, and folder expansion remain disabled because progress states are treated
  as a flat taxonomy owned by `Progress.wsprogress`, not as mutable hierarchy folders in the view.
