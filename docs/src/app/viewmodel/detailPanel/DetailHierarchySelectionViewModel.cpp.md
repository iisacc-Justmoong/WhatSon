# `src/app/viewmodel/detailPanel/DetailHierarchySelectionViewModel.cpp`

## Responsibility
This implementation keeps the detail panel decoupled from sidebar hierarchy selection churn.
It listens only for source `hierarchyModelChanged()` events, snapshots the incoming entry list, and preserves the detail-panel-local selection across source refreshes.

## Synchronization Rules
- `setSourceViewModel(...)` performs an initial one-time import of source items and source selection.
- After that handoff, only source item-list refreshes are mirrored.
- Local `setSelectedIndex(...)` never writes back to the source hierarchy viewmodel.
- Selection preservation prefers `key`, then `itemId`, then `label`.

## Why It Exists
The detail panel uses the same domain data as the hierarchy selectors, but it cannot share the same mutable selection object.
Without this adapter, clicking a hierarchy row would also mutate the detail-panel combo state and vice versa.
