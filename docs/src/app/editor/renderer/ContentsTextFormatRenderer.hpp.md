# `src/app/editor/renderer/ContentsTextFormatRenderer.hpp`

## Responsibility
Declares the editor-side rich-text renderer bridge that converts `.wsnbody` inline style tags into QML RichText HTML.

## Public Contract
- `sourceText`
  Raw editor text payload (plain text plus inline tags such as `<bold>...</bold>`).
- `renderedHtml`
  Render-ready HTML string consumed by QML `Text`/`TextEdit` in rich-text mode.
- `renderRichText(sourceText)`
  Stateless helper to render any input text without mutating bridge ownership state.
- `requestRenderRefresh()`
  Slot entrypoint for explicit refresh requests from QML when immediate recompute is needed.

## Supported Inline Style Tags
- Bold aliases: `bold`, `b`, `strong` -> `<strong>`
- Italic aliases: `italic`, `i`, `em` -> `<em>`
- Underline aliases: `underline`, `u` -> `<u>`
- Strike aliases: `strikethrough`, `strike`, `s`, `del` -> `<s>`
