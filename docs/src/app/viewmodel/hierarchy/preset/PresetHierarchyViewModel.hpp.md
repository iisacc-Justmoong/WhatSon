# `src/app/viewmodel/hierarchy/preset/PresetHierarchyViewModel.hpp`

## Responsibility

This header defines the preset hierarchy viewmodel used by the sidebar. It exposes the preset
taxonomy as a QML-friendly item model and declares the mutation and expansion hooks required by the
LVRS hierarchy view.

## Public Contract

- Publishes `itemModel`, `hierarchyModel`, `selectedIndex`, `itemCount`, and load-state properties.
- Implements rename, create, delete, and expansion interfaces in addition to the base hierarchy
  viewmodel contract.
- Accepts direct preset-name updates through `setPresetNames(...)`.
- Accepts runtime-loader updates through `applyRuntimeSnapshot(...)`.

## State Rules

- `m_presetNames` is the canonical preset list.
- `m_items` contains the rendered hierarchy rows and stores `expanded` state.
- `m_presetFilePath` points to the `Preset.wspreset` file used for persistence mutations.

## Refresh Constraints

- Unchanged runtime snapshots must not rebuild the hierarchy rows.
- Changed snapshots must preserve both selection and expansion by stable preset row key.
- Load failures must surface as load-state changes without discarding the current visible rows.
