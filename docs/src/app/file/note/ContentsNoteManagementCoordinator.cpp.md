# `src/app/file/note/ContentsNoteManagementCoordinator.cpp`

## Status
- Documentation phase: focused pass for the editor note-management split.
- Detail level: runtime behavior and queue semantics captured.

## Runtime Notes

- The coordinator now centralizes the editor-adjacent note-management queue that sits downstream of
  `file/sync/ContentsEditorIdleSyncController`.
- Direct body persistence still uses `WhatSonLocalNoteFileStore`, but that worker request is now just one request kind
  inside the coordinator.
- Non-editing maintenance work also lives here:
  - `WhatSonNoteFileStatSupport::incrementOpenCountForNoteHeader(...)`
  - `WhatSonNoteFileStatSupport::refreshTrackedStatisticsForNote(...)`
  - post-persist runtime mirror/update hooks back into the bound content view-model
- Successful direct persistence first updates the editable runtime note snapshot through
  `applyPersistedBodyStateForNote(...)`, then queues tracked-stat refresh as a separate follow-up request.
- When the bound hierarchy view-model does not expose `applyPersistedBodyStateForNote(...)` but does expose
  `requestViewModelHook()`, the coordinator now queues that hook after a successful direct body write. This lets
  fallback-driven hierarchy screens such as Tags re-read freshly mutated hierarchy files (`Tags.wstags`) after inline
  hashtag promotion.
- Successful tracked-stat refresh then asks the content view-model to reload that note's metadata snapshot.
- The fallback persistence lane, when used, is also deferred behind the coordinator queue so the editor hot path still
  only observes enqueue acceptance rather than immediate management work.

## Queue Semantics

- Persistence and maintenance requests are serialized under one coordinator-owned active request plus pending queue.
- Pending body-save requests are coalesced per note id so only the latest queued body survives while one earlier save is
  still running.
- Pending tracked-stat refresh requests are coalesced per note id.
- Header open-count requests deduplicate while the same note already has one such request in flight or pending.

## Regression Checks

- A body persistence completion must still emit `editorTextPersistenceFinished(...)` so `ContentsEditorSession.qml` can
  clear or retry its save state correctly.
- Persist completion must not immediately run backlink/open-count scans on the editor path; those must be queued as
  coordinator follow-up tasks.
- A failed tracked-stat refresh or open-count update must not break the editor save completion signal for the body write
  that already finished.
- Destroying the bound content view-model during an in-flight request must not crash queued completion handling.
- Fallback/direct body persistence for a Tags-selected note must still cause the active tags hierarchy view-model to
  re-read `Tags.wstags`, so newly promoted `#label` tags appear in the hierarchy without a manual app restart.
