# `src/app/models/editor/persistence/ContentsEditorPersistenceController.cpp`

## Status
- Documentation phase: focused pass for editor persistence runtime behavior.
- Detail level: drain-timer runtime buffering and queue handoff captured.

## Runtime Notes

- The implementation now keeps all editor-owned persistence state on the main-thread controller:
  - `m_bufferedSnapshotsByNote`: latest editor snapshot per note, including any direct note-directory context captured
    at staging time
  - `m_lastPersistedTextByNote`: latest successfully persisted payload per note
  - `m_dirtyNoteOrder`: round-robin persistence order for dirty notes
  - `m_inFlightNoteId` / `m_inFlightNoteDirectoryPath` / `m_inFlightText`: the async request currently owned by the
    downstream coordinator
- A recurring `1000ms` `QTimer` acts as the persistence drain clock.
- Each drain turn asks for the newest buffered snapshot of one dirty note and forwards it into
  `ContentsNoteManagementCoordinator::persistEditorTextForNote(...)`.
- If that buffered snapshot already carries a direct note-directory path, the drain turn now prefers
  `persistEditorTextForNoteAtPath(...)` so the delayed save stays attached to the original note package even after a
  hierarchy/content-view-model transition.
- Explicit flush requests no longer mean "persist synchronously now"; they only request one immediate drain-cycle
  enqueue attempt on top of the same buffered state.
- That immediate flush path now returns the real enqueue result for the current snapshot instead of always reporting
  success after buffering.
- Entry-time session/filesystem reconcile requests are forwarded as a dedicated pass-through operation to
  `ContentsNoteManagementCoordinator`; this controller does not reimplement RAW comparison logic locally.
- Those reconcile calls now also carry an explicit "prefer view session on mismatch" bit so the downstream coordinator
  can choose between RAW-first refresh and editor-first RAW repair without QML needing to duplicate that policy.
- Selected-note body reads are also forwarded as pass-through operations to `ContentsNoteManagementCoordinator`.
  This controller emits `noteBodyTextLoaded(sequence, ...)` back toward the selection bridge without mutating the
  dirty-save buffer state.
- The controller now also exposes explicit path-aware entry points such as
  `stageEditorTextForPersistenceAtPath(...)`, `flushEditorTextForNoteAtPath(...)`,
  `loadNoteBodyTextForNoteAtPath(...)`, `reconcileViewSessionAndRefreshSnapshotForNoteAtPath(...)`, and
  `bindSelectedNoteAtPath(...)`.
  Those overloads let the selection/session stack keep operating on the exact `.wsnote` package that was selected,
  even when another runtime view-model could resolve the same `noteId` differently later.
- Existing `stageEditorTextForIdleSync...` methods remain as compatibility aliases and delegate into the
  persistence-named staging entry points.
- The controller now also provides one read-only "pending editor text" query that reports dirty or in-flight note
  bodies back to the selection layer.
  This lets note-open prefer the newest unsaved local snapshot instead of immediately trusting filesystem text.
- The controller now also provides one read-only note-directory query per note.
  That query prefers the note-directory path captured into the buffered snapshot at stage time, then falls back to the
  downstream coordinator's current resolution. The selection bridge uses it to keep body resource rendering anchored to
  the same mounted note package even if hierarchy view-model wiring is in flux.
- Successful async persistence completion now also runs one post-write filesystem reconcile step:
  - compares the persisted editor snapshot against note RAW through
    `ContentsNoteManagementCoordinator::reconcileViewSessionAndRefreshSnapshotForNote(...)`
  - marks that compare as editor-authoritative so any mismatch rewrites RAW from the persisted editor snapshot first
  - refreshes the bound note snapshot only after that RAW repair path finishes.
- Failed writes keep the note dirty and wait for the next drain turn instead of trying to force recovery inline on the
  editor path.

## Buffer Semantics

- Buffering is note-scoped rather than revision-scoped.
- Multiple edits to the same note before the next drain turn collapse into one latest buffered payload.
- The same collapse also keeps the newest resolved direct note-directory path captured for that note.
- If the filesystem misses one intermediate drain turn, the following drain turn still sees the latest buffered text and
  persists that instead.
- Buffer identity is still keyed by logical note id for coalescing, but explicit path-carrying APIs preserve which
  concrete `.wsnote` package that buffered state belongs to while it is in flight or waiting for enqueue.

## Regression Checks

- The controller must accept buffered editor text even while the downstream persistence contract is temporarily absent.
- `editorTextPersistenceQueued(...)` must still mean "accepted into the downstream persistence queue", not merely
  "buffered in memory".
- An immediate flush call must not report success when `enqueueNextBufferedPersistenceIfNeeded()` rejected the
  snapshot.
- A later drain turn must still write the newest buffered text after an earlier async completion or failure.
- The controller must not depend on a worker-thread idle monitor or revision bookkeeping to preserve the latest editor
  buffer.
- Reconcile operations must remain side-effect free for the buffered persistence queue state (no forced dirty-order
  mutation).
- A successful persistence completion must still verify note RAW/session alignment once, so editor-visible snapshots
  can self-heal if downstream body serialization canonicalizes the source text without letting stale RAW reclaim the
  editor.
- Lazy selected-note body reads must stay side-effect free for the dirty-note persistence queue and must not trigger
  extra save scheduling by themselves.
- A newer same-note lazy body-read request must still be observable upstream after an older same-note read is already
  in flight.
- A buffered note save must not become blocked solely because the active content view-model changed, as long as the
  staged snapshot already captured a direct note-directory path.
- A selected note must not reopen from filesystem text while the controller still owns a newer dirty or in-flight
  editor snapshot for that same note.
- When the caller already knows the mounted note directory, the controller must not silently downgrade that request
  back to id-only note resolution.
