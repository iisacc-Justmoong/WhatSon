# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.cpp`

## Responsibility

This implementation keeps the progress sidebar on the fixed ten-row LVRS taxonomy while
materializing the matching note list from `.wsnhead` metadata.

## Runtime Data Flow

- `loadFromWshub(...)` resolves the active `Progress.wsprogress`, preserves its payload, and then
  indexes the hub library through `LibraryAll`.
- `applyRuntimeSnapshot(...)` refreshes the same payload from the startup snapshot and reindexes
  notes by resolving the enclosing `.wshub` from the snapshot file path.
- `setProgressState(...)` keeps the persisted payload synchronized while rebuilding the fixed
  ten-row sidebar taxonomy and immediately reapplies note filtering.

## Note List Semantics

- `noteListModel()` exposes a `LibraryNoteListModel` so the progress domain can drive the shared
  note list and editor selection bridge.
- `selectedIndex` is treated as the canonical progress enum value because the sidebar rows are fixed
  to the current ten-item product taxonomy.
- `refreshNoteListForSelection()` keeps only notes whose `.wsnhead` `progress` integer matches the
  selected progress value. When no row is selected, all indexed notes remain visible.
- `saveBodyTextForNote(...)`, `saveCurrentBodyText(...)`, and `noteDirectoryPathForNoteId(...)`
  keep the progress domain compatible with existing note editing and detail-panel current-note
  wiring.

## Intentional Constraints

- Rename, create, and delete remain disabled because progress rows are product-defined support
  buckets rather than user-editable folders.
- The first four rows still expose chevrons so the viewmodel can preserve Figma-aligned expansion
  state even though no child nodes are materialized yet.
