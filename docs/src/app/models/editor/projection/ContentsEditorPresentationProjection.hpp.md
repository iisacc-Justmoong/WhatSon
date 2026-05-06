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
- `sourceCursorPosition`/`logicalCursorPosition` properties for rendered caret projection
- coordinate mapping invokables used by the editor surface without exposing the raw offset table to QML

## Render Contract

`htmlTokens` and `normalizedHtmlBlocks` are forwarded from `ContentsTextFormatRenderer`, whose pipeline now parses RAW
`.wsnbody`, renders HTML fragments, converts them through `iiHtmlBlock`, and publishes block-object metadata for the
QML host.

The projection does not mutate source text. Source mutations stay on the editor session/persistence path.
`logicalOffsetForSourceOffset(int)` and `sourceOffsetForLogicalOffset(int)` are read-side coordinate transforms only;
they must use the current whole RAW source snapshot rather than partial source-prefix parsing.

The logical/source offset table is internal model state. QML editor hosts must bind this projection object as a
`coordinateMapper` and consume `logicalCursorPosition`; they must not receive or iterate the whole offset table.
