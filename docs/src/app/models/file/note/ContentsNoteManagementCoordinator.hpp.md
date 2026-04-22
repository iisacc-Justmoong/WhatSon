# `src/app/models/file/note/ContentsNoteManagementCoordinator.hpp`

## Status
- Documentation phase: focused pass for the editor note-management split.
- Detail level: interface contract and regression notes captured.

## Responsibility

`ContentsNoteManagementCoordinator` is the downstream non-editing management boundary for the editor stack.
It owns note-package persistence enqueue, open-count maintenance, tracked-stat refresh scheduling, and metadata reload
follow-ups after the upstream `file/sync/ContentsEditorIdleSyncController` has already decided that an editor snapshot
should synchronize to disk.

## Public Contract

- `setContentViewModel(QObject*)`: binds the editable hierarchy/content view-model contract used to resolve note paths
  and mirror persisted state back into runtime models.
- `contentPersistenceContractAvailable()`: reports whether either the direct `.wsnote` lane or the deferred fallback
  view-model persistence lane is available.
- `directPersistenceAvailable()`: reports whether the fast direct `.wsnote` lane is available.
- `persistEditorTextForNote(noteId, text)`: accepts an already-approved sync snapshot and enqueues it onto the
  coordinator-owned management queue instead of performing save/stat work inline on the editor path.
- `persistEditorTextForNoteAtPath(noteId, noteDirectoryPath, text)`: direct worker-lane save path used when the caller
  already captured the destination note directory.
- `captureDirectPersistenceContextForNote(noteId, &noteDirectoryPath)`: allows the upstream sync boundary to snapshot
  the current direct persistence target before a later hierarchy transition can change the active view-model.
- `noteDirectoryPathForNote(noteId)`: exposes the current best-effort note-directory resolution without requiring the
  direct-persistence lane to be active, so read-side consumers can keep resource/package resolution tied to the same
  note package as the editor session.
- `loadNoteBodyTextForNote(noteId, noteDirectoryPath = "")`: enqueues one worker-thread note read for the selected
  note body and returns the resulting request sequence for `noteBodyTextLoaded(...)`.
  When `noteDirectoryPath` is present, that explicit `.wsnote` package is read directly instead of being rediscovered
  from `noteId`.
  That completion payload is the canonical persisted note source text when available, not a plain-text projection.
- `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText, ..., noteDirectoryPath = "")`: reads the
  current note RAW body source from filesystem, compares it against the provided view-session text, and only requests
  snapshot refresh when they differ.
- `bindSelectedNote(noteId, noteDirectoryPath = "")`: selected-note session bind contract that can retain the exact
  mounted package directory across selection/mount transitions.
- `refreshNoteSnapshotForNote(noteId)`: reloads one note's metadata/body snapshot through the bound content view-model.
- `clearSelectedNote()`: clears the selected-note session and header-open-count follow-up context.
- `editorTextPersistenceFinished(...)`: completion signal back to QML/editor session code once the queued persistence
  request reaches a final result.

## Internal Queue Notes

- The coordinator owns the serialized request queue for:
  - lazy selected-note body reads
  - direct `.wsnote` body persistence
  - fallback view-model persistence requests
  - open-count header updates
  - tracked-stat refresh work
- Editor/QML code only asks for enqueue acceptance; the coordinator performs file IO and follow-up management later.
- Repeated autosaves for the same note coalesce to the newest pending persistence payload.
- Repeated pending tracked-stat refresh requests for the same note coalesce into one pending request.
- Repeated selected-note body load requests for the same note keep only the newest pending worker read for that note.
- Requests that already captured an explicit note-directory path must preserve that package target while pending, so
  same-id duplicate `.wsnote` packages do not alias each other inside the queue.

## Regression Checks

- Ordinary typing must only enqueue persistence through the coordinator; it must not directly run stat refresh or
  open-count work through the editor-facing bridge.
- A successful body persistence request must still mirror the normalized body state back into the editable hierarchy
  view-model and then schedule tracked-stat refresh as a follow-up management task.
- Selecting a note must still bump header open-count metadata, but that work must happen through the coordinator-owned
  background lane instead of synchronously inside editor selection refresh.
- When the content view-model contract is unavailable, the coordinator must reject persistence requests cleanly instead
  of letting QML assume the write succeeded.
- If the upstream sync layer already captured a valid note-directory path, the coordinator must still be able to
  enqueue a direct body write for that note even after the active content view-model changed.
- Session/filesystem reconciliation must not force a metadata reload when the session text already matches filesystem
  RAW.
- Selected note body reads must stay asynchronous and must not be satisfied by mirroring full note bodies through the
  note-list model contract.
- Selected note body loads must preserve the same structured source contract that
  `noteBodySourceTextForNoteId(...)` exposes from hierarchy view-models, so downstream mount code can continue to
  parse canonical `.wsnbody` content instead of a flattened projection.
- The selected-note body-read signal contract must preserve request sequencing so the selection bridge can ignore stale
  completions.
- A caller-supplied note-directory path must remain authoritative for load/reconcile/bind requests; the coordinator
  must not silently redirect those operations through another package that happens to share the same `noteId`.
