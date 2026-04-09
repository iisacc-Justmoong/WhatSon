# `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.cpp`

## Responsibility

This file implements the tag hierarchy runtime behavior. It translates parsed
`WhatSonTagDepthEntry` records into LVRS rows, keeps the writable `WhatSonTagsHierarchyStore`
aligned with those rows, projects `.wshub` notes into a tag-filtered `LibraryNoteListModel`, and
preserves user navigation state across runtime refreshes.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now enforces three rules:

1. Persist the incoming file path and load state.
2. If the incoming tag depth entries are identical to the current hierarchy source, do not rebuild
   the hierarchy model.
3. If the hierarchy source changed, rebuild from sanitized entries but restore the previous
   selection key and all expanded row keys before syncing the model.
4. Regardless of whether the hierarchy rows changed, re-index notes from the owning `.wshub` so the
   tag note-list projection reflects current `.wsnhead` tags.

This prevents watcher-driven runtime loads from collapsing the entire tag tree when note or header
files change elsewhere in the hub.

## ViewModel Hook Contract

`requestViewModelHook()` now refreshes from the persisted `Tags.wstags` source instead of behaving
as a passive notification only.

- `reloadFromTagsFilePath(...)` reparses the file and routes the result through
  `applyRuntimeSnapshot(...)`.
- The viewmodel therefore keeps one refresh path for both runtime bootstrap snapshots and manual
  hook-driven reloads.
- Parse/read failures update load-state error metadata and still emit `viewModelHookRequested()`
  so downstream hook listeners do not lose synchronization events.

## Tag Note Projection

- `loadFromWshub(...)` and `refreshIndexedNotesFromTagsFilePath(...)` index all notes through
  `LibraryAll`.
- `refreshNoteListForSelection()` builds the list bar contents from that cache by matching the
  selected tag subtree against note header tags.
- `depthItems()` now computes each row's `count` from that same subtree projection, so the sidebar
  badge and the note list are backed by one matching rule instead of diverging contracts.
- `depthItems()` also now emits `iconName: vcscurrentBranch` for each tag row, aligning the sidebar
  hierarchy icon with the icon already used by tag metadata chips in note cards/details.
- Subtree matching is inclusive: selecting a parent tag includes notes assigned to any descendant
  tag id/label/path that belongs to that subtree.
- `reloadNoteMetadataForNoteId(...)` re-reads one note document and immediately re-applies the tag
  projection, which is what lets detail-panel tag writes show up in the Tags domain without a full
  hub reload.

## Expansion Ownership

- `setItemExpanded(...)` mutates `m_items[index].expanded` only for valid expandable rows.
- Expansion is part of the viewmodel state, not a disposable view concern.
- `expandedTagItemKeys(...)` and `restoreExpandedTagItemKeys(...)` convert expansion state to stable
  tag keys so a rebuilt tree can recover the user's open branches.

## Mutation And Store Synchronization

- `setTagDepthEntries(...)` is the imperative path used by non-runtime callers.
- `renameItem(...)`, `createFolder()`, and `deleteSelectedFolder()` mutate the store-backed entry
  list and then rebuild/sync the row model.
- `syncStore()` persists the current tag depth entries into `WhatSonTagsHierarchyStore`.
- `syncModel()` publishes the current row vector to `TagsHierarchyModel` and emits QML-facing
  change signals.

## Invariants

- Empty or malformed depth values are normalized before row creation.
- Expansion and selection are keyed by stable tag identity rather than by row index.
- A successful snapshot reload may refresh both note metadata and load-state metadata without
  forcing a visual reset.

## Count Role Compatibility

`depthItems()` still includes a numeric `count` role for every tag row. The role is now populated
from the same tag-subtree projection used by `refreshNoteListForSelection()`, so tag badge counts
update immediately when note metadata reloads or runtime re-indexing changes the matched note set.
