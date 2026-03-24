# `src/app/viewmodel/hierarchy/event/EventHierarchyViewModel.cpp`

## Responsibility

This implementation builds the event bucket tree, forwards user mutations into the event hierarchy
store, and protects selection and expansion state from runtime refresh churn.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now treats runtime loads as stateful updates instead of full resets.

- The incoming event names are sanitized before comparison.
- If the sanitized list matches the current `m_eventNames`, the function only updates load-state
  metadata and returns.
- If the list changed, the viewmodel rebuilds the bucket rows, restores expansion by stable row key,
  restores selection by the same key, and then syncs the model.

This prevents unrelated file activity from collapsing the event hierarchy to its default closed
state.

## Expansion Handling

- `setItemExpanded(...)` updates the stored row state for valid expandable rows.
- `expandedEventItemKeys(...)` captures the currently open rows before rebuild.
- `restoreExpandedEventItemKeys(...)` reapplies those openings after `buildBucketItems(...)`.

## Mutation Flow

- `renameItem(...)`, `createFolder()`, and `deleteSelectedFolder()` work against the event-name
  list, then call `syncDomainStoreFromItems()` and `syncModel()`.
- `loadFromWshub(...)` resolves the first available `Event.wsevent`, parses it into
  `WhatSonEventHierarchyStore`, and seeds the row model through `setEventNames(...)`.

## Invariants

- Selection is preserved by semantic row key, never by stale row index.
- Expansion resets only when the underlying event hierarchy truly changes in a way that removes the
  target row.
