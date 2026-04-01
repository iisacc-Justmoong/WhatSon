# `src/app/file/note/WhatSonNoteBodyPersistence.cpp`

## Role
This implementation owns the reusable note-body normalization and save workflow.

The file now also contains the shared XML-to-plain-text extraction path used by both the local note file store and the library runtime indexer.

## Key Behaviors
- `normalizeBodyPlainText(...)` normalizes only `CRLF` / `CR` into `LF`.
- `serializeBodyDocument(...)` is the single write-side serializer. It normalizes three editor-source shapes into `.wsnbody`:
  - plain text with newlines
  - inline `.wsnbody` style/resource tags
  - Qt Rich HTML fragments/documents
- `plainTextFromBodyDocument(...)` parses the `.wsnbody` XML with `QXmlStreamReader` and treats paragraph-like block elements as explicit text lines.
- `richTextFromBodyDocument(...)` uses the same parser pipeline and emits HTML-ready lines (`<br/>` joins), mapping inline style aliases to canonical tags:
  - `bold` / `b` / `strong` -> `strong`
  - `italic` / `i` / `em` -> `em`
  - `underline` / `u` -> `u`
  - `strikethrough` / `strike` / `s` / `del` -> `s`
  - `highlight` / `mark` -> styled `span` (`background-color:#8A4B00; color:#FFD9A3; font-weight:600`)
- Before XML parsing, resource tags are normalized into strict empty-element form (`<resource ... />`), so the body parser still works when notes contain shorthand resource tags such as `<resource ...>` or unquoted attribute values.
- Rich HTML `<span style=...>` runs are reduced into canonical inline tags before writing. This keeps storage format stable while still accepting LV text editor RichText output.
- `firstLineFromBodyDocument(...)` preserves leading inline title text even when the visible plain-text summary is driven by later paragraph blocks.
- Empty paragraphs are emitted as empty lines instead of being dropped.
- Whitespace-only paragraphs are emitted as whitespace-only lines instead of being trimmed away.
- `persistBodyPlainText(...)` now canonicalizes incoming editor text through `serializeBodyDocument(...)` before no-op comparison, then returns:
  - plain text for indexing/search/list summaries
  - rich editor source text for editor binding

## Why This Matters
Before this change, RichText scaffolding could leak into the logical note body (`<!DOCTYPE HTML ...>` becoming first-line text). Canonicalizing through the `.wsnbody` serializer keeps parser/index behavior stable and preserves formatted editing semantics.
