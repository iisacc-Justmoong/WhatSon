# `src/app/models/editor/persistence`

## Responsibility
Editor-owned persistence orchestration that preserves RAW `.wsnote/.wsnbody` as the only write authority.
This directory owns the note editor save boundary after a live editor session has produced a RAW body snapshot.

## Boundary
- `ContentsEditorPersistenceController` owns editor-session snapshot buffering, immediate enqueue attempts,
  retry/drain scheduling, pending-snapshot adoption, selected-note body reads, and post-save reconcile handoff.
- The controller may call `src/app/models/file/note/ContentsNoteManagementCoordinator` for concrete `.wsnote` IO, but
  file/note code must not decide editor-session save timing or keep editor-owned dirty buffers.
- `ContentsEditorSessionController` decides when the live session should stage or flush RAW text.
- `ContentsEditorSelectionBridge` remains the QML-facing adapter and delegates persistence work into this directory.
- Every helper here must mutate RAW source first, then let parser-derived projections update the UI.

## Current Files
- `ContentsEditorPersistenceController.hpp`
- `ContentsEditorPersistenceController.cpp`
