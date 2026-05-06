# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.cpp`

## Responsibility

Implements RichText-side inline resource presentation helpers.

## Current Behavior

- Resolves image resource frame dimensions from the editor text-column width.
- Replaces rendered resource placeholders with framed image HTML.
- Normalizes renderer payloads through C++ dynamic-object support before producing presentation HTML.
