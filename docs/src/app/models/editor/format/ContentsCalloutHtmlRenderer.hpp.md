# `src/app/models/editor/format/ContentsCalloutHtmlRenderer.hpp`

## Responsibility

Declares the narrow callout helper shared by editor-format renderers.

## Public Contract

- `containsCalloutTag(...)`
  Detects RAW callout markup without converting the caller into a persistence writer.
- `singleCalloutSpan(...)`
  Returns the inner RAW span only when the source is exactly one non-nested callout wrapper.
- `renderCalloutBlockHtml(...)`
  Builds the Qt RichText-compatible callout frame. The helper owns the table and leading-bar markup so all renderer
  entry points keep the same visual contract.
