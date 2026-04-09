# `src/app/file/sync/ContentsEditorIdleSyncController.cpp`

## Status
- Documentation phase: focused pass for editor fetch-sync runtime behavior.
- Detail level: fetch-based runtime buffering and queue handoff captured.

## Runtime Notes

- The implementation now keeps all editor-owned persistence state on the main-thread controller:
  - `m_bufferedTextByNote`: latest editor snapshot per note
  - `m_lastPersistedTextByNote`: latest successfully persisted payload per note
  - `m_dirtyNoteOrder`: round-robin fetch order for dirty notes
  - `m_inFlightNoteId` / `m_inFlightText`: the async request currently owned by the downstream coordinator
- A recurring `1000ms` `QTimer` acts as the fetch clock.
- Each fetch turn asks for the newest buffered snapshot of one dirty note and forwards it into
  `ContentsNoteManagementCoordinator::persistEditorTextForNote(...)`.
- Explicit flush requests no longer mean "persist synchronously now"; they only request one immediate fetch-cycle
  enqueue attempt on top of the same buffered state.
- Entry-time session/filesystem reconcile requests are forwarded as a dedicated pass-through operation to
  `ContentsNoteManagementCoordinator`; this controller does not reimplement RAW comparison logic locally.
- Failed writes keep the note dirty and wait for the next fetch turn instead of trying to force recovery inline on the
  editor path.

## Buffer Semantics

- Buffering is note-scoped rather than revision-scoped.
- Multiple edits to the same note before the next fetch turn collapse into one latest buffered payload.
- If the filesystem misses one intermediate fetch turn, the following fetch turn still sees the latest buffered text and
  persists that instead.

## Regression Checks

- The controller must accept buffered editor text even while the downstream persistence contract is temporarily absent.
- `editorTextPersistenceQueued(...)` must still mean "accepted into the downstream persistence queue", not merely
  "buffered in memory".
- A later fetch turn must still write the newest buffered text after an earlier async completion or failure.
- The controller must not depend on a worker-thread idle monitor or revision bookkeeping to preserve the latest editor
  buffer.
- Reconcile operations must remain side-effect free for the buffered persistence queue state (no forced dirty-order
  mutation).
