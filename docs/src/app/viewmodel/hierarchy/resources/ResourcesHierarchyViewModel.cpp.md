# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp`

## Responsibility

This implementation exposes the resources taxonomy to the sidebar. Unlike mutable folder-based
hierarchies, the resource bucket tree is static and the runtime payload only changes the attached
resource path list.

## Static-Tree Refresh Contract

`setResourcePaths(...)` now separates data refresh from tree reconstruction.

- The incoming path list is sanitized and stored in `m_resourcePaths`.
- The store is updated on every call.
- The hierarchy rows are built only once, when `m_items` is still empty.
- Later runtime refreshes keep the existing row vector intact, which preserves LVRS expansion state
  and selection instead of resetting the tree.

`applyRuntimeSnapshot(...)` delegates to `setResourcePaths(...)`, so note-save triggered runtime
reloads no longer collapse the resources tree.

## Interaction Semantics

- `setItemExpanded(...)` is now a real state mutation entry point.
- Rename/create/delete remain rejected because the resources taxonomy is intentionally read-only.
- `loadFromWshub(...)` parses `Resources.wsresources` when present, or falls back to an empty payload
  while keeping the static supported-type tree available to QML.

## Invariants

- The resource path list may change frequently.
- The resource category tree should not change as a side effect of those path updates.
