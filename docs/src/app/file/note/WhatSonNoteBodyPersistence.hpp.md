# `src/app/file/note/WhatSonNoteBodyPersistence.hpp`

## Role
This header defines the shared plain-text persistence contract for note bodies.

It is the boundary between the editor-facing text model and the filesystem-facing `.wsnbody` XML document format.

## Public Contract
- `normalizeBodyPlainText(...)` only normalizes line endings. It must not trim lines or collapse whitespace.
- `serializeBodyDocument(...)` is now the canonical `.wsnbody` writer entry. It accepts editor source text
  (plain text, inline `.wsnbody` tags, or Qt Rich HTML) and emits normalized `<paragraph>` XML with
  canonical inline tags (`bold`, `italic`, `underline`, `strikethrough`, `highlight`, `tag`) plus normalized
  self-closed `<resource ... />` tags. The single divider source tag `</break>` is persisted as canonical XML
  `<break/>`.
  Proprietary checklist tags are also canonicalized:
  - `<agenda>` always persists with mandatory `date="YYYY-MM-DD"`
  - `<task>` always persists with canonical `done="true|false"`
  - `<callout>` persists as canonical wrapper tags (`<callout>...</callout>`) without escaping
- When a proprietary inline style stays open across a source newline, the serializer now reopens that style at the
  beginning of the next `<paragraph>` and closes it again at the end of that paragraph, so multi-paragraph formatting
  survives a full save round-trip instead of stopping at the first paragraph boundary.
- Editor-visible inline hashtags such as `#label` are promoted into canonical body storage as
  `<tag>label</tag>` during serialization.
- `extractedInlineTagValues(...)` collects deduplicated inline-tag values from editor source, whether the caller
  passes visible `#label` source text or already-canonical `<tag>label</tag>` text.
- Markdown-presentation RichText spans are first matched through `WhatSonNoteMarkdownStyleObject`, which decides whether
  they intentionally promote into proprietary inline tags or should merely suppress generic CSS promotion.
- `plainTextFromBodyDocument(...)` extracts editor text from a `.wsnbody` XML payload while preserving empty paragraphs and whitespace-only paragraphs, including leading/trailing empty paragraphs the user intentionally created. Stored `<tag>` nodes are projected back to visible `#label` text in that plain-text projection.
- `sourceTextFromBodyDocument(...)` extracts the canonical inline-tag source projection used by the editor/session
  layer. Style/resource tags stay in proprietary inline-tag form, while stored `<tag>label</tag>` nodes project back
  to editor-visible `#label`. Stored `<break/>` (or legacy `<hr/>`) projects back to canonical editor source
  `</break>`. This projection now avoids an extra structured-tag linter rewrite pass so passive file reads do not
  mutate the editor RAW shape.
- `richTextFromBodyDocument(...)` extracts a rich-text projection from `.wsnbody` and maps inline style tags
  (`bold`, `italic`, `underline`, `strikethrough`, `highlight`, `mark`) to RichText HTML (styled `span` tags).
  Divider tags (`<break/>` / `<hr/>`) render as `<hr/>`.
- `firstLineFromBodyDocument(...)` derives preview text from the first logical XML line, including leading inline text that appears before the first paragraph block.
- `firstLineFromBodyPlainText(...)` derives preview text from the first non-empty trimmed line, without mutating the stored plain text.
- `persistBodyPlainText(...)` is the high-level save entry used by hierarchy viewmodels. It now returns both:
  - normalized plain text (search/preview/index role)
  - normalized editor RAW source text (editor/source role)

## Important Invariants
- Empty `<paragraph></paragraph>` nodes must survive a read/save round-trip when the editor text is unchanged, regardless of whether they appear at the beginning, middle, or end of the note body.
- Whitespace-only paragraphs must remain representable in the plain-text editor model.
- The persistence layer must not perform unsolicited body-tag cleanup or whitespace-line trimming beyond the serializer
  that is explicitly chosen for a real editor-driven body rewrite.
- Rich-text scaffolding from Qt (`<!DOCTYPE HTML ...><html>...`) must never leak into logical note content or
  first-line indexing.
- Unordered-list display glyph recovery is intentionally canonicalized to the single source marker `-`.
- Inline hashtag storage must round-trip as:
  - editor/source text: `#label`
  - `.wsnbody` canonical storage: `<tag>label</tag>`
  - plain-text/rich-text read projection: visible `#label`
- Proprietary inline formatting that spans multiple editor paragraphs must still round-trip as visible formatting on
  every touched paragraph even though `.wsnbody` remains paragraph-based XML.
- Divider round-trip must stay canonical:
  - editor/source token: `</break>`
  - `.wsnbody` XML storage: `<break/>`
  - rich preview projection: `<hr/>`
- Agenda/task round-trip must stay canonical:
  - editor/source tokens remain `<agenda ...>` / `<task ...>`
  - serializer must not escape them into literal text
  - `agenda.date` stays mandatory `YYYY-MM-DD`, `task.done` stays `true|false`
- Callout round-trip must stay canonical:
  - editor/source tokens remain `<callout>...</callout>`
  - serializer must not escape wrapper tags into literal text
