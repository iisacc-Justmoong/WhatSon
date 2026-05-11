# `src/app/models/navigationbar/EditorViewState.hpp`

## Responsibility
Declares the restored editor view-mode enum used by the navigation bar combo box.

## Contract
- `EditorView` preserves the visible `Plain`, `Page`, `Print`, `Web`, and `Presentation` order.
- Helper functions convert between enum values, integer QML state, display names, and next-mode cycling.

## Boundary
This state drives the navigation chrome selection only. It does not switch the note editor away from the LVRS
`TextEditor` source-of-truth surface.
