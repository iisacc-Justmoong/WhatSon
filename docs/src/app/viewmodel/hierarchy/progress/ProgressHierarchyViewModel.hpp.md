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

## Stored State

- `m_progressStates` is the canonical ordered enum list mirrored from `Progress.wsprogress`.
- `m_items` is the flat LVRS-facing row set derived directly from `m_progressStates`.
- `m_allNotes` stores the indexed hub notes used to build the filtered note list.
- `m_progressFilePath` keeps the source file path required to resolve the owning `.wshub` during
  runtime refreshes.
