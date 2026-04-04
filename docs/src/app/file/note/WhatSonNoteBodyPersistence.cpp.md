# `src/app/file/note/WhatSonNoteBodyPersistence.cpp`

## Role
This implementation owns the reusable note-body normalization and save workflow.

The file now also contains the shared XML-to-plain-text extraction path used by both the local note file store and the library runtime indexer.

## Key Behaviors
- `normalizeBodyPlainText(...)` normalizes `CRLF` / `CR` into `LF`, converts Qt line/paragraph separator characters
  into plain `LF`, and converts non-breaking spaces back into ordinary spaces before `.wsnbody` canonicalization.
- `serializeBodyDocument(...)` is the single write-side serializer. It normalizes three editor-source shapes into `.wsnbody`:
  - plain text with newlines
  - inline `.wsnbody` style/resource tags
  - Qt Rich HTML fragments/documents
- `plainTextFromBodyDocument(...)` parses the `.wsnbody` XML with `QXmlStreamReader` and treats paragraph-like block elements as explicit text lines.
- `sourceTextFromBodyDocument(...)` is the canonical read-side source extractor. It converts `.wsnbody` back into
  editor-facing inline tags such as `<bold>...</bold>` and `<resource ... />`, instead of returning RichText spans.
- `richTextFromBodyDocument(...)` uses the same parser pipeline and emits HTML-ready lines (`<br/>` joins), mapping inline style aliases to explicit span styling:
  - `bold` / `b` / `strong` -> `<strong style="font-weight:900;">`
  - `italic` / `i` / `em` -> `<span style="font-style:italic;">`
  - `underline` / `u` -> `<span style="text-decoration: underline;">`
  - `strikethrough` / `strike` / `s` / `del` -> `<span style="text-decoration: line-through;">`
  - `highlight` / `mark` -> styled `span` (`background-color:#8A4B00; color:#D6AE58; font-weight:600`)
- Before XML parsing, resource tags are normalized into strict empty-element form (`<resource ... />`), so the body parser still works when notes contain shorthand resource tags such as `<resource ...>` or unquoted attribute values.
- Rich HTML `<span style=...>` runs are reduced into canonical inline tags before writing. This keeps storage format stable while still accepting LV text editor RichText output.
- Markdown-presentation spans are matched against `WhatSonNoteMarkdownStyleObject` before the generic CSS heuristics
  run. This makes heading/blockquote/link/code promotion intentional and lets marker-only spans suppress generic style
  promotion.
- Formatting-only whitespace between tags (`>\n    <`) is stripped before Rich HTML / `.wsnbody` source is reduced into
  inline-tag text, so pretty-printed HTML/XML indentation cannot leak into the note body as real content lines.
- `firstLineFromBodyDocument(...)` preserves leading inline title text even when the visible plain-text summary is driven by later paragraph blocks.
- Empty paragraphs are emitted as empty lines instead of being dropped, including leading/trailing empty paragraphs the user intentionally created.
- Whitespace-only paragraphs are preserved exactly as stored; the persistence layer no longer trims outer whitespace-only lines during read/normalization.
- `persistBodyPlainText(...)` now canonicalizes incoming editor text through `serializeBodyDocument(...)` before no-op comparison, then returns:
  - plain text for indexing/search/list summaries
  - inline-tag editor source text for editor binding (`<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, `<highlight>`)
- Unordered-list display glyph recovery intentionally normalizes back to the canonical source marker `-` instead of
  preserving `*` / `+` source variants.

## Why This Matters
Before this change, RichText scaffolding could leak into the logical note body (`<!DOCTYPE HTML ...>` becoming first-line text). Canonicalizing through the `.wsnbody` serializer keeps parser/index behavior stable and preserves formatted editing semantics.
