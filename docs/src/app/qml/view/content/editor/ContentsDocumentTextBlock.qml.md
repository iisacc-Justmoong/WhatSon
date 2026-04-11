# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Renders one plain-text document segment inside the structured document-flow editor.

## Key Behavior
- Uses `ContentsTextFormatRenderer.editorSurfaceHtml` so inline style tags still render as styled text instead of raw
  markup.
- Converts edited rich-text surface content back into canonical source with
  `ContentsTextFormatRenderer.normalizeEditorSurfaceTextToSource(...)`.
- Accepts focus restoration requests by source offset so source rewrites can re-focus the same block after reparsing.
- Reports shortcut insertion offsets from the live rich-text cursor by normalizing only the rendered prefix back into
  canonical source, so structured shortcuts can insert at the active text caret instead of always using the block end.
