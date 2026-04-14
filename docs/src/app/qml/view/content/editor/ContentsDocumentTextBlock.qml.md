# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Renders one plain-text document segment inside the structured document-flow editor.

## Key Behavior
- Uses `ContentsTextFormatRenderer.editorSurfaceHtml` so inline style tags still render as styled text instead of raw
  markup.
- When the shared inline-editor wrapper exposes `currentPlainText()`, the block now uses that helper as its current
  visible plain-text snapshot before diffing RAW block source.
- The live block-edit path no longer converts edited RichText surface HTML back into canonical source.
  It now derives a plain-text delta from the previous RAW block source vs the current visible text, maps that delta
  back into source offsets with `ContentsStructuredCursorSupport.js`, and applies the replacement directly to the RAW
  block source with `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`.
- Inline text-block focus restore and caret-origin mutation requests are now both mapped through an inline-tag-aware
  source/plain cursor bridge instead of treating `TextEdit.cursorPosition` as a raw source offset.
  Closing tags such as `</highlight>` therefore stay zero-width for caret restore and `Enter`/typing mutations, so a
  styled paragraph tail is no longer re-targeted into the wrong block or peeled off one character at a time after a
  split/reparse turn.
- The inline editor now uses a paragraph-like minimum height that tracks the actual text content instead of keeping the
  previous 28px floor. This keeps mixed text/image note bodies visually closer to the Figma `294:7933` markdown-style
  reference.
- The block now depends on `ContentsInlineFormatEditor.qml` publishing a real `implicitHeight` from live text content.
  That keeps text delegates materially present inside `ContentsStructuredDocumentFlow.qml` instead of collapsing to
  zero height while neighboring resource/image blocks remain visible.
- The block body text now renders with `Font.Medium` at the existing 12px size, matching the current note-body visual
  spec more closely than the previous normal-weight fallback.
- Accepts focus restoration requests by source offset so source rewrites can re-focus the same block after reparsing.
- Focus restoration is now invoked directly by `ContentsStructuredDocumentFlow.qml` on the targeted block instance,
  rather than by rebroadcasting one request through every text block.
- Reports shortcut insertion offsets from the live plain cursor through the same RAW-source bridge, so structured
  shortcuts can insert at the active text caret without consulting rendered RichText HTML as a source authority.
