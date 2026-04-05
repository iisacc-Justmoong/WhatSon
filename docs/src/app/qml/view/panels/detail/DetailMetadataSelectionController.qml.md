# `src/app/qml/view/panels/detail/DetailMetadataSelectionController.qml`

## Responsibility

This helper owns modifier-aware selection state for the compact metadata lists used by the detail panel.

## Selection Rules

- Plain click collapses the selection to a single row and promotes that row to the section's primary `activeIndex`.
- `Cmd/Ctrl + click` toggles the clicked row in or out of the visual selection set.
- `Shift + click` selects a contiguous range from `selectionAnchorIndex` to the clicked row.
- `Cmd/Ctrl + Shift + click` unions the contiguous range with the existing selection set.

## Primary Selection Contract

- The detail panel still exposes only one committed metadata index (`activeFolderIndex` / `activeTagIndex`) through the
  properties viewmodel.
- This controller therefore keeps a QML-local `selectedIndices` array for visual multi-selection, but always routes a
  primary row back through `section.itemTriggered(...)` so delete and other active-row actions continue to operate on a
  single committed index.
- When the committed active row falls out of the current visual set, the controller promotes the last remaining selected
  row as the new primary active row.

## Modifier Recovery

- The helper caches press-time pointer modifiers for a short window, mirroring the existing sidebar/list-bar pattern.
- This prevents quick `Cmd/Ctrl` or `Shift` clicks from degrading into plain clicks when LVRS click callbacks arrive
  after pointer-up.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Plain click on a metadata row must leave exactly one selected row.
  - `Cmd/Ctrl + click` must toggle only the targeted row while preserving the others.
  - `Shift + click` must select a contiguous range from the anchor row.
  - Plain click after a modifier-driven multi-selection must collapse the list back to a single selected row.
