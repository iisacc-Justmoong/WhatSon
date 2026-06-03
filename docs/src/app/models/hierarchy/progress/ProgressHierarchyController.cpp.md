# `src/app/models/hierarchy/progress/ProgressHierarchyController.cpp`

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

## Controller Hook Contract

`requestControllerHook()` now performs a real self-refresh when the progress file path is known.

- `reloadFromProgressFilePath(...)` reparses `Progress.wsprogress` and reapplies
  `setProgressState(...)`.
- The same hook reload also reindexes library notes via
  `refreshIndexedNotesFromProgressFilePath(...)`, so progress counts and filtered note rows update
  together.
- Reload errors surface as load-state failures while still emitting `controllerHookRequested()` to
  preserve hook-notification sequencing.

## Note List Semantics

- `noteListModel()` exposes a `LibraryNoteListModel` so the progress domain can drive the shared
  note list and editor selection bridge.
- Progress-filtered note rows now keep the selected note's RAW/source snapshot in `LibraryNoteListItem::bodyText`.
  The list still renders preview/search metadata only, but the editor selection bridge can now bootstrap the current
  note body from the progress note-list row itself before it needs an async reload.
- `selectedIndex` is treated as the canonical progress enum value because the sidebar rows are fixed
  to the current ten-item product taxonomy.
- When the hierarchy has visible rows, a negative or invalid selection is normalized to the first visible row
  before filtering. The current first progress bucket is `First draft`.
- `refreshNoteListForSelection()` keeps only notes whose `.wsnhead` `progress` integer matches the
  selected progress value, so the initial progress entry state now filters by that first bucket instead of
  showing every indexed note.
- `noteDirectoryPathForNoteId(...)` keeps the progress domain compatible with detail-panel current-note wiring.
- Body save, body-state apply, editor stat-refresh, and single-note metadata reload APIs were removed with the note
  editor/save boundary.

## Hierarchy Count Badge

- `depthItems()` now includes a `count` value per progress row.
- The badge value is computed from indexed note metadata by counting notes whose `progress` integer
  equals the row index.
- Runtime metadata refresh paths (`refreshIndexedNotesFromWshub(...)`,
  `refreshIndexedNotesFromProgressFilePath(...)`) now emit
  `hierarchyModelChanged()` after note refresh so progress counts are not stale until restart.

## Intentional Constraints

- Rename, create, and delete remain disabled because progress rows are product-defined support
  buckets rather than user-editable folders.
- The first four rows still expose chevrons so the controller can preserve Figma-aligned expansion
  state even though no child nodes are materialized yet. `setItemExpanded(...)` delegates the shared row validation/state
  flip to `IHierarchyController` before syncing this projection.

## Regression Checks

- Entering the progress hierarchy must keep the active sidebar row and the note-list filter aligned on the
  first visible bucket.
- Runtime payload refresh must preserve that normalized first-row fallback whenever rows still exist.
