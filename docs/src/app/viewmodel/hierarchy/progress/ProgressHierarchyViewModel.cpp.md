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

## ViewModel Hook Contract

`requestViewModelHook()` now performs a real self-refresh when the progress file path is known.

- `reloadFromProgressFilePath(...)` reparses `Progress.wsprogress` and reapplies
  `setProgressState(...)`.
- The same hook reload also reindexes library notes via
  `refreshIndexedNotesFromProgressFilePath(...)`, so progress counts and filtered note rows update
  together.
- Reload errors surface as load-state failures while still emitting `viewModelHookRequested()` to
  preserve hook-notification sequencing.

## Note List Semantics

- `noteListModel()` exposes a `LibraryNoteListModel` so the progress domain can drive the shared
  note list and editor selection bridge.
- Progress-filtered note rows no longer carry full note bodies inside that shared note-list model.
  The progress domain keeps summary/search metadata only and relies on editor-side lazy loading for the selected note
  body.
- `selectedIndex` is treated as the canonical progress enum value because the sidebar rows are fixed
  to the current ten-item product taxonomy.
- When the hierarchy has visible rows, a negative or invalid selection is normalized to the first visible row
  before filtering. The current first progress bucket is `First draft`.
- `refreshNoteListForSelection()` keeps only notes whose `.wsnhead` `progress` integer matches the
  selected progress value, so the initial progress entry state now filters by that first bucket instead of
  showing every indexed note.
- `saveBodyTextForNote(...)`, `saveCurrentBodyText(...)`, and `noteDirectoryPathForNoteId(...)`
  keep the progress domain compatible with existing note editing and detail-panel current-note
  wiring.
- Successful body saves now emit `hubFilesystemMutated()`, aligning progress note writes with hub sync baseline
  acknowledgement logic already used by other note-editable hierarchy domains.
- Editor autosave can now mirror normalized body state immediately through `applyPersistedBodyStateForNote(...)` and
  leave the expensive `.wsnbody` backlink/open-count scan to `requestTrackedStatisticsRefreshForNote(...)`.
- `reloadNoteMetadataForNoteId(...)` re-reads one indexed note from disk and re-applies the active
  progress filter so a detail-panel progress write can immediately remove or keep the note in the
  visible list according to its new enum value.

## Hierarchy Count Badge

- `depthItems()` now includes a `count` value per progress row.
- The badge value is computed from indexed note metadata by counting notes whose `progress` integer
  equals the row index.
- Runtime metadata refresh paths (`refreshIndexedNotesFromWshub(...)`,
  `refreshIndexedNotesFromProgressFilePath(...)`, `reloadNoteMetadataForNoteId(...)`) now emit
  `hierarchyModelChanged()` after note refresh so progress counts are not stale until restart.

## Intentional Constraints

- Rename, create, and delete remain disabled because progress rows are product-defined support
  buckets rather than user-editable folders.
- The first four rows still expose chevrons so the viewmodel can preserve Figma-aligned expansion
  state even though no child nodes are materialized yet.

## Regression Checks

- Entering the progress hierarchy must keep the active sidebar row and the note-list filter aligned on the
  first visible bucket.
- Runtime payload refresh must preserve that normalized first-row fallback whenever rows still exist.
