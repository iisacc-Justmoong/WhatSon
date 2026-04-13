# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Renders one plain-text document segment inside the structured document-flow editor.

## Key Behavior
- Uses `ContentsTextFormatRenderer.editorSurfaceHtml` so inline style tags still render as styled text instead of raw
  markup.
- Converts edited rich-text surface content back into canonical source with
  `ContentsTextFormatRenderer.normalizeEditorSurfaceTextToSource(...)`.
- The inline editor now uses a paragraph-like minimum height that tracks the actual text content instead of keeping the
  previous 28px floor. This keeps mixed text/image note bodies visually closer to the Figma `294:7933` markdown-style
  reference.
- The block body text now renders with `Font.Medium` at the existing 12px size, matching the current note-body visual
  spec more closely than the previous normal-weight fallback.
- Accepts focus restoration requests by source offset so source rewrites can re-focus the same block after reparsing.
- Focus restoration is now invoked directly by `ContentsStructuredDocumentFlow.qml` on the targeted block instance,
  rather than by rebroadcasting one request through every text block.
- Reports shortcut insertion offsets from the live rich-text cursor by normalizing only the rendered prefix back into
  canonical source, so structured shortcuts can insert at the active text caret instead of always using the block end.
