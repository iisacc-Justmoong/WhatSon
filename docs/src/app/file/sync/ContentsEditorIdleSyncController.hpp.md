# `src/app/file/sync/ContentsEditorIdleSyncController.hpp`

## Status
- Documentation phase: focused pass for editor fetch-sync ownership.
- Detail level: interface contract and regression notes captured.

## Responsibility

`ContentsEditorIdleSyncController` is the editor-facing sync boundary that lives under `file/sync`.
It accepts hot-path body snapshots from QML, keeps the latest body text per note in memory, and uses a recurring
`1000ms` fetch tick to forward the newest dirty snapshot into the downstream note-management queue.
The live editor buffer remains authoritative; filesystem persistence is eventually consistent.
Each buffered snapshot may also retain the resolved note-directory path that was valid when the edit was staged, so a
later fetch turn does not have to rediscover that path through a different active hierarchy view-model.

## Public Contract

- `setContentViewModel(QObject*)`: forwards the editable content view-model dependency into the downstream
  note-management coordinator.
- `contentPersistenceContractAvailable()` / `directPersistenceAvailable()`: expose whether the downstream persistence
  lanes are currently available.
- `stageEditorTextForIdleSync(noteId, text)`: stores the latest editor snapshot for that note and marks it dirty for
  the next fetch turn even if the downstream persistence contract is temporarily unavailable.
- When the direct persistence lane is available during that staging call, the controller also snapshots the resolved
  note-directory path for that note.
- `flushEditorTextForNote(noteId, text)`: compatibility path that still stores the same buffered snapshot, but also
  asks the controller to attempt one immediate fetch-cycle enqueue when possible.
- `persistEditorTextForNote(noteId, text)`: compatibility alias for the buffered stage path.
- `pendingEditorTextForNote(noteId, &text)`: exposes the newest dirty or in-flight editor snapshot for one note
  without forcing a filesystem read, so selection/open flows can prefer the unsaved local body over stale package IO.
- `loadNoteBodyTextForNote(noteId)`: forwards one selected-note lazy body read to the downstream coordinator and
  returns its request sequence while later forwarding completion through `noteBodyTextLoaded(sequence, ...)`.
- `refreshNoteSnapshotForNote(noteId)`, `bindSelectedNote(noteId)`, `clearSelectedNote()`: forward selection/session
  work to the downstream coordinator while keeping the sync boundary in `file/sync`.
- `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`: compares one editor session snapshot
  against filesystem RAW through the downstream coordinator and only refreshes note snapshot when mismatch is detected.
- `editorTextPersistenceQueued(...)`: emitted when one buffered snapshot actually enters the downstream persistence
  queue.
- `editorTextPersistenceFinished(...)`: forwarded once that queued persistence request completes.

## Internal Scheduling Notes

- The controller no longer asks QML to prove that "this exact idle turn must persist now".
- Instead it keeps:
  - the latest buffered text per dirty note
  - the latest captured note-directory path per dirty note when direct persistence context was available
  - the last persisted text per note
  - one round-robin dirty-note order list
  - one in-flight async persistence payload
- A fetch tick may miss one intermediate editor state without data loss:
  - the in-memory editor/session buffer stays untouched
  - the next fetch tick simply writes the latest buffered snapshot
- Persistence itself remains asynchronous because actual `.wsnote` IO still runs through
  `ContentsNoteManagementCoordinator`.
- Lazy selected-note body reads also remain asynchronous, but they do not participate in dirty-note save ordering.
- A buffered snapshot that already captured a direct note-directory path may still be written on a later fetch turn even
  if the active content view-model contract has since changed.
- After each successful queued persistence completion, the controller also performs one reconcile fetch verify against
  filesystem RAW to keep editor-visible note snapshots aligned without forcing unconditional reloads.

## Regression Checks

- Repeated keystrokes must only replace the buffered snapshot for that note; QML must not depend on direct file writes
  per edit.
- The newest dirty note snapshot must still reach the async persistence queue on a later fetch turn even if one earlier
  fetch cycle missed it.
- Switching notes must not depend on a synchronous or immediate-save success path before the old note buffer stays safe.
- Failed persistence completion must keep that note dirty so a later fetch turn can retry the latest buffered text.
- Delayed persistence must stay attached to the note directory that was resolved when the edit was staged; a later
  hierarchy/content-view-model swap must not silently redirect that buffered text through a different runtime model.
- A buffered snapshot that already carries a direct note-directory path must still be eligible for a fetch-turn write
  even when the current content view-model contract is temporarily absent.
- Session/filesystem reconciliation must not trigger unconditional reloads; filesystem refresh should happen only on
  mismatch.
- Successful persistence completion must include one reconcile verify pass so the latest visible snapshot can converge
  with canonicalized filesystem RAW.
- Lazy selected-note body reads must not require list-model `currentBodyText` as an alternate transport for the full
  note body.
- The body-read completion contract must preserve per-request sequence identity so stale completions can be ignored.
- Reopening a note with a dirty or in-flight editor snapshot must be able to recover that buffered text before a stale
  file read is allowed to reclaim the editor body.
