# `src/app/models/editor/bridge/ContentsEditorSelectionBridge.hpp`

## Responsibility

`ContentsEditorSelectionBridge` is the note-selection adapter for editor hosts. It exposes the selected
note id, selected note directory path, resolved RAW body text, body-loading state, and visible note count
to QML while keeping note IO in C++.

## Current Contract

- The bridge owns note-list and content-controller wiring only.
- It delegates selected-note binding, lazy `.wsnbody` reads, snapshot refresh, and
  session/filesystem reconciliation directly to `ContentsNoteManagementCoordinator`.
- It no longer owns or forwards an editor-side buffered persistence controller. The removed
  `ContentsEditorPersistenceController` path must not return.
- `selectedNoteDirectoryPath` remains part of note identity, so duplicate same-id `.wsnote` packages do
  not alias each other during selection or body loading.
- `reconcileViewSessionAndRefreshSnapshotForNote(...)` requires an explicit note id and prefers the
  already resolved selected note directory path when one is available.

## Regression Notes

- Selection refresh must stay coalesced through one queued refresh turn.
- Lazy body reads must preserve request sequencing so stale completions cannot replace a newer selected
  body.
- Runtime snapshots from `noteBodySourceTextForNoteId(...)` remain the fallback only when package-path
  loading cannot start.
- QML must not stage, flush, or buffer editor text through this bridge; editor writes go through
  `ContentsEditorSaveCoordinator` and then the note-management direct write path.
