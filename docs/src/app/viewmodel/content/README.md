# `src/app/viewmodel/content`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/content`
- Child directories: 0
- Child files: 24

## Child Directories
- No child directories.

## Child Files
- `ContentsEditorPresentationProjection.cpp`
- `ContentsEditorPresentationProjection.hpp`
- `ContentsEditorSessionController.cpp`
- `ContentsEditorSessionController.hpp`
- `ContentsDisplayPresentationRefreshController.cpp`
- `ContentsDisplayPresentationRefreshController.hpp`
- `ContentsDisplaySelectionSyncCoordinator.cpp`
- `ContentsDisplaySelectionSyncCoordinator.hpp`
- `ContentsEditorSelectionBridge.cpp`
- `ContentsEditorSelectionBridge.hpp`
- `ContentsGutterMarkerBridge.cpp`
- `ContentsGutterMarkerBridge.hpp`
- `ContentsLogicalTextBridge.cpp`
- `ContentsLogicalTextBridge.hpp`
- `ContentsStructuredDocumentBlocksModel.cpp`
- `ContentsStructuredDocumentBlocksModel.hpp`
- `ContentsStructuredDocumentCollectionPolicy.cpp`
- `ContentsStructuredDocumentCollectionPolicy.hpp`
- `ContentsStructuredDocumentFocusPolicy.cpp`
- `ContentsStructuredDocumentFocusPolicy.hpp`
- `ContentsStructuredDocumentHost.cpp`
- `ContentsStructuredDocumentHost.hpp`
- `ContentsStructuredDocumentMutationPolicy.cpp`
- `ContentsStructuredDocumentMutationPolicy.hpp`

## Current Notes

- `ContentsEditorSessionController` now owns editor-session RAW sync acceptance, agenda/empty-block normalization,
  pending-save state, and persistence enqueue decisions in C++ instead of keeping those calculations in QML.
- That same controller now also treats the live editor session as authoritative for same-note mismatch resolution once
  local edits exist, so delayed RAW/model refreshes cannot reclaim modified text until RAW has been repaired.
- `ContentsDisplayView.qml` now mounts `ContentsEditorSessionController` directly for the primary editor session path,
  and no QML session controller remains.
- `ContentsEditorSelectionBridge` no longer owns the asynchronous direct `.wsnote` save queue itself.
- The editor/UI path now stages body-write intent into
  `src/app/models/editor/persistence/ContentsEditorPersistenceController`, which owns the buffered note snapshot
  cache, recurring `1000ms` persistence drain clock, and best-effort lifecycle flush path.
- `ContentsEditorSelectionBridge` now also prefers that persistence-owned dirty snapshot cache when reopening a
  recently edited note, so selection can reuse the newest local body before lazy file IO completes.
- The same bridge now also advances its selected-note body cache on successful same-note persistence completion, so the
  note-open body snapshot cannot keep reclaiming the editor after a save that required no extra filesystem refresh.
- Downstream note-management work still lives under the `file/note` domain in `ContentsNoteManagementCoordinator`,
  which performs actual persistence, open-count maintenance, and tracked-stat follow-up later.
- `ContentsNoteManagementCoordinator` now also accepts editor-authoritative reconcile requests, repairing RAW from the
  current editor snapshot before it allows the visible note snapshot to refresh.
- Note-selection changes now reuse the same `{noteId, noteDirectoryPath}` metadata session and no longer trigger a
  hub-wide `.wsnbody` stat refresh just to bump `openCount`.
- The coordinator now applies persisted body state and schedules tracked-stat refresh after background completion returns
  to the main thread.
- `ContentsLogicalTextBridge` now also exports its cached logical-to-source offset table in one QML call so the editor
  typing hot path can reseed once per presentation commit and avoid whole-note bridge regeneration on every typed
  character.
- `ContentsLogicalTextBridge` now also accepts incremental live typing adoption from QML, so the bridge no longer needs
  to rebuild line starts and logical/source offset tables from the whole note after every committed character.
- `ContentsLogicalTextBridge` now normalizes Qt container `size()` values through a bounded integer helper before
  reserve/export paths, which keeps Apple libc++ from failing mixed `int` / `qsizetype` template deduction in the
  live-typing bridge code.
- `ContentsDisplaySelectionSyncCoordinator` now owns note-selection sync queuing, note snapshot reconcile gating, and
  pending editor-focus intent for both desktop and mobile editor hosts.
- `ContentsDisplayPresentationRefreshController` now owns whole-document presentation refresh policy, including
  projection-disabled clearing, focused-input defer decisions, and deferred timer-trigger decisions.
- `ContentsDisplayView.qml` no longer keeps a second structured-flow activation latch above the selected RAW snapshot.
- Once a note is selected and the RAW body snapshot is available, the structured document host can mount immediately
  from that snapshot instead of waiting for a separate editor-session-bound confirmation path.
- Note-backed hierarchy note-list models now also keep the selected row's RAW/source body snapshot in their
  `currentBodyText` contract consistently across Library, Projects, Bookmarks, and Progress.
  `ContentsEditorSelectionBridge` can therefore bootstrap the visible note body from the active note-list row before
  it needs to fall back to the content-view-model runtime snapshot or filesystem reload path.
- That same bridge now also treats note-list `currentIndexChanged()` as a committed selection refresh source and
  invalidates its selected-note body snapshot on `currentBodyTextChanged()`.
  List-item activation and late row-body hydration therefore keep propagating into the editor even when the note id
  signal order is delayed or unchanged.
- The traced note-open RAW path for editor hosts is now explicitly:
  - `.wsnbody`
  - `WhatSonLocalNoteFileStore::applyBodyDocumentText(...)`
  - `WhatSon::NoteBodyPersistence::sourceTextFromBodyDocument(...)`
  - note-list `currentBodyText` or content-view-model `noteBodySourceTextForNoteId(...)`
  - `ContentsEditorSelectionBridge`
  - `ContentsDisplayView.documentPresentationSourceText`
  - `ContentsEditorPresentationProjection` / `ContentsStructuredBlockRenderer`
- The storage read side now strips rendered-editor HTML block artifacts before that path begins, so HTML comment
  placeholders and non-canonical block controllers are treated as suspicious input instead of editor-authoritative RAW.
- Note-backed hierarchy viewmodels now also expose `noteBodySourceTextForNoteId(...)` as a shared runtime fallback
  contract, so the selection bridge can recover RAW note body source even when a path-based `.wsnbody` reload is not
  immediately available.
- `ContentsEditorPresentationProjection` now centralizes the whole-document RAW-derived editor presentation snapshot.
  Desktop/mobile hosts bind one projection object per note surface instead of keeping separate host-owned
  `ContentsLogicalTextBridge` and `ContentsTextFormatRenderer` state graphs for the same document snapshot.
- Canonical RAW resource-tag construction now lives under
  `src/app/models/editor/tags/ContentsResourceTagTextGenerator.*` so non-format body tag helpers stay in the editor
  tag boundary.
- `ContentsStructuredDocumentMutationPolicy` now also accepts QML array/list variants robustly for resource insertion,
  so drag/drop tag blocks do not silently no-op when QML hands the policy a non-`QVariantList` sequential value.
- That same mutation policy now compares wrapped QML container/value types through `QMetaType::fromType<T>()`,
  keeping the `QJSValue` unwrap path compatible with the Qt 6.8 headers used by the app build.
- The mutation policy now also owns paragraph-boundary RAW rewrites for structured prose blocks.
  Adjacent `paragraph` / `p` blocks merge through one canonical payload builder, and paragraph split rewrites clone
  explicit controllers when needed instead of letting QML delegates perform ad-hoc tag surgery.
- `ContentsStructuredDocumentHost` now centralizes structured-flow host state that used to live inline in
  `ContentsStructuredDocumentFlow.qml`, including normalized block/resource collections, pending focus requests,
  active-block tracking, and layout-cache-facing viewport state.
- `ContentsStructuredDocumentBlocksModel` now sits between parsed `renderedDocumentBlocks` snapshots and the QML
  `Repeater` host.
  Structured single-line deletes and other small RAW edits therefore keep stable delegate rows for unchanged suffix
  blocks instead of remounting the whole document column every time parsed source offsets shift.
- That host now also owns the selection-clear revision and retained-block hint used to drop stale
  `persistentSelection` highlight when another structured editor becomes active. QML delegates route live
  `TextEdit.cursorPosition` changes through a cursor-only host method, so native `TextEdit` drag selection and iOS
  selection gestures are not collapsed by a cleanup tick.
- `ContentsStructuredDocumentCollectionPolicy`, `ContentsStructuredDocumentFocusPolicy`, and
  `ContentsStructuredDocumentMutationPolicy` now split collection normalization, focus resolution, and RAW mutation
  rules into separate C++ SRP units so structured host behavior no longer collapses back into one QML god object.
- Automated C++ regression coverage for this directory now lives in
  `test/cpp/suites/*.cpp`, locking imported resource descriptor normalization/tag generation through the editor tags
  boundary, collection normalization/resource resolution for
  `ContentsStructuredDocumentCollectionPolicy`, structured deletion/insertion payload generation for
  `ContentsStructuredDocumentMutationPolicy`, stable row retention/removal behavior in
  `ContentsStructuredDocumentBlocksModel`, selection-clear revision behavior in
  `ContentsStructuredDocumentHost`, same-note editor-authority protection in
  `ContentsEditorSessionController`, and editor-authoritative RAW repair in
  `ContentsNoteManagementCoordinator`.
- That same regression suite now also locks paragraph merge/split payload generation in
  `ContentsStructuredDocumentMutationPolicy`, covering both implicit newline-delimited prose and explicit
  `<paragraph>...</paragraph>` controllers.
- `ContentsStructuredDocumentFocusPolicy` now also resolves structured shortcut/resource insertion anchors from
  `{focused block hint, active block, pending focus request, RAW source}`.
  `ContentsStructuredDocumentFlow.qml` no longer keeps the fallback policy that guessed insertion at a block or
  document tail when no live caret-derived source offset was available.
- `ContentsEditorSelectionBridge` and `ContentsLogicalTextBridge` now also emit verbose editor trace events for
  constructor/destructor turns, selection sync, selected-note body load/reconcile, dirty snapshot adoption, text-state
  rebuilds, and incremental live-typing adoption so the editor pipeline can be followed from the QML host boundary.
- The new coordinators also emit editor trace events for their own state transitions and policy decisions, so the SRP
  split remains observable without pushing orchestration back into QML.
- The editor-creation path now also traces QML host decisions explicitly:
  - which note/body pair the host is trying to mount
  - which structured document surface policy became active
  - which structured document-flow object instance became the active editor surface
  - which host-owned collaborators (`selectionBridge`, mount coordinator, display session coordinator, editor session)
    were created and attached for that note-selection turn
- The note-selection path now also exposes one grep-friendly trace vocabulary from selection to mount:
  - `ContentsEditorSelectionBridge` emits `selectionFlow.*` turns for note-list activation, queued refresh, settled
    note transition, and selected-body load decisions
  - `ContentsDisplaySelectionSyncCoordinator` emits `selectionFlow.flushPlan`, while
    `ContentsDisplayView.qml` mirrors the handled result as
    `selectionFlow.selectionSyncPlan` / `selectionFlow.selectionSyncResult`
  - `ContentsDisplayNoteBodyMountCoordinator` emits `selectionFlow.mountPlan` for each mount/snapshot-refresh/editor
    binding decision
  - `ContentsDisplayView.qml` mirrors those plans with `selectionFlow.mountPlan` / `selectionFlow.mountResult`,
    making it possible to reconstruct exactly what happened after a specific note id was clicked
- `ContentsDisplayView.qml` now fans one `scheduleSelectionModelSync(...)` call into both the selection-sync
  coordinator and the mount coordinator, with separate `Connections` handlers for
  `selectionSyncFlushRequested(...)` and `mountFlushRequested(...)`.
  A note-open turn therefore cannot silently drop the mount-plan execution by attaching the wrong target object.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
