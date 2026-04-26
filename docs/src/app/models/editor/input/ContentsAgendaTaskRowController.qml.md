# `src/app/models/editor/input/ContentsAgendaTaskRowController.qml`

## Responsibility

Owns non-visual agenda task row editing state for `ContentsAgendaBlock.qml`.

It tracks the live task text snapshot, focus/cursor operations, checkbox-toggle forwarding, selection cleanup, cursor
geometry, and committed task text emission.

## Contract

- The task row view draws the checkbox, inline editor, and row layout.
- The nested inline editor stays on native `TextEdit` input.
- Committed task text changes are emitted through `agendaBlock.taskTextChanged(...)` with the task data and live cursor
  position.

## Boundary

The controller handles row state and source-change emission only. It must not introduce generic text-key interception.
