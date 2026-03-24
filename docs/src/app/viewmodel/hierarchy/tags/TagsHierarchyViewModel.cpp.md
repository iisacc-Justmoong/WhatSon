# `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.cpp`

## Responsibility

This file implements the tag hierarchy runtime behavior. It translates parsed
`WhatSonTagDepthEntry` records into LVRS rows, keeps the writable `WhatSonTagsHierarchyStore`
aligned with those rows, and preserves user navigation state across runtime refreshes.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now enforces three rules:

1. Persist the incoming file path and load state.
2. If the incoming tag depth entries are identical to the current hierarchy source, do not rebuild
   the hierarchy model.
3. If the hierarchy source changed, rebuild from sanitized entries but restore the previous
   selection key and all expanded row keys before syncing the model.

This prevents watcher-driven runtime loads from collapsing the entire tag tree when note or header
files change elsewhere in the hub.

## Expansion Ownership

- `setItemExpanded(...)` mutates `m_items[index].expanded` only for valid expandable rows.
- Expansion is part of the viewmodel state, not a disposable view concern.
- `expandedTagItemKeys(...)` and `restoreExpandedTagItemKeys(...)` convert expansion state to stable
  tag keys so a rebuilt tree can recover the user's open branches.

## Mutation And Store Synchronization

- `setTagDepthEntries(...)` is the imperative path used by tests and non-runtime callers.
- `renameItem(...)`, `createFolder()`, and `deleteSelectedFolder()` mutate the store-backed entry
  list and then rebuild/sync the row model.
- `syncStore()` persists the current tag depth entries into `WhatSonTagsHierarchyStore`.
- `syncModel()` publishes the current row vector to `TagsHierarchyModel` and emits QML-facing
  change signals.

## Invariants

- Empty or malformed depth values are normalized before row creation.
- Expansion and selection are keyed by stable tag identity rather than by row index.
- A successful snapshot reload may refresh load-state metadata without forcing a visual reset.
