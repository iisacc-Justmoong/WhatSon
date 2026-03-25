# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Responsibility

This file is the orchestration layer for the library hierarchy domain. It owns UI-facing folder
selection state, note-list filtering, runtime snapshot application, and the translation from
hierarchy edits into persistent mutations.

`WhatSonLibraryIndexedState` now owns the backend note index and smart-bucket projections behind this
viewmodel. `LibraryHierarchyViewModel.cpp` still decides when to mutate or filter the library state,
but it no longer rebuilds `all` / `draft` / `today` note collections manually after every note
write.

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
- `deleteSelectedFolder()` must not write `Folders.wsfolders` directly. It routes through the same
  mutation service so notes that pointed at the deleted folder subtree have those `<folders>`
  bindings pruned from `.wsnhead` instead of keeping ghost assignments.

## Runtime Refresh Contract

- `applyRuntimeSnapshot(...)` now preserves the current hierarchy selection by serialized item key
  and falls back to the previous folder path when a legacy snapshot rebuild changes transient UUIDs,
  instead of resetting to the implicit All Library fallback on every successful reload.
- `applyRuntimeSnapshot(...)` also preserves the set of expanded folder keys before any rebuild and
  restores those openings after the folder rows are reconstructed.
- If the incoming `folderEntries` are identical to the currently rendered hierarchy source, the
  function must not rebuild `m_items` at all. It refreshes only the note collections and the current
  note list binding for the existing selection.
- This matters because hub file watchers reload the runtime after note saves and header/body updates.
  A folder-scoped note list must remain bound to the previously selected `folder:<uuid>` key across
  those refreshes.
- If the previously selected key no longer exists after reload, the viewmodel falls back to the
  unselected state, which still resolves to the All Library bucket.

## Note Filtering And Assignment

- `resolvedNoteFolderBindings(...)` reconstructs a note's effective folder membership and tracks
  whether each binding came from an explicit UUID/full-path assignment or from a legacy leaf-only
  token.
- `effectiveNoteFolderUuids(...)` keeps explicit UUID/full-path bindings even when one folder is an
  ancestor of another. Only legacy leaf-only context tokens are collapsed when they merely exist to
  disambiguate a nested descendant such as `Research` + `/Competitor`.
- `noteMatchesFolderScope(...)` checks the selected folder by UUID rather than by the current path.
- `canAcceptNoteDrop(...)` and `assignNoteToFolder(...)` now read persisted header bindings through
  `WhatSonNoteFolderBindingRepository` instead of trusting runtime arrays alone.
- `assignNoteToFolder(...)` merges header bindings first, then supplements them with runtime state,
  before persisting through `WhatSonNoteFolderBindingRepository`.
- A note can belong to multiple folders at once, so drag-and-drop must append the dropped target
  instead of replacing the existing `<folders>` array.
- A drop onto a folder now repairs stale serialized folder text when the note already has the same
  UUID but an outdated path string.
- A note that is explicitly assigned to both a parent folder and one of its descendants remains
  visible in both folder scopes and keeps both serialized bindings.

## Note List Ordering

`buildNoteListItems(...)` now forwards raw `createdAt` and `lastModifiedAt` values into
`LibraryNoteListItem`.

The actual sort is still performed inside `LibraryNoteListModel`, but the viewmodel is now
responsible for supplying the timestamp keys required to keep the visible note list ordered by the
most recently modified note first after load, refresh, and save.

Primary note preview text also restores `bodyFirstLine` ahead of the truncated plain-text summary
when the rendered first line came from markup that is not preserved as the leading plain-text line.

## Backend Boundary

- note creation, deletion, folder clearing, folder assignment, and body persistence now update the
  shared `WhatSonLibraryIndexedState` backend instead of separately touching `LibraryAll`,
  `LibraryDraft`, and `LibraryToday`
- `notesForBucket(...)` reads the smart buckets from that backend state
- runtime snapshot application delegates the indexed collections into `WhatSonLibraryIndexedState`
  before the viewmodel rebuilds visible folder rows
- note-list rows are now cached per note ID until either the indexed note state changes or the
  rendered folder hierarchy changes, so bucket switches and folder-scope refreshes no longer
  rebuild every visible row from scratch
- folder-scoped note filtering now builds the visible `LibraryNoteListItem` list directly instead
  of first materializing an intermediate filtered `LibraryNoteRecord` vector

## Why This Matters

Before this change, renaming a parent folder changed every descendant full path and could make child
folder notes disappear from the filtered list. With UUID-based identity, a rename only changes the
display path, while the logical folder relationship remains stable and persisted.
