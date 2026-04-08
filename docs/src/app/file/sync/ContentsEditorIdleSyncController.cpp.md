# `src/app/file/sync/ContentsEditorIdleSyncController.cpp`

## Status
- Documentation phase: focused pass for editor idle-sync runtime behavior.
- Detail level: worker-thread idle gate and queue handoff captured.

## Runtime Notes

- The implementation owns a private `ContentsEditorIdleSyncWorker` on a dedicated `QThread`.
- That worker only does one thing: detect `1000ms` of editor inactivity and emit the approved staged revision back to
  the main-thread controller.
- That worker type must stay the same concrete class named by the header forward declaration, because the `.moc`
  output references `ContentsEditorIdleSyncWorker` directly. A second anonymous-namespace class with the same name is
  a build-breaking ambiguity.
- The main-thread controller then checks revision state and forwards the newest eligible snapshot into
  `ContentsNoteManagementCoordinator::persistEditorTextForNote(...)`.
- Explicit note-exit flushes clear the worker timer and ask the controller to enqueue the staged snapshot immediately,
  but the actual note write still stays asynchronous because the downstream coordinator owns the file-IO queue.
- Failed writes reschedule the newest unsynced revision back through the idle monitor instead of retrying inline on the
  editor path.

## Revision Semantics

- `m_latestStagedRevision`: newest editor snapshot accepted from QML.
- `m_latestIdleRevision`: newest revision that actually crossed the async idle threshold.
- `m_forceFlushRevision`: newest revision that should bypass the idle wait because the editor is leaving the note.
- `m_lastQueuedRevision`: newest revision already accepted into the downstream persistence queue.
- `m_lastCompletedRevision`: newest revision that finished successfully.

## Regression Checks

- The worker-thread timer must remain the only place that decides editor idle.
- The controller must emit `editorTextPersistenceQueued(...)` only when a staged snapshot was actually accepted into the
  downstream persistence queue.
- Idle-approved newer text must queue immediately after an older async write finishes, without waiting for another user
  keystroke.
- Destroying the controller must stop the worker thread cleanly.
- The worker type must compile as one unambiguous moc-visible class name.
