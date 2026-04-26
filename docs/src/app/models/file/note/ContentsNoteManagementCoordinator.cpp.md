# `src/app/models/file/note/ContentsNoteManagementCoordinator.cpp`

## Status
- Documentation phase: focused pass for the editor note-management split.
- Detail level: runtime behavior and queue semantics captured.

## Runtime Notes

- The coordinator now centralizes the editor-adjacent note-management queue that sits downstream of
  `src/app/models/editor/persistence/ContentsEditorPersistenceController`.
- Direct body persistence still uses `WhatSonLocalNoteFileStore`, but that worker request is now just one request kind
  inside the coordinator.
- The coordinator now also exposes one direct-context capture path for the upstream editor persistence controller:
  - resolve `noteDirectoryPathForNoteId(noteId)` while the correct content view-model is still bound
  - allow a later buffered drain turn to enqueue `DirectPersistBody` with that frozen note-directory path
  - avoid re-routing stale buffered editor text through whichever hierarchy view-model happens to be active later
- The coordinator now also exposes a read-side `noteDirectoryPathForNote(noteId)` helper.
  `ContentsEditorPersistenceController` and the selection bridge use that helper to surface the currently selected note's
  resolved package directory back to body resource rendering without depending on the editor's current hierarchy shell.
- Read/reconcile/bind requests are no longer forced through `noteId`-only lookup.
  `loadNoteBodyTextForNote(...)`, `reconcileViewSessionAndRefreshSnapshotForNote(...)`, and `bindSelectedNote(...)`
  can all carry an explicit `noteDirectoryPath`, allowing the caller to keep targeting the exact `.wsnote` package
  that selection already resolved even when another runtime model could resolve the same id differently.
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
- The coordinator now also provides one-shot session/filesystem reconciliation for editor entry:
  - resolves note/session metadata on the UI thread only
  - queues RAW-read comparison work onto the same coordinator-owned worker lane
  - accepts an explicit `preferViewSessionOnMismatch` policy bit from the upstream editor/session layer
  - emits `viewSessionSnapshotReconciled(noteId, refreshed, success, errorMessage)` back to QML-facing adapters
  - when mismatch is reported for a non-authoritative session, triggers `refreshNoteSnapshotForNote(...)` on the main
    thread
  - when mismatch is reported for an editor-authoritative session, first persists that view-session text back into RAW,
    then refreshes the visible note snapshot from the repaired filesystem state.
- The same queue now also owns selected-note lazy body reads:
  - `loadNoteBodyTextForNote(noteId, noteDirectoryPath = {})` resolves the note path on the main thread, preferring
    the caller-provided path when present
  - the actual `.wsnote` read runs on the worker lane
  - each load keeps its own request sequence
  - `noteBodyTextLoaded(sequence, noteId, text, success, errorMessage)` returns canonical persisted note source text
    back to the selection bridge, falling back to plain body text only when a note package has no separate RAW source
  - a newer same-note lazy read can therefore supersede an older in-flight read instead of being silently dropped

## Queue Semantics

- Persistence and maintenance requests are serialized under one coordinator-owned active request plus pending queue.
- Pending body-save requests are coalesced per note id so only the latest queued body survives while one earlier save is
  still running.
- Pending tracked-stat refresh requests are coalesced per note id.
- Header open-count requests deduplicate while the same note already has one such request in flight or pending.
- Pending lazy note-body load requests keep only the newest queued request per note id while still preserving request
  sequence ordering across in-flight completions.
- Reconcile requests are also coalesced by note id plus normalized session text; if a later queued request upgrades the
  same session text to `preferViewSessionOnMismatch`, that stronger editor-authoritative policy must be preserved.
- Explicit note-directory paths must survive request enqueue/coalescing so a queued same-id operation does not drift to
  a different `.wsnote` package before the worker lane executes it.

## Regression Checks

- A body persistence completion must still emit `editorTextPersistenceFinished(...)` so `ContentsEditorSessionController` can
  clear or retry its save state correctly.
- Persist completion must not immediately run backlink/open-count scans on the editor path; those must be queued as
  coordinator follow-up tasks.
- A failed tracked-stat refresh or open-count update must not break the editor save completion signal for the body write
  that already finished.
- Destroying the bound content view-model during an in-flight request must not crash queued completion handling.
- A buffered editor save that already captured its direct note-directory path must not be lost merely because the active
  content view-model changed before the next drain turn.
- Fallback/direct body persistence for a Tags-selected note must still cause the active tags hierarchy view-model to
  re-read `Tags.wstags`, so newly promoted `#label` tags appear in the hierarchy without a manual app restart.
- Session/filesystem reconciliation must return success without reload when RAW already matches the current view
  session snapshot, and that check must not perform note reads on the UI thread anymore.
- Editor-authoritative reconciliation must repair RAW from the current editor snapshot instead of refreshing stale RAW
  back into the visible session.
- Lazy selected-note body reads must execute on the worker lane and must not require the note-list model to carry the
  same full body text as selection state.
- Lazy selected-note body reads must preserve structured RAW tags such as `<paragraph>`, `<resource ... />`, and
  `<break/>`; flattening that worker-lane payload into plain text would desynchronize the parser-backed mount path
  from the persisted `.wsnbody`.
- A newer same-note body-read request must not be discarded merely because an older body-read for that note is already
  in flight.
- When the caller supplied an explicit note-directory path, the worker request must read or reconcile that package
  rather than re-resolving from `noteId` alone.
