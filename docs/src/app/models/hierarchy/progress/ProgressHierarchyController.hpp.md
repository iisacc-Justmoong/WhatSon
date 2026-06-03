# `src/app/models/hierarchy/progress/ProgressHierarchyController.hpp`

## Responsibility

This header declares the progress hierarchy controller that backs the progress sidebar and its note
list.

## Public Contract

- Publishes both `itemModel` and `noteListModel`, so the progress domain exposes sidebar rows and
  filtered notes through the same hierarchy interface used by other domains.
- Exposes `setProgressState(int, QStringList)` for direct state injection and
  `applyRuntimeSnapshot(...)` for startup/runtime refreshes.
- Exposes note-directory lookup for detail-panel current-note flows. Body persistence and editor stat-refresh helpers
  are not part of the progress controller surface.
- Exposes `requestControllerHook()` as a file-backed refresh hook that reparses
  `Progress.wsprogress` and reindexes note metadata.
- Declares every inherited capability method with explicit `override`, keeping the header aligned
  with `IHierarchyRenameCapability`, `IHierarchyCrudCapability`, and
  `IHierarchyExpansionCapability` without compiler override warnings.

## Stored State

- `m_progressStates` stores the persisted `Progress.wsprogress` payload for runtime synchronization.
- `m_items` is the fixed ten-row LVRS-facing taxonomy used by the sidebar.
- `m_allNotes` stores the indexed hub notes used to build the filtered note list.
- `m_progressFilePath` keeps the source file path required to resolve the owning `.wshub` during
  runtime refreshes.
