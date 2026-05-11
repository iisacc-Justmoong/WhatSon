# `src/app/models/navigationbar/EditorViewModeController.cpp`

## Responsibility
Implements restored editor view-mode state transitions for the navigation bar combo box.

## Behavior
- Starts in `Plain`.
- Rejects invalid mode values without emitting `activeViewModeChanged()`.
- Keeps each `EditorViewSectionController.active` flag synchronized with the selected mode.
- Cycles through `Plain -> Page -> Print -> Web -> Presentation -> Plain`.

## Tests
`test/cpp/suites/editor_view_mode_controller_tests.cpp` covers initial state, invalid values, direct selection, section
activation, and cyclic traversal.
