# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Renders one plain-text document segment inside the structured document-flow editor.

## Key Behavior
- The structured paragraph editor now binds plain visible text (`authoritativePlainText()`) into
  `ContentsInlineFormatEditor.qml` and keeps that nested `TextEdit` in `TextEdit.PlainText` mode.
  Inline block editing therefore no longer depends on Qt RichText document state while the RAW-source mutation bridge
  still remains authoritative.
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
- That live `implicitHeight` is geometry only, not logical line authority.
  Structured gutter numbering must come from the block's authored plain-text newline structure, with height applied
  only afterward to position the already-determined logical lines in the viewport/minimap.
- The block now also exposes logical line layout entries sampled from the live plain-text `TextEdit` through
  `positionToRectangle(...)`.
  Structured gutter Y placement can therefore align each authored line number with the line's actual rendered text row
  instead of evenly dividing the whole paragraph block height.
- The block now also exposes its current local logical line number by counting newline-delimited rows before the live
  caret position in the nested plain-text editor.
  `ContentsStructuredDocumentFlow.qml` uses that value to place current-line indicators on the actual authored line
  inside a multi-line paragraph block.
- While the editor stays focused, the block now also emits `activated()` on cursor moves.
  Structured-flow hosts therefore re-evaluate current-line indicator placement when the caret moves between logical
  lines inside the same paragraph block.
- The block now also exposes whether its nested inline editor currently owns focus.
  `ContentsStructuredDocumentFlow.qml` uses that signal-free focus state to keep host-level idle refresh guards from
  treating an actively edited structured paragraph as unfocused.
- The block body text now renders with `Font.Medium` at the existing 12px size, matching the current note-body visual
  spec more closely than the previous normal-weight fallback.
- Accepts focus restoration requests by source offset so source rewrites can re-focus the same block after reparsing.
- Focus restoration is now invoked directly by `ContentsStructuredDocumentFlow.qml` on the targeted block instance,
  rather than by rebroadcasting one request through every text block.
- Focus restoration requests may now also carry block-local `selectionStart` / `selectionEnd` values.
  After a formatting rewrite reparses the block, the same visible selection can be restored around the newly rewritten
  text instead of collapsing to a caret-only position.
- Reports shortcut insertion offsets from the live plain cursor through the same RAW-source bridge, so structured
  shortcuts can insert at the active text caret without consulting any rendered HTML payload as a source authority.
- The block now also exposes `applyInlineFormatToSelection(tagName)`, which resolves the live plain-text selection,
  rewrites canonical block source through `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`,
  and emits a block-scoped RAW mutation request with both caret and selection restore metadata.
  Inline-format shortcuts inside structured paragraphs therefore no longer depend on the legacy whole-document editor
  selection path.
