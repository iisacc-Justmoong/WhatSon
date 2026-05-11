# `src/app/models/navigationbar/EditorViewModeController.hpp`

## Responsibility
Declares the restored C++ state owner for the navigation bar editor view-mode combo box.

## Public Contract
- `activeViewMode`, `activeViewModeName`, and `activeViewModeController` expose the selected mode to QML.
- Dedicated section controllers are exposed for `Plain`, `Page`, `Print`, `Web`, and `Presentation`.
- `requestViewModeChange(...)` and `requestNextViewMode()` are the narrow mutation entrypoints used by QML.

## Boundary
The controller owns selection state only. It does not parse notes, mount renderers, or alter the LVRS editor file path.
