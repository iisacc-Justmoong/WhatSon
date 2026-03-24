# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Responsibility

This file is the orchestration layer for the library hierarchy domain. It owns UI-facing folder
selection state, note-list filtering, runtime snapshot application, and the translation from
hierarchy edits into persistent mutations.

## UUID Migration Summary

The viewmodel treats folder UUID as the canonical runtime identity.

- hierarchy item keys for user folders are `folder:<uuid>`
- selected-folder scope stores `selectedFolderUuid`
- note filtering resolves note assignments by `folderUuids` first
- note creation and drag-and-drop assignment persist both folder path and folder UUID

Path comparison still exists only as a fallback for legacy data that has not been rewritten yet.

## Legacy File Upgrade

`loadFromWshub()` now asks the folder parser whether it had to synthesize UUIDs for legacy
`Folders.wsfolders` content. If so, the viewmodel immediately rewrites the file through
`WhatSonFoldersHierarchyStore`.

This is important because otherwise a legacy folder tree would receive different random UUIDs on the
next launch, and note-folder matching would quietly fall back to path recovery again.

## Folder Mutation Contract

- Folder create, delete, move, LVRS reorder, and rename converge on
  `commitFolderHierarchyUpdate(...)`.
- `buildFolderItems(...)` and `finalizeFolderItems(...)` ensure every runtime folder row has a UUID.
- `folderEntriesFromItems(...)` persists those UUIDs back into `Folders.wsfolders`.
- `applyHierarchyNodes(...)` preserves existing UUIDs across LVRS reordering and can recover identity
  from a `folder:<uuid>` node key when no source index is supplied.

## Note Filtering And Assignment

- `resolvedNoteFolderUuids(...)` reconstructs a note's effective folder membership.
- `canonicalLeafFolderUuids(...)` removes redundant ancestor assignments before comparison.
- `noteMatchesFolderScope(...)` checks the selected folder by UUID rather than by the current path.
- `canAcceptNoteDrop(...)` and `assignNoteToFolder(...)` now read persisted header bindings through
  `WhatSonNoteFolderBindingRepository` instead of trusting runtime arrays alone.
- `assignNoteToFolder(...)` merges header bindings first, then supplements them with runtime state,
  before persisting through `WhatSonNoteFolderBindingRepository`.
- A note can belong to multiple folders at once, so drag-and-drop must append the dropped target
  instead of replacing the existing `<folders>` array.
- A drop onto a folder now repairs stale serialized folder text when the note already has the same
  UUID but an outdated path string.

## Why This Matters

Before this change, renaming a parent folder changed every descendant full path and could make child
folder notes disappear from the filtered list. With UUID-based identity, a rename only changes the
display path, while the logical folder relationship remains stable and persisted.
