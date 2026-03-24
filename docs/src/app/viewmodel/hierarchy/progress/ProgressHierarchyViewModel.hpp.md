# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModel.hpp`

## Responsibility

This header declares the progress hierarchy viewmodel. It presents the progress buckets defined by
`Progress.wsprogress` while exposing selection, load state, and expansion control to QML.

## Public Contract

- Publishes the row model and the usual hierarchy-facing observable properties.
- Implements rename/create/delete/expansion capability interfaces on top of the shared hierarchy
  base interface.
- Exposes `setProgressState(int, QStringList)` for imperative input and `applyRuntimeSnapshot(...)`
  for runtime-loader refreshes.
- Exposes `setItemExpanded(int, bool)` so the viewmodel, not the delegate tree, owns fold state.

## Refresh Rules

- The progress bucket taxonomy is effectively static after first construction.
- Snapshot updates may refresh progress value and state labels without rebuilding existing rows when
  the row tree is already initialized.
- Expansion state must therefore survive repeated runtime updates.

## Internal State

- `m_progressValue` and `m_progressStates` hold the current domain payload.
- `m_items` stores the rendered row tree and expansion state.
- `m_progressFilePath` points to the active `Progress.wsprogress` file used for loading.
