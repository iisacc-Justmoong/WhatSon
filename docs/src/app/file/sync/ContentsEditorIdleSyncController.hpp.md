# `src/app/file/sync/ContentsEditorIdleSyncController.hpp`

## Status
- Documentation phase: focused pass for editor idle-sync ownership.
- Detail level: interface contract and regression notes captured.

## Responsibility

`ContentsEditorIdleSyncController` is the editor-facing sync boundary that lives under `file/sync`.
It accepts hot-path body snapshots from QML, owns the asynchronous `1000ms` idle gate, and forwards actual persistence
into the downstream note-management queue only after the editor becomes idle or explicitly flushes on note exit.

## Public Contract

- `setContentViewModel(QObject*)`: forwards the editable content view-model dependency into the downstream
  note-management coordinator.
- `contentPersistenceContractAvailable()` / `directPersistenceAvailable()`: expose whether the downstream persistence
  lanes are currently available.
- `stageEditorTextForIdleSync(noteId, text)`: stores the latest editor snapshot and arms the worker-thread idle timer.
- `flushEditorTextForNote(noteId, text)`: promotes the latest snapshot into an immediate async sync request without
  waiting for the idle timer.
- `persistEditorTextForNote(noteId, text)`: compatibility alias for the idle-stage path.
- `refreshNoteSnapshotForNote(noteId)`, `bindSelectedNote(noteId)`, `clearSelectedNote()`: forward selection/session
  work to the downstream coordinator while keeping the sync boundary in `file/sync`.
- `editorTextPersistenceQueued(...)`: emitted when the idle/flush gate has actually accepted one staged snapshot into
  the downstream persistence queue.
- `editorTextPersistenceFinished(...)`: forwarded once that queued persistence request completes.

## Internal Scheduling Notes

- Idle detection is intentionally asynchronous:
  - a dedicated worker object lives on its own `QThread`
  - the worker owns the `1000ms` single-shot timer
  - QML/editor code only stages snapshots and never decides idle locally
- The private worker type is forward-declared in the header and defined with the same concrete type name in the
  implementation file. Do not shadow that name with a second anonymous-namespace class, because Qt moc then sees two
  incompatible `ContentsEditorIdleSyncWorker` types and the file stops compiling.
- The controller tracks staged, idle-approved, force-flush, queued, and completed revisions separately so:
  - repeated typing only keeps the newest staged snapshot
  - note-exit flushes bypass the idle wait without turning file IO synchronous
  - a newer idle-approved snapshot can queue immediately after an older write finishes
- Persistence itself remains asynchronous because actual `.wsnote` IO still runs through
  `ContentsNoteManagementCoordinator`.

## Regression Checks

- Repeated keystrokes inside one second must not enqueue repeated `.wsnote` writes directly from QML.
- Once typing stops for at least `1000ms`, the newest staged note body must be accepted into the async persistence
  queue.
- Leaving the current note while a body is still staged must request an async flush immediately instead of waiting for
  another idle turn.
- A failed persistence completion must re-arm the idle gate for the newest unsynced snapshot instead of dropping it.
- The private idle worker type must remain a single moc-visible class definition; duplicate names across the header and
  an anonymous namespace implementation are a compile-time regression.
