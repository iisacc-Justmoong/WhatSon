# `src/app/file/note/ContentsNoteManagementCoordinator.hpp`

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
- `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`: reads the current note RAW body source
  from filesystem, compares it against the provided view-session text, and only requests snapshot refresh when they
  differ.
- `refreshNoteSnapshotForNote(noteId)`: reloads one note's metadata/body snapshot through the bound content view-model.
- `bindSelectedNote(noteId)` / `clearSelectedNote()`: maintain the selected note session and enqueue header-only
  open-count maintenance outside the editor hot path.
- `editorTextPersistenceFinished(...)`: completion signal back to QML/editor session code once the queued persistence
  request reaches a final result.

## Internal Queue Notes

- The coordinator owns the serialized request queue for:
  - direct `.wsnote` body persistence
  - fallback view-model persistence requests
  - open-count header updates
  - tracked-stat refresh work
- Editor/QML code only asks for enqueue acceptance; the coordinator performs file IO and follow-up management later.
- Repeated autosaves for the same note coalesce to the newest pending persistence payload.
- Repeated pending tracked-stat refresh requests for the same note coalesce into one pending request.

## Regression Checks

- Ordinary typing must only enqueue persistence through the coordinator; it must not directly run stat refresh or
  open-count work through the editor-facing bridge.
- A successful body persistence request must still mirror the normalized body state back into the editable hierarchy
  view-model and then schedule tracked-stat refresh as a follow-up management task.
- Selecting a note must still bump header open-count metadata, but that work must happen through the coordinator-owned
  background lane instead of synchronously inside editor selection refresh.
- When the content view-model contract is unavailable, the coordinator must reject persistence requests cleanly instead
  of letting QML assume the write succeeded.
- Session/filesystem reconciliation must not force a metadata reload when the session text already matches filesystem
  RAW.
