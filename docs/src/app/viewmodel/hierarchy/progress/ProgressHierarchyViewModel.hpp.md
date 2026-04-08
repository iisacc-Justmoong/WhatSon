# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp`

## Responsibility

This header declares the progress hierarchy viewmodel that backs the progress sidebar and its note
list.

## Public Contract

- Publishes both `itemModel` and `noteListModel`, so the progress domain exposes sidebar rows and
  filtered notes through the same hierarchy interface used by other domains.
- Exposes `setProgressState(int, QStringList)` for direct state injection and
  `applyRuntimeSnapshot(...)` for startup/runtime refreshes.
- Exposes body persistence helpers and note-directory lookup so the active progress note can remain
  editable in shared editor flows.
- The same editable surface now splits hot-path editor persistence from expensive stat refresh by exposing
  `applyPersistedBodyStateForNote(...)` and `requestTrackedStatisticsRefreshForNote(...)`.
- Exposes `requestViewModelHook()` as a file-backed refresh hook that reparses
  `Progress.wsprogress` and reindexes note metadata.
- Emits `hubFilesystemMutated()` after successful `.wsnbody` writes so hub sync can treat progress-domain edits as
  local mutations without full user-interaction hints.
- Exposes `reloadNoteMetadataForNoteId(QString)` so detail-panel writes can refresh the current
  progress-filtered note list without waiting for a full runtime reload.
- Declares every inherited capability method with explicit `override`, keeping the header aligned
  with `IHierarchyRenameCapability`, `IHierarchyCrudCapability`, and
  `IHierarchyExpansionCapability` without compiler override warnings.

## Stored State

- `m_progressStates` stores the persisted `Progress.wsprogress` payload for runtime synchronization.
- `m_items` is the fixed ten-row LVRS-facing taxonomy used by the sidebar.
- `m_allNotes` stores the indexed hub notes used to build the filtered note list.
- `m_progressFilePath` keeps the source file path required to resolve the owning `.wshub` during
  runtime refreshes.
