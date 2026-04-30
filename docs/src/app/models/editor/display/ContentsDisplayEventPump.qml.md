# `src/app/models/editor/display/ContentsDisplayEventPump.qml`

## Responsibility

Owns editor display timing and signal connection plumbing that must remain in QML.

## Boundary

- Uses `Timer` and `Connections` for model-side orchestration.
- Does not act as a Controller.
- Calls the display host and model coordinators through explicit hook functions.
- When renderer block output arrives for a pending note entry, it asks the display host for the structured
  editor-open layout refresh path instead of issuing a single cache refresh directly.
  That keeps delegate-settled gutter geometry under the structured flow's lifecycle hook.
