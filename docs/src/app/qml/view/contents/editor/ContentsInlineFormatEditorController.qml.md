# `src/app/qml/view/contents/editor/ContentsInlineFormatEditorController.qml`

## Responsibility

Provides the QML helper object consumed by `ContentsInlineFormatEditorController`.

## Current Contract

- Exposes focus, selection, native composition, and programmatic text-sync helpers for the C++ controller.
- Restores non-empty selection ranges with native `moveCursorSelection(...)`, preserving the selected text range instead
  of collapsing it while applying the cursor edge.
- Keeps IME state sourced only from `LV.TextEditor.editorItem` (`inputMethodComposing` and `preeditText`).
- Defers programmatic text replacement while native composition is active.
- Tracks focused local cursor/selection interaction and consults `ContentsEditorInputPolicyAdapter` before accepting a
  host-side programmatic text replacement. This preserves OS/Qt selection gestures while the user is selecting text.
- Exposes `applyImmediateProgrammaticText(...)` for focused user commands that intentionally mutate the live editor
  buffer, such as formatting and body-tag insertion shortcuts. This path clears deferred host refresh state and applies
  the command payload immediately because the command already originates from the active text editor.
- Treats an active native selection as local selection interaction even if the platform does not emit every cursor or
  selection notify signal through the helper bridge.
- Does not own persistence or renderer state; it only reflects the mounted editor control and text input.
