# `src/app/viewmodel/detailPanel/DetailHierarchySelectionViewModel.cpp`

## Responsibility
This implementation keeps the detail panel decoupled from sidebar hierarchy selection churn.
It snapshots incoming selector entries, mirrors file-backed selector selection changes from the injected detail source object, and preserves the detail-panel-local selection across option-list refreshes.

## Synchronization Rules
- `setSourceViewModel(...)` performs an initial one-time import of source items and source selection.
- After that handoff, source item-list refreshes are mirrored and source `selectedIndexChanged()` is accepted only from the injected detail selector source.
- Local `setSelectedIndex(...)` never writes back to the source hierarchy viewmodel.
- Selection preservation prefers `key`, then `itemId`, then `label`.
- Synthetic clear entries such as `No project`, `No bookmark`, and `No progress` are treated like ordinary keyed items, so a cleared selector state survives option-list refreshes without falling back to a domain entry.

## Why It Exists
The detail panel uses the same domain data as the hierarchy selectors, but it cannot share the same mutable selection object.
Without this adapter, clicking a hierarchy row would also mutate the detail-panel combo state and vice versa.
The file-backed detail selector source now uses this adapter to push `.wsnhead` reads and writes back into the combo state without re-coupling the combo to sidebar row clicks.
