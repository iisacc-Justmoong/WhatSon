# `src/app/editor/renderer/ContentsTextFormatRenderer.hpp`

## Responsibility
Declares the editor-side rich-text renderer bridge that converts `.wsnbody` inline style tags into QML RichText HTML.

Highlight styling is delegated to the dedicated `ContentsTextHighlightRenderer` module so palette logic does not stay
embedded in the generic inline-tag parser.

## Public Contract
- `sourceText`
  Raw editor text payload (plain text plus inline tags such as `<bold>...</bold>`).
- `renderedHtml`
  Render-ready HTML string consumed by QML `Text`/`TextEdit` in rich-text mode.
- `renderRichText(sourceText)`
  Stateless helper to render any input text without mutating bridge ownership state.
- `normalizeEditorSurfaceTextToSource(surfaceText)`
  Converts RichText editor output back into canonical `.wsnbody` inline source tags.
- `applyInlineStyleToSelectionSource(surfaceText, selectionStart, selectionEnd, styleTag)`
  Applies the requested inline style to the current RichText selection with `QTextDocument/QTextCursor`, then
  canonicalizes the result back into `.wsnbody` source tags.
  When the full selection already carries the same style, the renderer removes formatting from that range and writes
  plain text back instead of nesting duplicate tags, so the matching inline source tags are also removed from
  `.wsnbody`.
  The same entrypoint also accepts `plain` / `clear` / `none` to strip all supported inline styling from the selected
  range explicitly.
- `requestRenderRefresh()`
  Slot entrypoint for explicit refresh requests from QML when immediate recompute is needed.

## Supported Inline Style Tags
- Bold aliases: `bold`, `b`, `strong` -> `<strong style="font-weight:900;">`
- Italic aliases: `italic`, `i`, `em` -> `<span style="font-style:italic;">`
- Underline aliases: `underline`, `u` -> `<span style="text-decoration: underline;">`
- Strike aliases: `strikethrough`, `strike`, `s`, `del` -> `<span style="text-decoration: line-through;">`
- Highlight aliases: `highlight`, `mark` -> Apple Notes-inspired styled `<span ...>`
- Clear aliases: `plain`, `clear`, `none` -> remove inline formatting from the selected range
