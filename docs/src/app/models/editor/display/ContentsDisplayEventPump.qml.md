# `src/app/models/editor/display/ContentsDisplayEventPump.qml`

## Responsibility

Owns editor display timing and signal connection plumbing that must remain in QML.

## Boundary

- Uses `Timer` and `Connections` for model-side orchestration.
- Does not act as a ViewModel.
- Calls the display host and model coordinators through explicit hook functions.
