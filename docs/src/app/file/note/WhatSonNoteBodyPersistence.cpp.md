# `src/app/file/note/WhatSonNoteBodyPersistence.cpp`

## Role
This implementation owns the reusable note-body normalization and save workflow.

The file now also contains the shared XML-to-plain-text extraction path used by both the local note file store and the library runtime indexer.
The current contract preserves editor-authored RAW source across save/load turns instead of re-linting that source on every persistence bounce.

## Key Behaviors
- `normalizeBodyPlainText(...)` normalizes `CRLF` / `CR` into `LF`, converts Qt line/paragraph separator characters
  into plain `LF`, and converts non-breaking spaces back into ordinary spaces before `.wsnbody` canonicalization.
- `serializeBodyDocument(...)` is the single write-side serializer. It materializes three editor-source shapes into `.wsnbody`:
  - plain text with newlines
  - inline `.wsnbody` style/resource tags
  - Qt Rich HTML fragments/documents
- The serializer still reduces those inputs into XML-safe body markup, but it no longer runs a separate structured-tag
  linter pass that rewrites RAW source simply because a persistence turn happened.
- When inline proprietary styles remain open across a source newline, the serializer now carries those open style tags
  forward and reopens them at the start of the next `<paragraph>`. This keeps multi-paragraph formatting alive in a
  paragraph-based XML document instead of silently truncating it at the first line boundary.
- During that write-side normalization, visible inline hashtags such as `#label` are promoted into canonical body tags
  as `<tag>label</tag>`.
- The same write-side normalization now also auto-wraps detectable web URLs into canonical RAW/body tags:
  - literal `www.iisacc.com` or `https://example.com` text can serialize as
    `<weblink href="...">...</weblink>`
  - markdown link destinations are intentionally skipped so `[label](url)` literals do not get rewritten by accident
- Semantic passthrough tags that the live note source must keep verbatim now round-trip through the same serializer
  instead of being escaped into literal text:
  - `next`
  - `event`
  - `title`
  - `subTitle` / `subtitle`
  - `eventTitle`
  - `eventDescription`
- Standalone `event` wrapper lines now stay direct `<body>` children during serialization, so nested legacy semantic
  blocks do not reopen as stray blank paragraphs on the next read.
- `plainTextFromBodyDocument(...)` parses the `.wsnbody` XML with `QXmlStreamReader` and treats paragraph-like block elements as explicit text lines.
- `sourceTextFromBodyDocument(...)` is the canonical read-side source extractor. It converts `.wsnbody` back into
  editor-facing inline tags such as `<bold>...</bold>` and `<resource ... />`, instead of returning RichText spans.
  Stored `<tag>label</tag>` elements are intentionally projected back to visible editor text as `#label` so the
  user keeps editing the hashtag they originally typed instead of raw XML.
- Stored `<weblink href="...">label</weblink>` nodes now also survive that read-side projection verbatim instead of
  being escaped into literal `<weblink>` text.
- That read-side source projection now stays on the parser/serializer projection only. It no longer runs a second
  structured-tag linter normalization pass, so note-open or note-switch cannot silently rewrite the editor RAW shape
  just because the note body was reparsed.
- If that canonical source projection unexpectedly collapses to an empty string for a non-empty body, the read path now
  falls back to the parsed plain-text projection from the same `.wsnbody` payload instead of returning a blank editor
  buffer.
  This keeps note reopen/load behavior aligned with the saved RAW body text even when the source extractor cannot fully
  recover a malformed or partially canonicalized document.
- The canonical single divider token `</break>` is now preserved end-to-end:
  - editor/source projection keeps `</break>`
  - `.wsnbody` body XML stores `<break/>` (valid XML)
  - legacy `<hr ...>` input aliases are normalized to `</break>` on read/write canonicalization
- Proprietary agenda/task tags are now canonicalized and preserved end-to-end instead of being escaped as literal text:
  - `<agenda ...>` start tags are normalized to mandatory `date="YYYY-MM-DD"`
  - `<task ...>` start tags are normalized to canonical `done="true|false"`
  - self-closing agenda/task aliases are expanded into explicit open/close pairs
  - read-side source projection keeps canonical `<agenda>` / `<task>` tags
- Proprietary callout tags are now canonicalized and preserved end-to-end:
  - `<callout ...>` start aliases are normalized to canonical `<callout>`
  - self-closing callout aliases are expanded into explicit `<callout></callout>`
  - read-side source projection keeps canonical `<callout>...</callout>` wrappers
- Agenda/callout/resource/divider source blocks are now normalized onto standalone editor lines before save/load
  projection. Adjacent text is split away from the proprietary block so block cards do not remain embedded in ordinary
  paragraph text on round-trip.
- The same standalone normalization now also repairs resource lines that were accidentally persisted as visible
  paragraph text such as `&lt;resource ... /&gt;` or a truncated `&lt;resource ... /`.
  When one paragraph line decodes to a standalone resource tag, read/write normalization upgrades it back to the
  canonical `<resource ... />` body block instead of preserving it as escaped prose.
- `serializeBodyDocument(...)` now writes standalone agenda/callout/resource/divider lines directly under `<body>`
  instead of wrapping them back into `<paragraph>...</paragraph>`.
- The same standalone normalization now applies to `<resource ... />` tags during read-back too, so a saved resource
  body slot cannot collapse back onto the previous paragraph line and disappear from the canonical editor source on the
  next note-open.
- `plainTextFromBodyDocument(...)` now treats direct `<agenda>` / `<callout>` body children as block content too, so
  logical text reconstruction still preserves block line boundaries even after those tags stop being paragraph-wrapped.
  Agenda task bodies are projected as newline-separated block lines inside that agenda block.
- `extractedInlineTagValues(...)` canonicalizes incoming editor text and extracts deduplicated body-tag payloads for
  `.wsnhead` and `Tags.wstags` synchronization.
- The parser now ignores whitespace-only top-level character nodes inside `<body>`, so pretty-printed empty bodies
  (`<body>\n  </body>`) no longer rehydrate as a leading blank line in the editor.
- `richTextFromBodyDocument(...)` uses the same parser pipeline and emits HTML-ready lines (`<br/>` joins), mapping inline style aliases to explicit span styling:
  - `bold` / `b` / `strong` -> `<strong style="font-weight:900;">`
  - `italic` / `i` / `em` -> `<span style="font-style:italic;">`
  - `underline` / `u` -> `<span style="text-decoration: underline;">`
  - `strikethrough` / `strike` / `s` / `del` -> `<span style="text-decoration: line-through;">`
  - `highlight` / `mark` -> styled `span` (`background-color:#8A4B00; color:#D6AE58; font-weight:600`)
  - `weblink` -> `<a href="...">` with the shared editor/preview link styling and scheme normalization for `www.*`
  - divider block tags (`<break/>` and legacy `<hr/>`) -> `<hr/>`
- The same rich-text projection now preserves visible horizontal whitespace inside text nodes:
  - leading paragraph indentation is emitted as `&nbsp;`, so a Tab-authored space run stays visible after the
    `.wsnbody` parse/read path regenerates the editor RichText
  - repeated interior spaces alternate plain spaces and `&nbsp;`, keeping authored spacing without turning every gap
    into a non-breaking run
  - tabs are projected as a four-space `&nbsp;` run, matching the editor-side tab-indent contract
- The same read-side parser now also resolves legacy semantic body tags through the shared semantic-tag registry:
  - `<next/>` behaves like a line break in plain/rich projections
  - `<title>`, `<subTitle>`, and `<eventTitle>` render as heading-style text instead of literal XML
  - `<event>` is treated as a transparent wrapper while its child semantic blocks still materialize into content lines
- Before XML parsing, resource tags are normalized into strict empty-element form (`<resource ... />`), so the body parser still works when notes contain shorthand resource tags such as `<resource ...>` or unquoted attribute values.
- The resource-preserving fallback source extractor now reuses the same shared anonymous-namespace whitespace and
  standalone-block normalization helpers as the canonical parser path, so those normalization rules stay defined in
  one place inside this translation unit.
- Rich HTML `<span style=...>` runs are reduced into canonical inline tags before writing. This keeps storage format stable while still accepting LV text editor RichText output.
- Markdown-presentation spans are matched against `WhatSonNoteMarkdownStyleObject` before the generic CSS heuristics
  run. This makes heading/blockquote/link/code promotion intentional and lets marker-only spans suppress generic style
  promotion.
- Formatting-only whitespace between tags (`>\n    <`) is stripped before Rich HTML / `.wsnbody` source is reduced into
  inline-tag text, so pretty-printed HTML/XML indentation cannot leak into the note body as real content lines.
- `firstLineFromBodyDocument(...)` preserves leading inline title text even when the visible plain-text summary is driven by later paragraph blocks.
- Empty paragraphs are emitted as empty lines instead of being dropped, including leading/trailing empty paragraphs the user intentionally created.
- Whitespace-only paragraphs are preserved exactly as stored; the persistence layer no longer trims outer whitespace-only lines during read/normalization.
- Stored `<tag>` nodes contribute a literal leading `#` in plain-text and rich-text projections, so previews and
  first-line extraction stay aligned with the editor-visible source.
- This whitespace filter applies only to top-level formatting whitespace around body markup. It must not strip
  whitespace that belongs to actual paragraph/block content.
- `persistBodyPlainText(...)` now canonicalizes incoming editor text through `serializeBodyDocument(...)` before no-op comparison, then returns:
  - plain text for indexing/search/list summaries
  - newline-normalized editor RAW source text for editor binding (`<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, `<highlight>`)
- The no-op comparison and returned `bodySourceText` now treat the editor-provided RAW text as authoritative instead of
  round-tripping it back through `sourceTextFromBodyDocument(...)` first.
- When `persistBodyPlainText(...)` does perform a filesystem write, it now opts out of `modifiedCount` increments.
  The editor autosave path still updates the persisted body/header material and `lastModifiedAt`, but it no longer
  treats each autosaved typing burst as a separate modification-count event.
- Unordered-list display glyph recovery intentionally normalizes back to the canonical source marker `-` instead of
  preserving `*` / `+` source variants.

## Why This Matters
Before this change, RichText scaffolding could leak into the logical note body (`<!DOCTYPE HTML ...>` becoming first-line text). Canonicalizing through the `.wsnbody` serializer keeps parser/index behavior stable and preserves formatted editing semantics.
The same rule now prevents empty-note body indentation from surfacing as a phantom first blank line during note
creation.
The hashtag projection rule also keeps note text human-readable after save: storage uses `<tag>`, but the editor and
text projections still show `#label`.
The same persistence boundary now also prevents note-open, note-switch, and other non-editor flows from quietly
rewriting `bodySourceText` RAW just because the body document was read and reparsed.

## Regression Notes
- Empty-note `<body>` whitespace handling is now a documented behavior contract only; this repository no longer
  maintains a dedicated scripted test for it.
- Editor autosave must not inflate `fileStat.modifiedCount` while the user types.
- Body hashtags must survive a full save/load round-trip as visible `#label` text while still persisting as canonical
  `<tag>` nodes inside `.wsnbody`.
- A paragraph that starts with Tab-inserted indentation spaces must still display that indentation after a
  save/load or parse/re-render round-trip; the projection must not collapse the stored spaces back into one HTML gap.
- A style applied across multiple logical paragraphs must still render on every touched paragraph after save/load, even
  though the serializer has to split that logical span into paragraph-local reopened canonical tags.
- A typed `</break>` token must survive save/load as `</break>` in editor source while `.wsnbody` persists it as
  `<break/>`, and rich-text projection must show a divider line instead of literal tag text.
- A typed or pasted web URL that canonicalizes into `<weblink href="...">label</weblink>` must survive save/load and
  rich-text projection as one active hyperlink instead of escaping back into literal XML.
- A typed agenda/task source block must survive save/load without escaping:
  - input: `<agenda date="..."><task done="false">todo</task></agenda>`
  - output source projection: canonical agenda/task tags with normalized attributes
- A typed `<callout>message</callout>` block must survive save/load without escaping wrapper tags.
- A standalone `<agenda>...</agenda>`, `<callout>...</callout>`, `<resource ... />`, or `</break>` source line must
  round-trip as a direct `<body>` child instead of being rewrapped into `<paragraph>`.
- A paragraph line that already contains only an escaped resource tag from an earlier bad save must recover to a direct
  `<resource ... />` body child on the next read/write turn instead of staying escaped forever.
- A saved legacy semantic body block such as `<title>`, `<subTitle>`, `<eventTitle>`, `<eventDescription>`, or
  `<next/>` must not degrade into escaped literal text on the next autosave.
- Legacy notes that already embedded agenda/callout blocks inside paragraph content must rehydrate into standalone
  editor lines on load so renderer-owned cards can be rebuilt immediately from the RAW tags.
- Legacy/self-closing/non-canonical structured tags may still be normalized by the serializer/parser projection that
  the editor explicitly invoked, but passive load/save turns must not introduce an extra RAW rewrite layer on top of
  that projection.
- Reopening a note whose saved `.wsnbody` still contains visible paragraph text must not yield an empty editor body
  solely because canonical inline-tag source extraction returned `""`.
