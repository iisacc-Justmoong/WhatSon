# `src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/editor/bridge/ContentsEditorSelectionBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsEditorSelectionBridge.cpp`
- Approximate line count: 317

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

## Current Implementation Notes
- The bridge no longer owns the serialized persistence queue or stat-refresh helpers directly.
- Instead it now:
  - keeps note-list property wiring (`currentIndex`, `currentNoteId`, `currentBodyText`, `itemCount`)
  - forwards persistence/snapshot requests into `src/app/models/editor/persistence/ContentsEditorPersistenceController`
  - forwards both `editorTextPersistenceQueued(...)` and `editorTextPersistenceFinished(...)` from that editor persistence boundary
    back to QML
- Note-list `currentIndexChanged()` now also queues the same deferred selection refresh used by
  `currentNoteIdChanged()`.
  The editor bridge therefore stays synchronized with list-item activation even when the visible selected note is
  derived from the committed row index.
- Note-list `currentBodyTextChanged()` now clears the bridge's cached selected-note snapshot before it schedules the
  next refresh turn.
  A selected note can therefore adopt a late list-provided RAW snapshot without waiting for an extra note-id change or
  a filesystem round trip.
- Selected note bodies still prefer the editor persistence package load path, but the bridge now first reuses the currently
  selected note-list model `currentBodyText` payload when that contract is already populated.
  This removes the empty-editor gap where a visible selected note could briefly lose its body while the asynchronous
  package load path was still rebinding.
- The bridge therefore owns one extra QML-facing state flag, `selectedNoteBodyLoading`, so editor hosts can defer
  note-open sync until the selected note body actually arrives.
- The bridge now also exposes `selectedNoteBodyNoteId`, making the owner of the current body payload explicit for QML.
- The bridge now also exposes `selectedNoteDirectoryPath`, resolved through the editor persistence controller boundary.
  This gives QML renderers one stable note-package path for inline `<resource ...>` resolution even while hierarchy
  view-model bindings are switching.
- Selection identity is now effectively `selectedNoteId + selectedNoteDirectoryPath`.
  The bridge therefore listens to note-list `currentNoteDirectoryPathChanged()` in addition to note-id/body signals,
  and a same-id selection that resolves to a different `.wsnote` directory is treated as a real rebind/remount rather
  than as a stable same-note refresh.
- Lazy body reads now also track one request sequence per selected-note body read.
  The bridge only accepts `noteBodyTextLoaded(sequence, ...)` for the latest requested sequence of the currently
  selected note, so an older in-flight read can no longer overwrite a newer same-note body after local edits,
  persistence, or reconcile-triggered reloads.
- Failed lazy body reads now clear only the loading state.
  They no longer replace the current selected-note body with an empty string, so a transient read failure cannot wipe
  the live editor through a same-note model sync.
- Before a selected-note lazy load adopts file-backed text, the bridge now asks the editor persistence controller whether that
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
- The bridge now emits the public `editorTextPersistenceFinished(...)` signal only after that selected-body snapshot
  has been updated for successful same-note persistence.
  This keeps the editor session from clearing `pendingBodySave` before QML's presentation resolver can see the saved
  RAW body, preventing a brief fallback to the previous `.wsnbody` snapshot during live typing.
- When note-package path resolution cannot produce a lazy-load request at all, the bridge now also asks the active
  content view-model for an optional `noteBodySourceTextForNoteId(...)` runtime snapshot before it falls back to an
  empty body.
  This keeps desktop note-open flows from collapsing to a blank editor when the library runtime snapshot already owns
  the selected note source but one selection turn cannot re-resolve the package path.
- When the bridge cannot produce note-owned body text for the current selection, it now falls back to an explicit empty
  body that is still tagged with that selected note id.
- `startSelectedNoteBodyLoad(...)` now keeps body ownership explicit during loading and fallback transitions instead of
  leaving QML to infer whether the current body text still belongs to the previous note.
- `startSelectedNoteBodyLoad(...)` now also tries the direct `noteBodySourceTextForNoteId(...)` snapshot before it
  advertises a loading-only empty body for the newly selected note.
  If the runtime source is already available, the bridge surfaces that body immediately, marks it as the selected-note
  snapshot, and lets asynchronous refresh continue in the background without blanking the editor host first.
- `refreshNoteSelectionState()` now delegates note-id transitions into the editor persistence controller through
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
- Persistence requests forwarded through the bridge now follow buffered persistence semantics rather than worker-thread idle
  semantics, so the bridge remains a pure adapter while the controller owns eventual filesystem sync.
- The bridge now also exposes `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`:
  - delegates session-vs-filesystem comparison to the editor persistence layer
  - now treats reconciliation as an asynchronous request/notification boundary
  - now requires an explicit non-empty `noteId` instead of falling back to the bridge's current selected note
  - forwards `viewSessionSnapshotReconciled(...)` back to QML so note-open hosts can finish their one-shot reconcile
    bookkeeping without blocking the UI thread
- Same-note snapshot refresh now also reuses the same lazy-load path, allowing the bridge to keep the currently visible
  editor body while a background reload for that selected note is still in flight.
- Note-list `currentIndexChanged()` and `currentNoteIdChanged()` still queue one deferred bridge refresh per
  event-loop turn instead of running `refreshNoteSelectionState()` immediately, so one logical selection transition
  stays coalesced before QML reacts.
- The bridge now preserves the note-list model's raw `currentIndex` sentinel.
  `readIntProperty(...)` no longer clamps negative integers to `0`, so launch-time `currentIndex=-1` remains a true
  "no committed selection yet" state instead of being misread as row `0`.
  This keeps `resolveCurrentNoteIdFromSelectionContract()` from collapsing a real "no committed selection yet" turn
  into an implicit row `0` selection.
- `resolveCurrentNoteIdFromSelectionContract()` now only trusts committed note identity contracts:
  `currentNoteEntry.noteId`/`currentNoteEntry.id` and `currentNoteId`.
  Row snapshots and `readNoteIdAt(...)` are no longer allowed to synthesize `selectedNoteId`.
- `refreshNoteSelectionState()` now treats a readable-but-empty committed selection contract as a real clear event.
  The bridge no longer retains the previous note across transient empty-id turns, so stale note identity/body state
  cannot stay mounted merely because the list still has visible rows.
- `currentNoteEntryChanged()` is now wired as a dedicated refresh signal.
  The bridge tracks the last committed entry map and treats same-note entry revisions as a real rebind/body-refresh
  trigger even when the committed `noteId` itself did not change.
- All bridge calls that target note IO now prefer the path-aware editor persistence overloads when
  `selectedNoteDirectoryPath` is already known.
  Lazy body load, staged persistence, immediate flush, and reconcile therefore keep operating on the same `.wsnote`
  package that the selection layer actually mounted instead of rediscovering a package later from `noteId` alone.
- The bridge now also emits one explicit `selectionFlow.*` trace vocabulary for note-open debugging:
  - `selectionFlow.noteListSelectionChanged` / `selectionFlow.noteListEntrySelectionChanged`
    / `selectionFlow.noteListBodyTextChanged`
  - `selectionFlow.refreshScheduled` / `selectionFlow.refreshFlush` / `selectionFlow.refreshState`
  - `selectionFlow.noteChanged` / `selectionFlow.noteStable` / `selectionFlow.noteCleared`
  - `selectionFlow.bodyLoadStart`, immediate/pending/fallback body-load decisions, and
    `selectionFlow.bodyLoadFinished`
- Those traces summarize the live note-list state (`currentIndex`, `currentNoteId`, `currentBodyText`, `itemCount`)
  beside the bridge's own selected-note/body state, so a clicked note id can be followed through selection
  coalescing, body ownership transfer, and asynchronous snapshot completion.
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
- The bridge must continue forwarding editor-persistence queued/completion signals so
  `ContentsEditorSessionController` behavior does not regress.
- The bridge must not reintroduce note-switch blocking logic on top of the controller's buffered persistence model.
- Reconciliation requests must not reintroduce synchronous UI-thread RAW reads, and post-reconcile
  `selectedNoteBodyText` must stay aligned with the async note-body load result consumed by QML.
- Reconciliation requests must not infer their target note from current bridge state when the caller omitted `noteId`.
- If the selected note-list model already exposes `currentBodyText` for the committed note, the bridge should reuse that
  payload immediately instead of forcing QML to wait for an asynchronous package read.
- A list-item activation path must still refresh editor selection when the model only emits `currentIndexChanged()`
  before QML reaches the editor surface.
- A note-backed list with visible rows but `currentIndex=-1` must stay in the bridge's no-selection state until the
  authoritative selection writer commits one non-negative current index.
- A note-backed list row/current index alone must not synthesize `selectedNoteId`; the bridge must wait for
  `currentNoteEntry` or `currentNoteId` to confirm note identity.
- A same-note `currentBodyTextChanged()` must still invalidate the cached selected-note snapshot so the editor can
  adopt the refreshed list-provided RAW body.
- A note-open turn must not push an empty interim body into the editor before the lazy body load completes.
- A note-open turn must also surface a direct content-view-model RAW snapshot immediately when that snapshot is already
  available for the selected note.
- An older same-note lazy body-read completion must not overwrite a newer selected-note body request.
- A failed same-note lazy body-read completion must not replace the current selected-note body with `""`.
- A same-turn note-list-model swap and content-view-model swap must collapse into one selected-note refresh/rebind turn.
- The bridge must provide an explicit empty-body fallback for the selected note when no note-owned body payload can be
  resolved.
- A missing selected-note package path must still allow the bridge to surface runtime snapshot text through
  `noteBodySourceTextForNoteId(...)` when the active content view-model exposes that contract.
- A readable-but-empty committed selection contract must clear `selectedNoteId` instead of retaining the previous note
  as stale mounted state.
- A same-note `currentNoteEntryChanged()` must still force one rebind/body-refresh turn even when `selectedNoteId`
  remains stable.
- Reopening a recently edited note must prefer the buffered editor snapshot over a stale package read until queued
  persistence catches up.
- A same-note successful save must advance `selectedNoteBodyText` even when filesystem reconcile reports that no extra
  snapshot refresh is needed.
- A same `noteId` paired with a different resolved `selectedNoteDirectoryPath` must force a selected-note rebind and
  body reload instead of being treated as the previously mounted package.
- Hierarchy-switch QML reassignment must not downgrade active content or note-list bindings to JS-owned objects that
  can be garbage-collected out from under the bridge.
