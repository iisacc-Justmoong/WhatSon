# `src/app/qml/view/panels/ListBarSelectionController.qml`

## Responsibility

`ListBarSelectionController.qml` owns note-list multi-selection state and modifier interpretation for
`ListBarLayout.qml`.

The controller keeps selection anchor bookkeeping, normalizes modifier payloads, and decides whether a pointer
activation should replace, extend, or toggle the current note selection.

## Public Contract

- `selectionAnchorIndex`: range-selection anchor preserved across click sequences.
- `selectedIndices`: normalized visual multi-selection set consumed by the host view.
- `requestNoteSelection(index, noteId, modifiers)`: main selection entry point for pointer activation.
- `syncSelectionFromCommittedState()`: rehydrates selection from the model-authoritative current index.

## Selection Rules

- `normalizedKeyboardModifiers(...)` merges event modifiers with `Qt.application.keyboardModifiers`.
- `Shift` extends selection from `selectionAnchorIndex`.
- `Cmd/Ctrl` toggles membership without losing the model-authoritative current note.
- `Cmd/Ctrl + Shift` unions the current selection with the requested anchor range.
- When a toggled-off row was also the committed current note, the controller activates the latest surviving row as a
  fallback primary selection.

## Host Dependency Direction

- The controller depends on `view.normalizeCurrentIndex(...)`, `view.currentIndexFromModel()`, and
  `view.activateNoteIndex(...)`.
- The host view keeps wrapper functions so existing delegates can call the same API surface while the selection logic
  lives in a separate sibling object.
