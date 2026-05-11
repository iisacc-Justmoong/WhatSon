# `src/app/models/navigationbar/EditorViewState.cpp`

## Responsibility
Implements value validation, enum conversion, display-name lookup, and cyclic traversal for the restored editor
view-mode combo.

## Notes
- Invalid values resolve to `Plain` only after callers explicitly choose to convert them.
- `editorViewNameFromValue(...)` returns an empty string for invalid input so QML/controller tests can reject stale
  menu indices without mutating state.
