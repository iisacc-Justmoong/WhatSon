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
- The bridge now also exposes `selectedNoteBodyNoteId`, making the owner of the current body payload explicit for QML.
- The bridge now also exposes `selectedNoteDirectoryPath`, resolved through the idle-sync/controller boundary.
  This gives QML renderers one stable note-package path for inline `<resource ...>` resolution even while hierarchy
  view-model bindings are switching.
- Lazy body reads now also track one request sequence per selected-note fetch.
  The bridge only accepts `noteBodyTextLoaded(sequence, ...)` for the latest requested sequence of the currently
  selected note, so an older in-flight read can no longer overwrite a newer same-note body after local edits,
  persistence, or reconcile-triggered reloads.
- Failed lazy body reads now clear only the loading state.
  They no longer replace the current selected-note body with an empty string, so a transient read failure cannot wipe
  the live editor through a same-note model sync.
- Before a selected-note lazy load adopts file-backed text, the bridge now asks the idle-sync controller whether that
  same note still has a dirty or in-flight editor snapshot.
  If so, the bridge reuses that pending body immediately and skips the stale file-backed adoption path.
- That same bridge refresh path now also re-resolves `selectedNoteDirectoryPath` for the current selected note on each
  settled selection/rebind turn and after same-note successful persistence completion.
- The same pending-body check also runs when a lazy body-read completion arrives, so an older file read cannot reclaim
  the selected body after the user already restaged newer local text for that note.
- The bridge now also updates `selectedNoteBodyText` immediately on successful
  `editorTextPersistenceFinished(noteId, text, success, ...)` for the currently visible note body.
  This closes the stale same-note gap where persistence could succeed, reconcile could report "no refresh needed", and
  QML would still keep the note-open body snapshot as the last selected-body value.
- When note-package path resolution cannot produce a lazy-load request at all, the bridge now also asks the active
  content view-model for an optional `noteBodySourceTextForNoteId(...)` runtime snapshot before it falls back to an
  empty body.
  This keeps desktop note-open flows from collapsing to a blank editor when the library runtime snapshot already owns
  the selected note source but one selection turn cannot re-resolve the package path.
- When the bridge cannot produce note-owned body text for the current selection, it now falls back to an explicit empty
  body that is still tagged with that selected note id.
- `startSelectedNoteBodyLoad(...)` now keeps body ownership explicit during loading and fallback transitions instead of
  leaving QML to infer whether the current body text still belongs to the previous note.
- `refreshNoteSelectionState()` now delegates note-id transitions into the sync controller through
  `bindSelectedNote(...)` / `clearSelectedNote()`, so open-count maintenance also left the bridge.
- The bridge now also checks an optional note-list-model `noteBacked` property before it accepts a `currentNoteId`
  contract.
  Models such as `ResourcesListModel` can therefore keep note-like selection ids for generic list delegates without
  being rebound into note open-count, note body load, or persistence lifecycles.
- `setContentViewModel(...)` now only swaps the downstream content-view-model dependency immediately.
  The actual selected-note rebind is deferred into the existing queued selection refresh, so note-list-model and
  content-view-model replacements share one settled refresh turn instead of creating an intermediate mismatched pair.
- `setNoteListModel(...)` now also uses that same queued selection refresh path after model replacement, while
  `refreshNoteCountState()` still updates immediately for count-only consumers.
- Both setter paths now also force `QQmlEngine::CppOwnership` on incoming `QObject*` bindings before the bridge stores
  them. This keeps hierarchy-switch handoffs from reclassifying long-lived C++ models as JS-owned garbage-collectable
  objects when QML reassigns the active content/note-list pair.
- `refreshNoteSelectionState()` now honors one `requiresRebind` flag, so the bridge can rebind the current selected
  note into a newly replaced content view-model even when the selected note id itself did not change.
- Persistence requests forwarded through the bridge now follow buffered fetch semantics rather than worker-thread idle
  semantics, so the bridge remains a pure adapter while the controller owns eventual filesystem sync.
- The bridge now also exposes `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`:
  - delegates session-vs-filesystem comparison to the sync layer
  - now treats reconciliation as an asynchronous request/notification boundary
  - now requires an explicit non-empty `noteId` instead of falling back to the bridge's current selected note
  - forwards `viewSessionSnapshotReconciled(...)` back to QML so note-open hosts can finish their one-shot reconcile
    bookkeeping without blocking the UI thread
- Same-note snapshot refresh now also reuses the same lazy-load path, allowing the bridge to keep the currently visible
  editor body while a background reload for that selected note is still in flight.
- Note-list `currentNoteIdChanged()` still queues one deferred bridge refresh per event-loop turn instead of running
  `refreshNoteSelectionState()` immediately, so one logical selection transition stays coalesced before QML reacts.
- Note-list count observation now connects to `itemCountChanged(int)` explicitly.
  The older parameterless signal signature no longer matched the actual note-list-model contract and produced a runtime
  QObject connect warning during live app startup.

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
- A non-note-backed list model must clear selected-note lifecycle state instead of attempting to load or bind a fake
  note package from its `currentNoteId`.
- The bridge must continue forwarding sync-boundary queued/completion signals so `ContentsEditorSession.qml` behavior
  does not regress.
- The bridge must not reintroduce note-switch blocking logic on top of the controller's buffered fetch model.
- Reconciliation requests must not reintroduce synchronous UI-thread RAW reads, and post-reconcile
  `selectedNoteBodyText` must stay aligned with the async note-body load result consumed by QML.
- Reconciliation requests must not infer their target note from current bridge state when the caller omitted `noteId`.
- Large-note selection must not depend on note-list `currentBodyText` carrying the full note body.
- A note-open turn must not push an empty interim body into the editor before the lazy body load completes.
- An older same-note lazy body-read completion must not overwrite a newer selected-note body request.
- A failed same-note lazy body-read completion must not replace the current selected-note body with `""`.
- A same-turn note-list-model swap and content-view-model swap must collapse into one selected-note refresh/rebind turn.
- The bridge must provide an explicit empty-body fallback for the selected note when no note-owned body payload can be
  resolved.
- A missing selected-note package path must still allow the bridge to surface runtime snapshot text through
  `noteBodySourceTextForNoteId(...)` when the active content view-model exposes that contract.
- Reopening a recently edited note must prefer the buffered editor snapshot over a stale package read until queued
  persistence catches up.
- A same-note successful save must advance `selectedNoteBodyText` even when filesystem reconcile reports that no extra
  snapshot refresh is needed.
- Hierarchy-switch QML reassignment must not downgrade active content or note-list bindings to JS-owned objects that
  can be garbage-collected out from under the bridge.
