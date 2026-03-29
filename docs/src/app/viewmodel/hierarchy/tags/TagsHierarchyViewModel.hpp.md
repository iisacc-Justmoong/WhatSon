# `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.hpp`

## Responsibility

This header defines the dedicated MVVM boundary for the tags hierarchy. The viewmodel owns the
UI-facing row model, selection state, CRUD entry points, and LVRS expansion hooks for the tag tree.

## Public Contract

- Exposes `itemModel`, `hierarchyModel`, `selectedIndex`, `itemCount`, and load-state properties to
  QML.
- Implements `IHierarchyViewModel`, `IHierarchyRenameCapability`, `IHierarchyCrudCapability`, and
  `IHierarchyExpansionCapability`.
- Provides `setItemExpanded(int, bool)` so LVRS hierarchy rows can persist user-driven fold state
  back into the viewmodel instead of treating expansion as transient view-only state.
- Accepts both imperative `setDepthItems(...)` input and runtime loader input through
  `applyRuntimeSnapshot(...)`.
- Declares inherited capability methods with explicit `override` markers so compiler diagnostics
  catch interface drift immediately and builds stay warning-clean.

## State And Persistence Rules

- `m_entries` is the canonical parsed tag tree payload.
- `m_items` is the rendered LVRS row state, including `expanded`.
- `m_tagsFilePath` tracks the writable `Tags.wstags` target for rename/create/delete mutations.
- `m_createdFolderSequence` is derived from the current data so generated labels do not collide with
  existing tag folders after reload.

## Refresh Semantics

`applyRuntimeSnapshot(...)` is expected to behave conservatively.

- A load failure updates the error state without mutating the current visible hierarchy.
- An unchanged tag snapshot must not rebuild the hierarchy rows.
- A changed snapshot may rebuild rows, but it must preserve both selection and expansion by stable
  tag keys so watcher-driven reloads do not collapse the tree.

## QML Integration

QML callers should treat `selectedIndex` and `setItemExpanded(...)` as the only supported mutable
view-state entry points. Direct row reconstruction in QML would bypass the viewmodel's persistence
and refresh constraints.
