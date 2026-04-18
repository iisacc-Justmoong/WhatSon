# `src/app/viewmodel/hierarchy/preset/PresetHierarchyViewModel.cpp`

## Responsibility

This file implements preset hierarchy loading, row-model reconstruction, persistence synchronization,
and state preservation across runtime snapshot refreshes.

## Runtime Refresh Contract

`applyRuntimeSnapshot(...)` now mirrors the event hierarchy behavior.

- It sanitizes the incoming preset names before any comparison.
- If the sanitized list matches `m_presetNames`, it only updates load-state metadata.
- If the list changed, it rebuilds the preset bucket rows, restores expanded branches, restores the
  previous selection by stable key, and syncs the model.

This stops note-save related runtime refreshes from collapsing the preset sidebar tree.

## ViewModel Hook Contract

`requestViewModelHook()` performs file-backed self-refresh when the preset source path is known.

- It reloads `Preset.wspreset` through `reloadFromPresetFilePath(...)`.
- On success, the viewmodel re-applies the snapshot pipeline and emits `loadStateChanged` only when
  state actually changed.
- On read/parse failure, it keeps the current rows, records the failure in load-state, and still
  emits `viewModelHookRequested()` so external hook listeners remain deterministic.

## Expansion Ownership

- `setItemExpanded(...)` mutates the stored row state for valid expandable items.
- Expansion is serialized only in memory; it is intentionally treated as user session state rather
  than as part of `Preset.wspreset`.

## Mutation Flow

- `setPresetNames(...)` is the imperative setter used by initial loads.
- `renameItem(...)`, `createFolder()`, and `deleteSelectedFolder()` transform the current item set,
  then rewrite the preset store from those items.
- `syncModel()` is the only path that republishes row vectors to QML.

## Invariants

- Preset hierarchy rows are keyed semantically, not by current row position.
- Rebuilds are allowed only when the preset source materially changed.
- When rows are available, a negative or invalid selected index is normalized to the first visible row so runtime
  refreshes do not leave the viewmodel in an invisible "no selection" state.

## Count Role Compatibility

`depthItems()` now publishes a numeric `count` field on every preset row. The preset domain does
not currently carry note-index membership by preset name, so the value is emitted as `0` while
still satisfying the shared hierarchy payload contract.
