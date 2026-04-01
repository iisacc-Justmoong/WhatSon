# `src/app/editor/renderer/ContentsTextHighlightRenderer.cpp`

## Responsibility
Implements the isolated highlight style palette and alias resolution.

## Key Behavior
- Normalizes tag aliases to support both `<highlight>` and `<mark>`.
- Emits a fixed RichText span style:
  - background: `#8A4B00`
  - text color: `#FFD9A3`
  - font weight: `600`
- Keeps highlight-stack semantics separate from the generic inline-tag mapper used by `ContentsTextFormatRenderer`.
