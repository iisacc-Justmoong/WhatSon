# `src/app/qml/view/panels/sidebar/SidebarHierarchySelectionController.qml`

## Responsibility

`SidebarHierarchySelectionController.qml` owns hierarchy multi-selection behavior for
`SidebarHierarchyView.qml`.

It centralizes pointer-modifier capture, range/toggle selection semantics, and primary-row activation so the root
sidebar view can stay focused on LVRS composition and overlay rendering.

## Public Contract

- `selectionAnchorIndex`: hierarchy range-selection anchor.
- `selectedIndices`: normalized set of visually selected hierarchy rows.
- `pointerSelectionModifiers` / `pointerSelectionModifiersCapturedAtMs`: short-lived press cache for modifier recovery.
- `requestHierarchySelection(item, resolvedIndex, modifiers)`: main hierarchy selection entry point.
- `syncHierarchySelectionFromSelectedFolder()`: rehydrates visual selection from the routed folder index.

## Modifier Recovery

- `captureHierarchyPointerSelectionModifiers(...)` stores press-time `Cmd/Ctrl` or `Shift` state.
- `resolveHierarchySelectionModifiers(...)` falls back to the cached modifier snapshot when activation callbacks arrive
  without modifier bits.
- This keeps `Cmd/Ctrl` toggle selection and `Shift` range selection stable across LVRS/platform callback timing
  differences.

## Host Dependency Direction

- The controller depends on `SidebarHierarchyView` helpers such as `normalizedInteger(...)`,
  `invalidateHierarchySelectionVisuals()`, `resolveVisibleHierarchyItem(...)`, and
  `hierarchyViewModel.setHierarchySelectedIndex(...)`.
- The host view retains wrapper functions so external callers keep a stable interface while the selection state machine
  lives in a dedicated sibling file.
