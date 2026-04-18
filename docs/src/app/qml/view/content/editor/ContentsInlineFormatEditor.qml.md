# `src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml`

## Responsibility

`ContentsInlineFormatEditor.qml` is the shared plain-text input wrapper for editor surfaces that still need one
whole-buffer `TextEdit`.

The wrapper keeps the host/editor contract expected by `ContentsDisplayView.qml` and structured block delegates:

- `cursorPosition`, `selectionStart`, `selectionEnd`, `selectedText`
- `contentHeight`, `editorItem`, `inputItem`
- `positionToRectangle(...)`, `setProgrammaticText(...)`, `currentPlainText()`

## Current Contract

- The live input surface is always `TextEdit.PlainText`.
- Formatted inline presentation is now a read-side HTML overlay only:
  - hosts pass logical plain text through `text`
  - hosts pass tokenized HTML through `renderedText`
  - the wrapper paints that HTML with a separate `Text` overlay when `showRenderedOutput` is enabled
- The wrapper no longer mounts or switches to a RichText editing mode.
- IME handling, cursor restoration, selection preservation, and `textEdited(...)` dispatch therefore all operate on
  one plain-text buffer only.

## Key Behavior

- Programmatic host sync is deferred only while IME/preedit composition is active.
- `currentPlainText()` now returns the live editor buffer directly instead of recovering text from a serialized Qt
  RichText document.
- `textEdited(...)` remains the host notification hook, but it now always reports plain text.
- The rendered HTML overlay is suppressed during active IME composition so native preedit text stays visible.
- `TextEdit.moveCursorSelection(...)` is still preferred when restoring an existing selection.
- The wrapper now also exposes `clearSelection()`, which explicitly clears `persistentSelection` highlight and any
  cached selection snapshot when a structured-flow activation moves elsewhere.
- macOS `Option` / `Command` + `Up/Down` now route through one dedicated vertical-navigation hook before ordinary
  shortcut dispatch.
  The wrapper computes paragraph/document boundary targets itself, lets structured block delegates override
  cross-block routing through `modifierVerticalNavigationHandler`, and falls back to local cursor collapse when the
  editor is the whole-document host.
- The wrapper keeps the existing external-scroll contract used by page/print layout, gutter, and minimap code.

## Regression Focus

- Programmatic note switches must not emit fake user edits.
- Overlay refresh must not overwrite the live plain-text buffer.
- IME composition must remain visible and must not be interrupted by host-side resync.
- The wrapper must never surface Qt RichText document scaffold as authored note text.
