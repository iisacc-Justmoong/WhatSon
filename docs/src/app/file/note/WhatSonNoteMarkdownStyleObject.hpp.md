# `src/app/file/note/WhatSonNoteMarkdownStyleObject.hpp`

## Role

Declares the shared markdown-promotion helper object for note bodies.

The object keeps renderer-side markdown presentation and persistence-side `.wsnbody` promotion aligned through one
contract.

## Public Contract

- `Role` enumerates the markdown presentation roles used by the editor renderer:
    - unordered / ordered list markers
    - heading marker / body
    - blockquote marker / body
    - inline code / fenced-code marker / fenced-code body
    - link literal
    - horizontal rule
- `wrapHtmlSpan(...)` emits the exact RichText `<span style=...>` payload for a markdown role.
- `promotionMatchForCss(...)` answers whether a span CSS declaration belongs to one of those shared roles and, if so,
  which proprietary `.wsnbody` inline tags that role should intentionally promote into.
- `canonicalUnorderedListSourceMarker()` defines the canonical write-side unordered-list marker. The current canonical
  marker is `-`.

## Important Invariants

- Marker-only roles must still count as matched roles so they suppress accidental generic CSS promotion.
- Renderer-emitted markdown role CSS and persistence-side role detection must stay in sync through this object.
