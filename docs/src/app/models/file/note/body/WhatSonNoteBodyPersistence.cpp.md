# `src/app/models/file/note/body/WhatSonNoteBodyPersistence.cpp`

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
  - `callout`
  - `task`
- Standalone `event` wrapper lines now stay direct `<body>` children during serialization, so nested legacy semantic
  blocks do not reopen as stray blank paragraphs on the next read.
- `plainTextFromBodyDocument(...)` parses the `.wsnbody` XML through the iiXml document support and treats
  paragraph-like block elements as explicit text lines.
- Its plain-text projection hides recognized inline source tags such as `<bold>`, `<italic>`, `<weblink>`, and semantic
  text wrappers while preserving their visible text. Stored `<tag>label</tag>` nodes project back to `#label`.
- `sourceTextFromBodyDocument(...)` is the canonical read-side source extractor. It converts `.wsnbody` back into
  editor-facing inline tags such as `<bold>...</bold>` and `<resource ... />`, instead of returning RichText spans.
  Stored `<tag>label</tag>` elements are intentionally projected back to visible editor text as `#label` so the
  user keeps editing the hashtag they originally typed instead of raw XML.
- The same read-side source extractor now strips rendered-editor HTML comment scaffolding such as
  `<!--whatson-resource-block:...-->` before it projects note source back into editor RAW text.
  This prevents renderer-owned HTML block markers from leaking into `bodySourceText` and then re-entering the editor
  through note-open or runtime snapshot paths.
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
- If a read-side source projection still looks like non-canonical rendered HTML after that comment stripping
  (`<p>`, `<div>`, `<hr>`, and similar block wrappers), the extractor now treats that payload as suspicious and falls
  back to the parsed plain-text projection instead of letting HTML block wrappers reach the editor RAW pipeline.
- The canonical single divider token `</break>` is now preserved end-to-end:
  - editor/source projection keeps `</break>`
  - `.wsnbody` body XML stores `<break/>` (valid XML)
  - legacy `<hr ...>` input aliases are normalized to `</break>` on read/write canonicalization
  - editor HTML renders the token through `component/Break` as a logical blank line, not literal tag text
  body-format blocks. The editor HTML projection renders paired `<callout>...</callout>` fragments through
- Resource/divider source blocks are normalized onto standalone editor lines before save/load projection. Adjacent text
  is split away from those proprietary body blocks so atomic slots do not remain embedded in ordinary paragraph text on
  round-trip.
- The same standalone normalization now also repairs resource lines that were accidentally persisted as visible
  paragraph text such as `&lt;resource ... /&gt;` or a truncated `&lt;resource ... /`.
  When one paragraph line decodes to a standalone resource tag, read/write normalization upgrades it back to the
  canonical `<resource ... />` body block instead of preserving it as escaped prose.
- `serializeBodyDocument(...)` now writes standalone resource/divider lines directly under `<body>`
  instead of wrapping them back into `<paragraph>...</paragraph>`.
- The same standalone normalization now applies to `<resource ... />` tags during read-back too, so a saved resource
  body slot cannot collapse back onto the previous paragraph line and disappear from the canonical editor source on the
  next note-open.
- `extractedInlineTagValues(...)` canonicalizes incoming editor text and extracts deduplicated body-tag payloads for
  `.wsnhead` and `Tags.wstags` synchronization.
- The parser now ignores whitespace-only top-level character nodes inside `<body>`, so pretty-printed empty bodies
  (`<body>\n  </body>`) no longer rehydrate as a leading blank line in the editor.
- `htmlProjectionFromBodyDocument(...)` projects canonical editor RAW source lines to HTML-ready lines (`<br/>`
  joins), mapping inline style aliases to explicit span styling instead of escaping the RAW tags as literal text:
  - `bold` / `b` / `strong` -> `<strong style="font-weight:900;">`
  - `italic` / `i` / `em` -> `<span style="font-style:italic;">`
  - `underline` / `u` -> `<span style="text-decoration: underline;">`
  - `strikethrough` / `strike` / `s` / `del` -> `<span style="text-decoration: line-through;">`
  - `highlight` / `mark` -> styled `span` (`background-color:#8A4B00; color:#D6AE58; font-weight:600`)
  - `weblink` -> `<a href="...">` with the shared editor/preview link styling and scheme normalization for `www.*`
  - `style` -> marker-backed `<span>` styling from `.wsnbody` attributes:
    `style` names an LVRS text token (`Title`, `Title2`, `Header`, `Header2`, `Body`, `Description`, `Caption`,
    `Disabled`) and supplies that token's size, weight, and line height without adding per-style colors; `font`,
    `weight`, `size`, `color`, `background`, `align`, and `height` remain optional overrides for the projected renderer
    CSS. The token metrics, marker projection, and style-span matching policy are delegated to
    `models/editor/component/style`.
  - `callout` -> `component/Callout` HTML block with the Figma `280:7897` full-width wrapping surface
  - divider block tags (`<break/>` and legacy `<hr/>`) -> logical editor break line
- `editorHtmlFromBodySource(...)` is the note-editor mount projection used before writing a session file for LVRS
  `TextEditor`. It is intentionally derived from canonical source through the `.wsnbody` serializer/projection path so
  line breaks, inline style rendering, and escaped resource text stay aligned with the persisted note body contract.
  The generated editor document carries the LVRS Body token default (`Pretendard`, `12px`, medium weight, `12px`
  line height, and `#CCFFFFFF`) so the note editor does not fall back to the platform/system rich-text font before
  token-specific typography or explicit style overrides are applied.
- `editorHtmlDocumentFromProjection(...)` exposes only the final LVRS Body-token editor document wrapper. It lets
  higher-level session code compose body fragments and resource frame fragments first, then wrap that projection once,
  avoiding nested `<!DOCTYPE HTML>` payloads and resource-adjacent spacer rows.
- `sourceTextFromEditorDocument(...)` is the inverse editor-session boundary. It detects LVRS/Qt rich-text HTML,
  extracts its visible text with preserved paragraph and `<br/>` boundaries, recovers command-generated inline format
  spans as canonical source tags, and returns normalized canonical source for persistence. Non-rich RAW source is passed
  through unchanged apart from line-ending normalization.
- The same inverse editor-session boundary recognizes note-session resource frame markers of the form
  `<!--whatson-resource-source:...-->...<!--/whatson-resource-source-->`, decodes the embedded canonical
  `<resource ... />` source tag, and preserves authored blank lines around that atomic frame. This lets the editor
  display a rich resource frame while the `.wsnbody` source continues to store only the canonical resource reference
  without treating adjacent paragraphs as renderer cleanup targets.
- The inverse boundary also recognizes `<!--whatson-callout-source:...-->...<!--/whatson-callout-source-->` marker pairs,
  extracts the inner `<!--whatson-callout-content-->...<!--/whatson-callout-content-->` payload from the live rendered
  callout frame, converts its rich text back to canonical source, and wraps that content in `<callout>...</callout>`
  before persistence. The generated leading-bar image is frame chrome and is intentionally excluded from this marker
  path; the gap is its rendering margin rather than source content.
- The inverse boundary recognizes `<!--whatson-style-source:...-->...<!--/whatson-style-source-->` marker pairs and the
  matching start-anchor plus styled-span fallback that survives `QTextDocument::toHtml()` around generated style spans.
  The marker stores the canonical opening `<style ...>` token as renderer metadata, never as visible/plain text, so
  edited rich-text content can be converted back to source and wrapped with the original style attributes before
  `.wsnbody` persistence.
- If Qt/LVRS serializes the editor document after those comment and data markers have been stripped, the inverse
  boundary still recognizes a block background or text fragments carrying the callout's distinctive `#262728` frame
  style and persists that content as `<callout>...</callout>` instead of degrading it into plain paragraphs. Serialized
  object replacement characters from the frame chrome are stripped before source reconstruction. A legacy
  fallback still recognizes the older `#262728` table, `3px` leading bar, and `12px` gap-cell shape so already-serialized
  editor sessions can be recovered.
- Projection joins standalone callout block lines without inserting extra `<br/>` separators around them. Adjacent
  ordinary source lines are wrapped as zero-margin paragraphs in that path, so QTextDocument exposes one plain-text row
  for the callout instead of blank rows above or below it.
- Explicit empty source lines adjacent to a standalone callout are rendered with an invisible source-line placeholder
  so the editor still exposes a real plain-text row for the gutter. On save, that placeholder line is restored to an
  empty source line before `.wsnbody` serialization.
- Recovered rendered callout content is trimmed, and renderer-owned blank padding rows around the recovered
  `<callout>...</callout>` source line are removed before persistence. Repeated editor save/serialize cycles must
  therefore keep the callout as one canonical source line instead of cloning empty paragraphs above or below it. The
  padding cleanup only removes the immediate renderer-owned row next to a recovered callout, so intentional blank lines
  inserted by the user are preserved.
- If the marker pair survives after the single resource image object has been deleted, the inverse boundary now drops
  that empty marker block instead of restoring the canonical resource source tag. This keeps backspace/delete behavior
  aligned with the editor's one-object frame contract.
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
- Empty paragraphs that follow standalone resource/divider body blocks are likewise projected as empty
  editor source lines. A saved `<resource ... /><paragraph></paragraph>` sequence therefore reopens as
  `<resource ... />\n`, giving the structured editor a real cursor line after the atomic block.
- Empty paragraphs before a standalone block, or between two standalone blocks, are also represented by explicit newline
  boundaries in the editor source projection. The read path must not decide that an empty tag has no value merely because
  it contributes no visible characters.
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
- When `persistBodyPlainText(...)` performs a changed-body filesystem write, it now opts in to `modifiedCount`
  increments. The no-op comparison still returns before the file-store update path, so unchanged editor snapshots do
  not inflate the counter.
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
- Changed editor saves must advance `fileStat.modifiedCount`; unchanged snapshots must not inflate it.
- Body hashtags must survive a full save/load round-trip as visible `#label` text while still persisting as canonical
  `<tag>` nodes inside `.wsnbody`.
- A paragraph that starts with Tab-inserted indentation spaces must still display that indentation after a
  save/load or parse/re-render round-trip; the projection must not collapse the stored spaces back into one HTML gap.
- A style applied across multiple logical paragraphs must still render on every touched paragraph after save/load, even
  though the serializer has to split that logical span into paragraph-local reopened canonical tags.
- A typed `</break>` token must survive save/load as `</break>` in editor source while `.wsnbody` persists it as
  `<break/>`, and rich-text projection must show a logical editor break line instead of literal tag text.
- A typed or pasted web URL that canonicalizes into `<weblink href="...">label</weblink>` must survive save/load and
  rich-text projection as one active hyperlink instead of escaping back into literal XML.
- A typed inline style run such as `<bold>Al<italic>pha</italic></bold><italic> Beta</italic>` must project to styled
  HTML in the read-side projection instead of displaying the RAW tags as text.
  `<callout>message</callout>` wrappers must survive save/load inside paragraph RAW source without escaping wrapper
  width is editor fill and height is content hug; neither axis is recovered from a static Figma node dimension.
- Standalone `<resource ... />` or `</break>` source lines must round-trip as direct `<body>` children instead of being
- A direct `<resource ... />` body child followed by an empty `<paragraph></paragraph>` must project back to editor
  source with a trailing newline, not to a resource-only source string, so the post-resource caret target is preserved
  on note reopen.
- Empty `<paragraph></paragraph>` body children before or between direct resource body children must likewise survive as
  leading/interior empty editor source lines.
- A paragraph line that already contains only an escaped resource tag from an earlier bad save must recover to a direct
  `<resource ... />` body child on the next read/write turn instead of staying escaped forever.
- A saved legacy semantic body block such as `<title>`, `<subTitle>`, `<eventTitle>`, `<eventDescription>`, or
  `<next/>` must not degrade into escaped literal text on the next autosave.
  paragraph RAW source on load.
- Legacy/self-closing/non-canonical structured tags may still be normalized by the serializer/parser projection that
  the editor explicitly invoked, but passive load/save turns must not introduce an extra RAW rewrite layer on top of
  that projection.
- Reopening a note whose saved `.wsnbody` still contains visible paragraph text must not yield an empty editor body
  solely because canonical inline-tag source extraction returned `""`.
- A malformed/legacy `.wsnbody` that contains rendered editor HTML comment wrappers must not project those wrappers
  back into the editor RAW source; the read path should collapse them back to plain editor text before note-open.
