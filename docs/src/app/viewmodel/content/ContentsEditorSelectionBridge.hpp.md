# `src/app/models/editor/bridge/ContentsEditorSelectionBridge.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/editor/bridge/ContentsEditorSelectionBridge.hpp`
- Source kind: C++ header
- File name: `ContentsEditorSelectionBridge.hpp`
- Approximate line count: 98

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

## Current Implementation Notes
- `ContentsEditorSelectionBridge` is now selection-facing only:
  - exports `selectedNoteId`, `selectedNoteDirectoryPath`, `selectedNoteBodyNoteId`, `selectedNoteBodyText`,
    `selectedNoteBodyLoading`, and `visibleNoteCount`
  - keeps the note-list selection/count property wiring for QML
  - forwards persistence and note-management requests to `file/sync/ContentsEditorIdleSyncController`
- The note-list contract can now optionally provide `currentBodyText` for the committed selected note.
  When that payload is available, the bridge reuses it immediately before it falls back to lazy sync/package loading.
- The bridge now also treats note-list `currentIndexChanged()` as a first-class selection refresh trigger.
  This keeps list-item activation aligned with editor selection even when the model derives `currentNoteId` lazily from
  its committed index instead of emitting a dedicated note-id signal first.
- The same bridge now also listens for note-list `currentBodyTextChanged()` and invalidates its cached selected-note
  snapshot before the next queued refresh turn.
  A selected note can therefore receive a late runtime body snapshot from the list model without requiring the note id
  itself to change again.
- Public invokables remain:
  - `persistEditorTextForNote(noteId, text)`
  - `stageEditorTextForIdleSync(noteId, text)`
  - `flushEditorTextForNote(noteId, text)`
  - `reconcileViewSessionAndRefreshSnapshotForNote(noteId, viewSessionText)`
  - `refreshSelectedNoteSnapshot()`
  and they now delegate to the sync boundary instead of performing note-management work inside the bridge itself.
- Those invokables no longer imply an idle-threshold or immediate-save guarantee:
  - staging means "buffer this latest note snapshot for a later fetch turn"
  - flush means "buffer it and request one immediate fetch attempt if possible"
- `reconcileViewSessionAndRefreshSnapshotForNote(...)` now also requires an explicit note id; it no longer falls back
  to whichever note the bridge currently marks as selected.
- The bridge still exposes `directPersistenceContractAvailable` and `contentPersistenceContractAvailable`, but those
  values now come from the sync-owned downstream management boundary.
- The bridge now also forwards `editorTextPersistenceQueued(...)` so QML sessions can distinguish
  "the fetch boundary accepted one buffered snapshot into the persistence queue" from "the write already finished".
- `editorTextPersistenceFinished(noteId, text, success, errorMessage)` is still the completion signal consumed by QML
  editor sessions, and the bridge now also uses successful same-note completion to advance its own
  `selectedNoteBodyText` cache.
- Internal note-list selection refresh now follows a queued-coalesced contract: one event-loop turn can schedule at
  most one pending note-selection refresh.
- Note-list-model replacement and content-view-model replacement now both funnel through that same queued refresh turn.
  The bridge keeps one `requiresRebind` flag so the downstream sync controller rebinds the selected note only after the
  final note-list/content-view-model pair for that event-loop turn has settled.
- The selected-note lazy-load contract now also keeps one latest request sequence, allowing the bridge to ignore stale
  same-note body-read completions.
- The bridge now also keeps one internal pending-body adoption helper so selected-note lazy loads can reuse a dirty or
  in-flight editor snapshot from `ContentsEditorIdleSyncController` before falling back to package IO.
- The bridge now also exposes `selectedNoteDirectoryPath`, resolved through the idle-sync boundary.
  QML body-resource rendering can therefore resolve `.wsresource` package references against the same note directory
  that the editor session is currently bound to, instead of re-deriving that path through whichever hierarchy
  view-model happens to be active.
- `selectedNoteBodyNoteId` now makes body ownership explicit for QML/session consumers.
  An empty body string is therefore no longer ambiguous: it can still be attached to a specific selected note as an
  explicit empty-body fallback.
- The bridge now also recognizes an optional content-view-model invokable,
  `noteBodySourceTextForNoteId(noteId)`, as the runtime-snapshot fallback contract for selected note bodies when
  package-path resolution cannot start a lazy load.
- Selection refresh now also retains the previous selected note when a note-backed list still reports visible items but
  transiently exposes an empty `currentNoteId`.
  This prevents one empty-id contract turn from being treated as a real deselection and unmounting the active
  document.

### Classes and Structs
- `ContentsEditorSelectionBridge`

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

- Selection refresh must not directly execute note-file persistence, stat refresh, or open-count maintenance logic.
- `persistEditorTextForNote(...)` must stay as a buffered-stage contract for QML even after the sync split.
- Binding a new content view-model must request one coalesced selected-note rebind into the sync controller so note-path
  resolution uses the settled note-list/content-view-model pair rather than an intermediate mismatch.
- QML must still be able to request a best-effort immediate fetch through `flushEditorTextForNote(...)` when the editor
  lifecycle is ending, but ordinary note switches must not depend on that path.
- Mobile entry-time reconciliation must be available through
  `reconcileViewSessionAndRefreshSnapshotForNote(...)`, so QML can compare a live editor session snapshot against
  filesystem RAW and only then request a refresh.
- The selected-note loading flag must stay aligned with the asynchronous note-body read lifecycle.
- A selected note whose model already exposes `currentBodyText` must not be forced through an empty interim editor state
  while the bridge waits for the lazy package-load path.
- A list-item activation path that only changes the committed `currentIndex` must still refresh `selectedNoteId` and
  `selectedNoteBodyText` for the editor.
- A same-note `currentBodyTextChanged()` from the note-list model must still refresh the bridge body snapshot even when
  the selected note id itself did not change.
- Stale same-note body-read completions must not reclaim the selected note body after a newer request was issued.
- Stale filesystem text must not reclaim the selected note body while the sync controller still owns a newer dirty or
  in-flight editor snapshot for that note.
- A successful same-note persistence completion must not leave `selectedNoteBodyText` behind at the note-open snapshot.
- A same-turn note-list/content-view-model swap must not bind the old note id to the new content view-model or the new
  note id to the old content view-model as an intermediate side effect.
- QML/session code must be able to tell which note owns the currently exposed body text without inferring from timing or
  previous selection state.
- QML body-resource rendering must also be able to read the selected note's resolved package directory from the bridge,
  so inline image/resource rendering does not disappear merely because the hierarchy view-model resolver is between
  rebinds.
- Selection refresh must keep the editor body populated from a runtime snapshot when the content view-model can resolve
  `noteBodySourceTextForNoteId(...)` but the sync boundary cannot resolve a package path for that note.
- A note-backed list with `itemCount > 0` must not clear bridge selection solely because one refresh turn exposed
  `currentNoteId=""`.
- Bridge APIs that target one note must not silently replace a missing `noteId` with the currently selected note.
