# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.cpp`

## Responsibility

Implements RichText-side inline resource presentation helpers.

## Current Behavior

- Resolves image resource frame dimensions from the editor text-column width.
- Replaces rendered resource placeholders with framed image HTML.
- Emits image-resource frame paragraphs with zero line height and top-aligned images, so the RichText paragraph does
  not add text baseline/descent space beyond the generated frame image height.
- Normalizes renderer payloads through C++ dynamic-object support before producing presentation HTML.
