# `src/app/models/editor/projection/ContentsEditorPresentationProjection.hpp`

## Responsibility

Declares the QML-facing editor presentation projection object.

It exposes the read-side projection derived from authoritative RAW `.wsnbody` text:

- `editorSurfaceHtml`
- `renderedHtml`
- `htmlTokens`
- `normalizedHtmlBlocks`
- logical text and logical-line metrics
- whole-source logical/source cursor mapping

## Render Contract

`htmlTokens` and `normalizedHtmlBlocks` are forwarded from `ContentsTextFormatRenderer`, whose pipeline now parses RAW
`.wsnbody`, renders HTML fragments, converts them through `iiHtmlBlock`, and publishes block-object metadata for the
QML host.

The projection does not mutate source text. Source mutations stay on the editor session/persistence path.
`logicalOffsetForSourceOffset(int)` and `sourceOffsetForLogicalOffset(int)` are read-side coordinate transforms only;
they must use the current whole RAW source snapshot rather than partial source-prefix parsing.
