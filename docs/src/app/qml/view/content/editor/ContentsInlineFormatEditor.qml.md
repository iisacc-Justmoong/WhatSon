# `src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml`

## Responsibility

`ContentsInlineFormatEditor.qml` is the dedicated RichText input surface for `.wsnbody` inline-format editing.

It keeps the editor contract expected by `ContentsDisplayView.qml` (`cursorPosition`, `selectionStart`,
`selectionEnd`, `selectedText`, `contentHeight`, `editorItem`, `inputItem`, `positionToRectangle(...)`) while using a
plain `QtQuick.TextEdit` as the actual rendering and input engine.

## Key Behavior

- Accepts `renderedEditorText` as an input-only property (`text`) and syncs it into the underlying `TextEdit`
  programmatically.
- Exposes a `textEdited(string text)` signal as a change event for the host controllers.
- The host no longer persists that whole-document RichText payload directly for ordinary typing.
- Instead, the typing controller treats the signal as a notification and derives the actual mutation from
  `getText(...)` plus the current source/plain-text bridges.
- The editor still emits the live `TextEdit.text` payload for formatting-oriented consumers. The earlier
  fragment-based `getFormattedText(...)` save path was removed because it leaked `StartFragment`/fragment-scaffold
  markup into note editing.
- Exposes direct Qt-style selection/text helpers on the wrapper itself:
  - `selectionSnapshot()`
  - `getText(start, end)`
  - `getFormattedText(start, end)`
  - `length`
  - `hasSelection`
- Keeps the scroll contract compatible with the existing gutter/minimap code:
  - `editorItem.parent.y` follows the `Flickable.contentItem` offset
  - `editorItem.parent.parent` resolves back to the owning `Flickable`
- Preserves the legacy nested editor access pattern:
  - `editorItem`
  - `editorItem.activeFocus` (native `Item` focus state)
  - `editorItem.inputItem`
  - `editorItem.inputItem.activeFocus`
  - `editorItem.positionToRectangle(...)`
- Falls back to `TextEdit.onTextChanged` dispatch when a native `textEdited` signal is unavailable, while suppressing
  programmatic text-sync loops.
- Programmatic text-sync now preserves the pre-sync logical cursor/selection range when the visible plain-text payload is
  unchanged and only the RichText markup wrapper changed (for example after inline formatting wraps).
- Selection/formatting controllers should prefer these wrapper-level Qt helpers over re-walking nested `editorItem` /
  `inputItem` objects.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - RichText spans derived from `.wsnbody` tags render visibly inside the live editor surface
  - cursor/selection updates still drive gutter and minimap geometry
  - programmatic note switches do not emit duplicate save mutations
  - direct typing must not surface fragment comment markup such as `<!--StartFragment-->`
