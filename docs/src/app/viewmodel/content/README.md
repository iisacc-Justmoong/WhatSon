# `src/app/viewmodel/content`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/content`
- Child directories: 0
- Child files: 22

## Child Directories
- No child directories.

## Child Files
- `ContentsEditorPresentationProjection.cpp`
- `ContentsEditorPresentationProjection.hpp`
- `ContentsDisplayPresentationRefreshController.cpp`
- `ContentsDisplayPresentationRefreshController.hpp`
- `ContentsDisplaySelectionSyncCoordinator.cpp`
- `ContentsDisplaySelectionSyncCoordinator.hpp`
- `ContentsDisplayStructuredFlowCoordinator.cpp`
- `ContentsDisplayStructuredFlowCoordinator.hpp`
- `ContentsEditorSelectionBridge.cpp`
- `ContentsEditorSelectionBridge.hpp`
- `ContentsGutterMarkerBridge.cpp`
- `ContentsGutterMarkerBridge.hpp`
- `ContentsLogicalTextBridge.cpp`
- `ContentsLogicalTextBridge.hpp`
- `ContentsStructuredDocumentCollectionPolicy.cpp`
- `ContentsStructuredDocumentCollectionPolicy.hpp`
- `ContentsStructuredDocumentFocusPolicy.cpp`
- `ContentsStructuredDocumentFocusPolicy.hpp`
- `ContentsStructuredDocumentHost.cpp`
- `ContentsStructuredDocumentHost.hpp`
- `ContentsStructuredDocumentMutationPolicy.cpp`
- `ContentsStructuredDocumentMutationPolicy.hpp`

## Current Notes

- `ContentsEditorSelectionBridge` no longer owns the asynchronous direct `.wsnote` save queue itself.
- The editor/UI path now stages body-write intent into `file/sync/ContentsEditorIdleSyncController`, which owns the
  buffered note snapshot cache, recurring `1000ms` fetch clock, and best-effort lifecycle flush path.
- `ContentsEditorSelectionBridge` now also prefers that sync-owned dirty snapshot cache when reopening a recently edited
  note, so selection can reuse the newest local body before lazy file IO completes.
- The same bridge now also advances its selected-note body cache on successful same-note persistence completion, so the
  note-open body snapshot cannot keep reclaiming the editor after a save that required no extra filesystem refresh.
- Downstream note-management work still lives under the `file/note` domain in `ContentsNoteManagementCoordinator`,
  which performs actual persistence, open-count maintenance, and tracked-stat follow-up later.
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
- `ContentsDisplayStructuredFlowCoordinator` now owns the note-scoped decision of when parsed structured flow becomes
  the active document host, so display QML no longer keeps that activation policy inline.
- `ContentsEditorPresentationProjection` now centralizes the whole-document RAW-derived editor presentation snapshot.
  Desktop/mobile hosts bind one projection object per note surface instead of keeping separate host-owned
  `ContentsLogicalTextBridge` and `ContentsTextFormatRenderer` state graphs for the same document snapshot.
- `ContentsStructuredDocumentHost` now centralizes structured-flow host state that used to live inline in
  `ContentsStructuredDocumentFlow.qml`, including normalized block/resource collections, pending focus requests,
  active-block tracking, and layout-cache-facing viewport state.
- `ContentsStructuredDocumentCollectionPolicy`, `ContentsStructuredDocumentFocusPolicy`, and
  `ContentsStructuredDocumentMutationPolicy` now split collection normalization, focus resolution, and RAW mutation
  rules into separate C++ SRP units so structured host behavior no longer collapses back into one QML god object.
- `ContentsEditorSelectionBridge` and `ContentsLogicalTextBridge` now also emit verbose editor trace events for
  constructor/destructor turns, selection sync, selected-note body load/reconcile, dirty snapshot adoption, text-state
  rebuilds, and incremental live-typing adoption so the editor pipeline can be followed from the QML host boundary.
- The new coordinators also emit editor trace events for their own state transitions and policy decisions, so the SRP
  split remains observable without pushing orchestration back into QML.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
