# `src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModel.cpp`

## Responsibility

This file implements the dedicated projects hierarchy viewmodel. It parses and mutates
`Projects.wsprojects`, builds the project sidebar rows, and keeps project selection stable across
runtime snapshot updates.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now treats watcher-driven updates conservatively.

- The current selection is captured by a stable project row key before any mutation.
- The incoming folder-depth entries are compared with the currently rendered project hierarchy.
- If the project hierarchy source is unchanged, the function updates only load-state metadata and
  returns.
- If the source changed, the viewmodel rebuilds the project rows and restores the previous
  selection by key instead of dropping the user back to the implicit default state.

Projects now exposes the same expansion hooks used by the shared sidebar footer. When a projects
snapshot eventually contains expandable rows, single-row and bulk expansion both mutate the
viewmodel-owned `expanded` flags and then rebuild the model so the footer menu can drive
`Expand All` / `Collapse All` without pushing that state into QML. If the current projects data is
flat, the footer menu stays disabled because no row advertises `showChevron: true`.

## Mutation Flow

- `loadFromWshub(...)` parses `Projects.wsprojects` into `WhatSonProjectsHierarchyStore`.
- `renameItem(...)`, `createFolder()`, `deleteSelectedFolder()`, and reorder/move helpers mutate the
  store-backed folder entries and then rebuild the model.
- `setItemExpanded(...)` and `setAllItemsExpanded(...)` mutate only in-memory expansion state. They do
  not rewrite `Projects.wsproj`, because fold state is a sidebar presentation concern.
- `itemsFromProjectEntries(...)` is the translation boundary from persisted folder-depth records to
  sidebar rows.

## Invariants

- Selection is semantic and should survive runtime snapshot churn.
- A project snapshot that does not change the rendered hierarchy must not reset the sidebar state.
