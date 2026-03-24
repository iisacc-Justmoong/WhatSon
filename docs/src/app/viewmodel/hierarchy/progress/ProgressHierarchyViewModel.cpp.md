# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.cpp`

## Responsibility

This implementation translates the progress hierarchy store into LVRS rows and keeps the progress
sidebar stable across runtime refreshes.

## Static-Tree Refresh Contract

`setProgressState(...)` now updates the domain payload without rebuilding the row tree after the
first initialization.

- The progress value and sanitized state labels are always refreshed.
- The backing store is always synchronized.
- `rebuildItems()` is called only when `m_items` is empty.
- As a result, watcher-driven runtime refreshes keep the user's current expansion state intact.

`applyRuntimeSnapshot(...)` delegates to that logic, so runtime reloads caused by note writes do not
collapse the progress buckets.

## Interaction Semantics

- `setItemExpanded(...)` persists open/closed state inside `m_items`.
- Rename/create/delete remain unavailable in practice because the progress taxonomy is treated as a
  fixed support structure.
- `loadFromWshub(...)` parses `Progress.wsprogress` when it exists and falls back to parser defaults
  when it does not.

## Invariants

- Progress payload changes are allowed.
- Progress bucket topology should remain stable once the initial row tree has been created.
