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
- Non-resource token flow is taken from the renderer-owned `ownsBlockFlow` flag. This controller does not reclassify
  HTML fragments by tag-name prefix, keeping block-flow ownership in the renderer contract.
- The transparent spacer line-height uses one named Qt RichText compensation helper, so the TextEdit line-box offset is
  isolated in the resource presentation layer instead of appearing as anonymous arithmetic in the view.
- Keeps legacy HTML placeholder replacement as compatibility behavior, but the live editor display path no longer uses
  it to build image-frame markup for the editing surface.
- Normalizes renderer payloads through C++ dynamic-object support before producing frame images or visual block
  records.
