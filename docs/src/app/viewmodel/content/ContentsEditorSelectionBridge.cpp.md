# `src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsEditorSelectionBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsEditorSelectionBridge.cpp`
- Approximate line count: 317

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

## Current Implementation Notes
- The bridge no longer owns the serialized persistence queue or stat-refresh helpers directly.
- Instead it now:
  - keeps note-list property wiring (`currentNoteId`, `itemCount`)
  - forwards persistence/snapshot requests into `file/sync/ContentsEditorIdleSyncController`
  - forwards both `editorTextPersistenceQueued(...)` and `editorTextPersistenceFinished(...)` from that sync boundary
    back to QML
- Selected note bodies are now lazy-loaded from the note package through the idle-sync boundary instead of being read
  out of the note-list model.
- The bridge therefore owns one extra QML-facing state flag, `selectedNoteBodyLoading`, so editor hosts can defer
  note-open sync until the selected note body actually arrives.
- Lazy body reads now also track one request sequence per selected-note fetch.
  The bridge only accepts `noteBodyTextLoaded(sequence, ...)` for the latest requested sequence of the currently
  selected note, so an older in-flight read can no longer overwrite a newer same-note body after local edits,
  persistence, or reconcile-triggered reloads.
- `refreshNoteSelectionState()` now delegates note-id transitions into the sync controller through
  `bindSelectedNote(...)` / `clearSelectedNote()`, so open-count maintenance also left the bridge.
- `setContentViewModel(...)` now rebinds the same content view-model into the sync controller and re-seeds the
  currently selected note session there.
- Persistence requests forwarded through the bridge now follow buffered fetch semantics rather than worker-thread idle
  semantics, so the bridge remains a pure adapter while the controller owns eventual filesystem sync.
- The bridge now also exposes `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`:
  - delegates session-vs-filesystem comparison to the sync layer
  - now treats reconciliation as an asynchronous request/notification boundary
  - forwards `viewSessionSnapshotReconciled(...)` back to QML so note-open hosts can finish their one-shot reconcile
    bookkeeping without blocking the UI thread
- Same-note snapshot refresh now also reuses the same lazy-load path, allowing the bridge to keep the currently visible
  editor body while a background reload for that selected note is still in flight.
- Note-list `currentNoteIdChanged()` still queues one deferred bridge refresh per event-loop turn instead of running
  `refreshNoteSelectionState()` immediately, so one logical selection transition stays coalesced before QML reacts.

### Classes and Structs
- None detected during scaffold generation.

### Enums
- None detected during scaffold generation.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.

## Regression Checks

- Note-list selection changes must still update `selectedNoteId` / `selectedNoteBodyText`, but they must no longer run
  open-count/stat maintenance directly inside the bridge implementation.
- The bridge must continue forwarding sync-boundary queued/completion signals so `ContentsEditorSession.qml` behavior
  does not regress.
- The bridge must not reintroduce note-switch blocking logic on top of the controller's buffered fetch model.
- Reconciliation requests must not reintroduce synchronous UI-thread RAW reads, and post-reconcile
  `selectedNoteBodyText` must stay aligned with the async note-body load result consumed by QML.
- Large-note selection must not depend on note-list `currentBodyText` carrying the full note body.
- A note-open turn must not push an empty interim body into the editor before the lazy body load completes.
- An older same-note lazy body-read completion must not overwrite a newer selected-note body request.
