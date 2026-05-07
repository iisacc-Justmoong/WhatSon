# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.cpp`

## Responsibility

Implements inline resource frame normalization for structured editor visual blocks.

## Current Behavior

- Resolves image resource frame dimensions from the editor text-column width.
- Emits `inlineResourceVisualBlocks(...)` records containing frame width, height, image source, label, target, and
  resource index for direct QML delegates.
- Builds `editorSurfaceHtmlWithResourceVisualBlocks(...)` from renderer `htmlTokens` and structured resource visual
  blocks. Resource slots become transparent block-flow paragraphs with the generated frame height, so following prose
  is placed below the same visual frame bottom used by gutter geometry.
- Keeps legacy HTML placeholder replacement as compatibility behavior, but the live editor display path no longer uses
  it to build image-frame markup for the editing surface.
- Normalizes renderer payloads through C++ dynamic-object support before producing frame images or visual block
  records.
