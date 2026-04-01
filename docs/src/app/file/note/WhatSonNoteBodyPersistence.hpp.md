# `src/app/file/note/WhatSonNoteBodyPersistence.hpp`

## Role
This header defines the shared plain-text persistence contract for note bodies.

It is the boundary between the editor-facing text model and the filesystem-facing `.wsnbody` XML document format.

## Public Contract
- `normalizeBodyPlainText(...)` only normalizes line endings. It must not trim lines or collapse whitespace.
- `serializeBodyDocument(...)` is now the canonical `.wsnbody` writer entry. It accepts editor source text
  (plain text, inline `.wsnbody` tags, or Qt Rich HTML) and emits normalized `<paragraph>` XML with
  canonical inline tags (`bold`, `italic`, `underline`, `strikethrough`, `highlight`) plus normalized
  self-closed `<resource ... />` tags.
- `plainTextFromBodyDocument(...)` extracts editor text from a `.wsnbody` XML payload while preserving empty paragraphs and whitespace-only paragraphs.
- `richTextFromBodyDocument(...)` extracts a rich-text projection from `.wsnbody` and maps inline style tags
  (`bold`, `italic`, `underline`, `strikethrough`, `highlight`, `mark`) to HTML (`strong`, `em`, `u`, `s`, styled `span`).
- `firstLineFromBodyDocument(...)` derives preview text from the first logical XML line, including leading inline text that appears before the first paragraph block.
- `firstLineFromBodyPlainText(...)` derives preview text from the first non-empty trimmed line, without mutating the stored plain text.
- `persistBodyPlainText(...)` is the high-level save entry used by hierarchy viewmodels. It now returns both:
  - normalized plain text (search/preview/index role)
  - normalized editor source text (renderer/editor role)

## Important Invariants
- Empty `<paragraph></paragraph>` nodes must survive a read/save round-trip when the editor text is unchanged.
- Whitespace-only paragraphs must remain representable in the plain-text editor model.
- The persistence layer must not perform unsolicited body-tag cleanup beyond the serializer that is explicitly chosen for a real body rewrite.
- Rich-text scaffolding from Qt (`<!DOCTYPE HTML ...><html>...`) must never leak into logical note content or
  first-line indexing.
