# `src/app/models/file/note/WhatSonNoteMarkdownStyleObject.cpp`

## Role

Implements the shared markdown presentation/promotion object for note bodies.

## Key Behaviors

- Normalizes CSS declarations into property/value maps so ordering and whitespace differences do not break role
  detection.
- Defines the exact RichText CSS used for markdown presentation roles:
    - list markers
    - heading marker/body
    - blockquote marker/body
    - inline code / fenced-code marker / fenced-code body
    - link literal
    - horizontal rule
- Returns intentional promotion mappings instead of relying on accidental generic CSS inference:
    - heading body -> `<bold>`
    - blockquote body -> `<italic>`
    - link literal -> `<underline>`
    - inline code / fenced-code body -> `<highlight>`
    - markers and horizontal rules -> no promotion
- Exposes the canonical unordered-list source marker (`-`) used when display-surface bullets are normalized back into
  `.wsnbody` source text.

## Regression Checks

- Markdown marker spans must match this object and suppress generic bold/italic/underline/highlight promotion.
- Heading/body, blockquote/body, link, and code role CSS must map to the intended proprietary inline tag set.
- Any unordered-list display glyph round-trip must collapse back to the canonical source marker `-`.
