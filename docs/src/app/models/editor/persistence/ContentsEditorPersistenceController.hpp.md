# `src/app/models/editor/persistence/ContentsEditorPersistenceController.hpp`

## Status
- Documentation phase: focused pass for editor persistence ownership.
- Detail level: interface contract and regression notes captured.

## Responsibility

`ContentsEditorPersistenceController` is the editor persistence boundary that lives under `models/editor/persistence`.
It accepts hot-path body snapshots from QML, keeps the latest body text per note in memory, and uses a recurring
`1000ms` drain tick to forward the newest dirty snapshot into the downstream note-management queue.
The live editor buffer remains authoritative; filesystem persistence is eventually consistent.
Each buffered snapshot may also retain the resolved note-directory path that was valid when the edit was staged, so a
later drain turn does not have to rediscover that path through a different active hierarchy view-model.
The public persistence contract therefore treats `noteId + noteDirectoryPath` as the stable mounted package identity
whenever the caller can provide both.

## Public Contract

- `setContentViewModel(QObject*)`: forwards the editable content view-model dependency into the downstream
  note-management coordinator.
- `contentPersistenceContractAvailable()` / `directPersistenceAvailable()`: expose whether the downstream persistence
  lanes are currently available.
- `stageEditorTextForPersistence(noteId, text)`: stores the latest editor snapshot for that note and marks it dirty for
  the next drain turn even if the downstream persistence contract is temporarily unavailable.
- `stageEditorTextForPersistenceAtPath(noteId, noteDirectoryPath, text)`: same buffer contract, but with an explicit
  mounted `.wsnote` directory that should travel with the staged snapshot.
- `stageEditorTextForIdleSync(...)` and `stageEditorTextForIdleSyncAtPath(...)`: compatibility aliases for older
  bridge/session callers; new code should use the persistence-named staging methods.
- When the direct persistence lane is available during that staging call, the controller also snapshots the resolved
  note-directory path for that note.
- `flushEditorTextForNote(noteId, text)`: compatibility path that still stores the same buffered snapshot, but also
  asks the controller to attempt one immediate drain-cycle enqueue when possible. The return value now reflects whether
  that immediate enqueue was actually accepted.
- `flushEditorTextForNoteAtPath(noteId, noteDirectoryPath, text)`: immediate-enqueue variant that preserves the
  caller-provided mounted package path.
- `persistEditorTextForNote(noteId, text)`: compatibility alias for the buffered stage path.
- `noteDirectoryPathForNote(noteId)`: exposes the freshest known note-directory path for one note, preferring a
  buffered snapshot's captured path and otherwise falling back to the downstream coordinator's live resolution.
- `pendingEditorTextForNote(noteId, &text)`: exposes the newest dirty or in-flight editor snapshot for one note
  without forcing a filesystem read, so selection/open flows can prefer the unsaved local body over stale package IO.
- `loadNoteBodyTextForNote(noteId)`: forwards one selected-note lazy body read to the downstream coordinator and
  returns its request sequence while later forwarding completion through `noteBodyTextLoaded(sequence, ...)`.
- `loadNoteBodyTextForNoteAtPath(noteId, noteDirectoryPath)`: same read contract, but bound to an explicit selected
  `.wsnote` package.
- `refreshNoteSnapshotForNote(noteId)`, `bindSelectedNote(noteId)`, `clearSelectedNote()`: forward selection/session
  work to the downstream coordinator while keeping the persistence boundary in `models/editor/persistence`.
- `bindSelectedNoteAtPath(noteId, noteDirectoryPath)`: selected-note bind contract that avoids a second package
  resolution step when the selection layer already knows the mounted directory.
- `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`: compares one editor session snapshot
  against filesystem RAW through the downstream coordinator. Callers may also mark the current editor session as
  authoritative so a mismatch persists the view-session text back into RAW before refreshing the visible note snapshot.
- `reconcileViewSessionAndRefreshSnapshotForNoteAtPath(noteId, noteDirectoryPath, viewSessionText, ...)`: package-
  stable reconcile path for same-id duplicate note packages.
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
- A persistence drain tick may miss one intermediate editor state without data loss:
  - the in-memory editor/session buffer stays untouched
  - the next drain tick simply writes the latest buffered snapshot
- Persistence itself remains asynchronous because actual `.wsnote` IO still runs through
  `ContentsNoteManagementCoordinator`.
- Lazy selected-note body reads also remain asynchronous, but they do not participate in dirty-note save ordering.
- A buffered snapshot that already captured a direct note-directory path may still be written on a later drain turn even
  if the active content view-model contract has since changed.
- After each successful queued persistence completion, the controller also performs one editor-authoritative reconcile
  verify against filesystem RAW so the just-saved editor snapshot cannot be replaced by a stale same-note RAW reload.

## Regression Checks

- Repeated keystrokes must only replace the buffered snapshot for that note; QML must not depend on direct file writes
  per edit.
- The newest dirty note snapshot must still reach the async persistence queue on a later drain turn even if one earlier
  drain cycle missed it.
- Switching notes must not depend on a synchronous or immediate-save success path before the old note buffer stays safe.
- Immediate flush callers must not treat a merely buffered snapshot as if it had already entered the downstream
  persistence queue.
- Failed persistence completion must keep that note dirty so a later drain turn can retry the latest buffered text.
- Delayed persistence must stay attached to the note directory that was resolved when the edit was staged; a later
  hierarchy/content-view-model swap must not silently redirect that buffered text through a different runtime model.
- A buffered snapshot that already carries a direct note-directory path must still be eligible for a drain-turn write
  even when the current content view-model contract is temporarily absent.
- Path-aware selected-note reads/reconcile/bind calls must preserve the caller-provided `.wsnote` package identity
  instead of re-resolving from `noteId` alone.
- Session/filesystem reconciliation must not trigger unconditional reloads; filesystem refresh should happen only on
  mismatch.
- Successful persistence completion must include one reconcile verify pass, and that verify must prefer the just-saved
  editor session over stale RAW when the two still differ.
- Lazy selected-note body reads must not require list-model `currentBodyText` as an alternate transport for the full
  note body.
- The body-read completion contract must preserve per-request sequence identity so stale completions can be ignored.
- Reopening a note with a dirty or in-flight editor snapshot must be able to recover that buffered text before a stale
  file read is allowed to reclaim the editor body.
