# `src/app/qml/view/panels/ListBarLayout.qml`

## Responsibility
`ListBarLayout.qml` is the desktop list surface for note and resource rows. It owns tap, drag, context-menu, and viewport interactions while note-list probing remains delegated to C++ bridge objects.

## Composition Model
- The root view delegates note multi-selection state and modifier interpretation to `noteSelectionController`.
- Delegates and peer panels use wrapper functions such as `requestNoteSelection(...)` and `syncSelectionFromCommittedState()`.
- Visual delegates switch between `NoteListItem` and `ResourceListItem` by model contract.

## Viewport Contract
- The visible `ListView` binds directly to `resolvedNoteListModel`.
- The parent shell supplies `noteListModel`; this view does not rediscover it through hierarchy toolbar indexes.
- Viewport motion uses the desktop narrow-step contract with `Flickable.StopAtBounds`, `LV.WheelScrollGuard`, and restore-after-reset handling.
- Drag-preview badge text, row spacing, and toolbar height use LVRS theme tokens instead of raw visual literals.

## Interaction Contract
- `activateNoteIndex(index, noteId)` is the only immediate note activation path.
- Pointer selection routes through `requestNoteSelection(index, noteId, modifiers)`.
- Desktop internal drag is immediate; row assignment and context actions share the same target selection rules.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp` and selection-related C++ regression tests keep the list contract reachable through the desktop workspace shell.
