# `src/app/qml/view/content/editor/ContentsInlineFormatEditorController.qml`

## Responsibility

Provides the QML helper object consumed by `ContentsInlineFormatEditorController`.

## Current Contract

- Exposes focus, selection, native composition, and programmatic text-sync helpers for the C++ controller.
- Restores non-empty selection ranges with native `moveCursorSelection(...)`, preserving the selected text range instead
  of collapsing it while applying the cursor edge.
- Keeps IME state sourced only from `LV.TextEditor.editorItem` (`inputMethodComposing` and `preeditText`).
- Defers programmatic text replacement while native composition is active.
- Does not own persistence or renderer state; it only reflects the mounted editor control and text input.
