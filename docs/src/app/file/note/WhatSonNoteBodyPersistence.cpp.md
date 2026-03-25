# `src/app/file/note/WhatSonNoteBodyPersistence.cpp`

## Role
This implementation owns the reusable note-body normalization and save workflow.

The file now also contains the shared XML-to-plain-text extraction path used by both the local note file store and the library runtime indexer.

## Key Behaviors
- `normalizeBodyPlainText(...)` normalizes only `CRLF` / `CR` into `LF`.
- `plainTextFromBodyDocument(...)` parses the `.wsnbody` XML with `QXmlStreamReader` and treats paragraph-like block elements as explicit text lines.
- `firstLineFromBodyDocument(...)` preserves leading inline title text even when the visible plain-text summary is driven by later paragraph blocks.
- Empty paragraphs are emitted as empty lines instead of being dropped.
- Whitespace-only paragraphs are emitted as whitespace-only lines instead of being trimmed away.
- `persistBodyPlainText(...)` short-circuits when the editor text matches the parsed plain text from disk, which prevents accidental body rewrites and tag cleanup during no-op saves.

## Why This Matters
Before this change, the read path collapsed whitespace and discarded empty lines. That made the editor believe the body had changed even when the user had not asked for any structural cleanup, and a later save could rewrite `.wsnbody` without preserving intentional blank paragraphs.
