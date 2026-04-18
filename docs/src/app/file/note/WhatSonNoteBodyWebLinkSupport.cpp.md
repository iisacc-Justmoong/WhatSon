# `src/app/file/note/WhatSonNoteBodyWebLinkSupport.cpp`

## Responsibility
Implements shared note-body hyperlink normalization for RAW source and rendered RichText output.

## Key Behavior
- Detects `http://`, `https://`, and `www.` style literals in ordinary text segments.
- Refuses to double-wrap content that is already inside `<weblink>` tags.
- Skips markdown link destinations such as `[label](https://example.com)` so RAW markdown literals are not rewritten
  accidentally during ordinary source edits.
- Canonicalizes existing malformed/opening `weblink` tags back to one lowercase `href="..."` form.
- Normalizes activation URLs so scheme-less `www.example.com` values still open through `Qt.openUrlExternally(...)`
  after RichText projection.
- Emits one consistent anchor style (`color:#8CB4FF;text-decoration: underline;`) so page/print palette remapping can
  recolor links through the same renderer-side string replacement path as the other semantic spans.
