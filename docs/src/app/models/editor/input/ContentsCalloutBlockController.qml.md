# `src/app/models/editor/input/ContentsCalloutBlockController.qml`

## Responsibility

Owns non-visual callout block editing state for `ContentsCalloutBlock.qml`.

It tracks focused live text, computes cursor geometry, applies focus requests, clears stale selection, and emits
committed callout text changes to the structured flow.

## Contract

- The callout view remains responsible for layout and visual styling.
- The nested inline editor stays in native `TextEdit` input mode.
- Committed text changes are emitted with the previous live text snapshot so the host can reject stale source
  mutations.
- Plain Enter is the only callout-local key path owned here. It is treated as tag management, not ordinary text input:
  the controller asks the backend to close the `<callout>...</callout>` range at the current source cursor.
- Plain Backspace is also tag management only when the callout's live text is empty, the cursor is at position `0`,
  and there is no selection. In that narrow case the controller requests deletion of the whole callout RAW block.
- Shift+Enter is deliberately ignored by the tag-management handler, so Qt's native `TextEdit` inserts a callout body
  line break.
- `Qt.KeypadModifier` is treated as neutral for Enter detection, while Shift and all other text modifiers still fall
  back to native `TextEdit` handling.

## Boundary

Callout typing logic belongs here, but generic key interception does not. Native editing keys stay with the nested
inline editor, except for the explicit plain-Enter callout-exit command and empty-callout Backspace deletion.
