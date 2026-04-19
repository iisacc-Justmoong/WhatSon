# `src/app/models/display/paper/ContentsTextFormatRenderer.cpp`

## Responsibility
Implements inline-format rendering from note-editor text to RichText HTML.

## Key Behavior
- Normalizes CRLF/CR to LF before parsing.
- Escapes plain text segments to keep HTML output safe.
- The renderer now maintains two different HTML outputs:
  - `editorSurfaceHtml`: cheap source-editing HTML for the live editor
  - `renderedHtml`: optional markdown-aware preview HTML, recomputed only while preview is enabled
- `paperPaletteEnabled` now lets the same renderer recolor both HTML payloads for page/print mode:
  - the editor/preview HTML keeps the same semantic spans and block structure
  - screen-only bright foregrounds are remapped to dark-on-paper colors before the HTML leaves C++
  - the published `htmlTokens` / `normalizedHtmlBlocks` snapshots are recolored through the same pass so downstream
    consumers do not drift from `editorSurfaceHtml`
- The live editor HTML path now has one explicit intermediate pipeline:
  - `ContentsWsnBodyBlockParser` reparses RAW source into ordered document blocks
  - `ContentsHtmlBlockRenderPipeline` converts those blocks into `htmlTokens`
  - the same pipeline resolves one normalized HTML block per token and publishes `normalizedHtmlBlocks`
  - `ContentsTextFormatRenderer` finally republishes the joined `editorSurfaceHtml` plus the intermediate payloads to
    QML
- When that pipeline reports a deterministic canonical structured RAW projection, the renderer now also feeds that
  corrected snapshot into:
  - the legacy whole-document inline-style composer
  - the markdown-aware preview renderer
  so self-closing structured tags and legacy divider aliases cannot consume trailing note text differently from the
  parser-owned final block projection.
- `htmlOverlayVisible` is now derived in C++ from that normalized block pipeline instead of being guessed only from one
  inline-style regex in QML.
  Semantic text blocks such as `title`, `subTitle`, and `eventTitle` can therefore request the styled overlay even
  when their RAW inner text contains no inline formatting tag.
- The live editor surface now treats markdown as raw `.wsnbody` source:
  - list markers, headings, blockquotes, fenced code fences, inline code, and markdown links remain literal text in
    `editorSurfaceHtml`
  - only proprietary inline tags such as `<bold>` / `<italic>` / `<highlight>` are promoted into styled HTML spans
- The live editor surface no longer flattens ordinary body text into one `<br/>`-stitched HTML run.
  It now projects non-structured body text as paragraph-like RichText document blocks (`<p ...>...</p>`), which keeps
  the desktop/mobile `TextEdit.RichText` layout closer to Qt's native raw rich-text flow and gives inline media/image
  blocks a real document slot instead of an overlay-only approximation.
- Single structural line breaks that only separate a proprietary block (`resource`, `agenda`, `callout`, `break`) from
  the next body fragment are consumed as block boundaries rather than re-rendered as extra blank paragraphs.
- The preview path still recognizes markdown-style block prefixes without rewriting the stored source text:
    - unordered list markers (`- ` / `* ` / `+ `) render as bullet-list lines
    - leading literal bullet markers (`• `) also render through the same unordered-list role so pasted or legacy source
      lines do not fall back to plain black text in markdown preview views
    - ordered list markers (`1. ` / `2. ` / `3) `) render as numbered-list lines
    - heading markers (`#` ... `######`) render as larger title lines while keeping the marker text visible
    - blockquotes (`> `) render as muted italic quote lines
    - fenced code blocks keep the literal `` ``` `` lines but render the fenced region with monospace/code styling
- The RichText spans for those markdown roles are emitted through `WhatSonNoteMarkdownStyleObject`, not ad-hoc
  free-form CSS strings.
- Converts supported inline style tags to explicit RichText/CSS spans:
  - `bold`/`b`/`strong` -> `<strong style="font-weight:900;">`
  - `italic`/`i`/`em` -> `<span style="font-style:italic;">`
  - `underline`/`u` -> `<span style="text-decoration: underline;">`
  - `strikethrough`/`strike`/`s`/`del` -> `<span style="text-decoration: line-through;">`
- Routes `highlight` / `mark` tags through `ContentsTextHighlightRenderer` and renders an Apple Notes-inspired
  highlight span (`#8A4B00` background, `#D6AE58` foreground, semibold text).
- In page/print mode that highlight and the other markdown/semantic bright colors are now remapped to a paper-safe
  palette (`#111111` text, lighter code/highlight backgrounds, darker muted/link accents) so white-on-paper spans do
  not survive into the final RichText payload.
- Styles inline markdown-shaped literals that do not conflict with proprietary formatting tags:
    - inline code spans (`` `code` ``)
    - link-shaped literals (`[label](url)`)
- Proprietary RAW hyperlinks now render as real anchors instead of literal text:
  - `<weblink href="www.iisacc.com">아이작닷컴</weblink>` becomes an active RichText `<a href="https://www.iisacc.com">`
  - the same link color string is shared with paper-palette recoloring, so page/print mode still darkens hyperlinks
- Proprietary formatting remains authoritative for bold/italic/underline/strikethrough/highlight; markdown emphasis
  tokens are intentionally not promoted into those styles.
- Converts `<br>` style tags and newline characters to `<br/>`.
- The renderer now also resolves legacy semantic body tags through the same note-body semantic registry used by
  persistence:
  - `<next/>` renders as a real line break instead of literal tag text
  - `<title>`, `<subTitle>`, and `<eventTitle>` render as heading-style text spans
  - `<event>` becomes a transparent wrapper in read-side HTML instead of painting literal open/close tags
- Converts the canonical single divider tag `</break>` (and legacy `<hr ...>` aliases) into rendered divider HTML
  (`<hr/>`) on both the live editor surface and preview HTML.
- The live editor surface no longer drops `<resource ...>` tags as zero-height markup.
  Instead it emits a single-placeholder-paragraph block for each resource tag, wrapped in stable
  `<!--whatson-resource-block:N--> ... <!--/whatson-resource-block:N-->` markers.
  The editor host can then replace those exact slots with resolved inline media HTML while keeping the same
  one-logical-line contract that the structured block parser and logical-text bridge use.
- The live editor surface now also projects proprietary structured blocks into editor-owned RichText flow instead of
  treating them as zero-height skipped markup:
  - `<agenda ...><task ...>...</task>...</agenda>` emits one padded editor block that contains only task body text,
    joined with synthetic `<br/>` separators between tasks
  - `<callout>...</callout>` emits one padded editor block that contains the authored callout body text
  - those blocks intentionally do not emit editable header/date/checkbox glyphs; the host QML card chrome remains
    responsible for non-source visuals, while the renderer owns the text-flow slot that the cursor and typing system
    use
  - empty task/callout anchors are preserved as `&nbsp;`-backed editable placeholders so blank cards stay cursor-
    reachable immediately after insertion
- Proprietary `<task ...>` wrapper tags still stay out of the editor text flow as raw literal markup.
- Escapes unsupported tags as literal text instead of executing arbitrary markup.
  Legacy tags that are part of the shared semantic registry no longer fall into that unsupported bucket.
- Recognizes supported `<span style=...>` runs and folds them back into the same canonical style stack, so stored
  `.wsnbody` aliases and editor-generated HTML spans render through one path.
- Auto-closes unmatched open style tags at end-of-input to keep emitted HTML structurally valid.
- Emits `renderedHtmlChanged()` only when the derived HTML payload actually changes.
- Decodes one entity layer from safe literal text fragments (`&amp;`, `&lt;`, `&gt;`, `&quot;`, `&#39;`, `&nbsp;`)
  before emitting RichText HTML, so RAW-safe source escapes render as their real glyphs in the editor while the
  canonical source can still keep escaped tokens.
- Literal-text rendering now preserves visible horizontal whitespace too:
  - leading indentation in ordinary paragraph lines is emitted as `&nbsp;` so a typed Tab-space run survives the next
    editor-surface refresh instead of collapsing back to zero-width HTML spacing
  - repeated interior spaces alternate plain spaces and `&nbsp;`, keeping the authored gap width without forcing every
    word separator into a non-breaking span
  - the same whitespace policy is reused by markdown preview and structured block literal rendering, so callout/agenda
    lines do not disagree with plain paragraph lines about how inserted indentation should look
- Exposes `normalizeInlineStyleAliasesForEditor(...)` for editor-surface normalization:
  - rewrites inline aliases into editable RichText tags (`bold` -> `<strong style="font-weight:900;">`, etc.)
  - preserves non-style tags such as `<resource ...>` unchanged
  - normalizes canonical source LF-delimited text into paragraph-style RichText document fragments instead of one flat
    `<br/>` chain
  - renders escaped safe text such as `&lt;bold&gt;...&lt;/bold&gt;` as visible `<bold>...</bold>` glyphs without
    promoting it into authoritative inline-style tags
  - deliberately stops promoting markdown syntax into list/heading/blockquote/code/link preview spans on the live
    editor surface
- Exposes `plainTextFromEditorSurfaceHtml(...)` as the inverse read-side helper for the QML editor wrapper:
  - feeds one serialized Qt RichText document into `QTextDocument`
  - returns the visible plain text after Qt removes the `<!DOCTYPE HTML ... qrichtext ...>` scaffold
  - keeps that conversion strictly read-only so whole-document RichText HTML still cannot become `.wsnbody` source
- Exposes `applyPlainTextReplacementToSource(...)` so ordinary typing can mutate only the affected raw source span:
  - accepts canonical source text plus source start/end offsets
  - normalizes plain-text line endings
  - clamps source offsets against an `int`-safe `QString` length before replacement
  - escapes inserted literal text before stitching it back into `.wsnbody`
  - when the replacement commits a completed/pasted URL, rewrites the authoritative RAW source into canonical
    `<weblink href="...">...</weblink>` form instead of leaving an inert literal
  - avoids whole-document RichText export for normal typing/backspace/delete/paste
- Exposes `applyInlineStyleToLogicalSelectionSource(...)` for shortcut/context-menu formatting that must ignore markdown
  presentation roles:
    - no longer permits any whole-surface `QTextDocument`/RichText serialization path to become the source of truth
    - reuses the same local tag-token and entity classifiers as the rest of the renderer pipeline, so RAW-source
      coverage scans and HTML/source parsing stay aligned
    - instead scans the RAW editor source, computes logical character coverage for each proprietary inline style, and
      rebuilds canonical `<bold>` / `<italic>` / `<underline>` / `<strikethrough>` / `<highlight>` tags directly from
      that coverage
    - the selected range is therefore compared against the actual stored source tags, not against a temporary RichText
      fragment split that may collapse multi-tag or multi-paragraph selections
    - `plain` clears every supported proprietary inline style across the selected logical range
    - reapplying the same style toggles that style off by zeroing its RAW-source coverage inside the range
    - applying a style across text that is split by other proprietary tags now expands one continuous source-tag span
      over the whole logical selection
    - markdown line prefixes are now style-protected during RAW coverage rebuild:
      - leading indentation (`[ \t]*`)
      - unordered list prefixes (`- ` / `+ ` / `* ` / `• `)
      - ordered list prefixes (`N. ` / `N) `)
      - heading prefixes (`#` .. `######` + whitespace)
      - blockquote/fence prefixes (`>`, `` ``` ``)
      so inline style tags are not inserted in front of structural markers
- Preview HTML generation and editable-surface normalization now reuse the same strong/span-style openings, so
  read-side rendering and editor rendering no longer diverge on weight/decoration styling.

## Regression Checks

- Typing `- item` in the note body must keep `- ` visible on the live editor surface; the source marker must not be
  replaced by a rendered bullet glyph in that editing path.
- Pressing `Tab` at the start of an ordinary paragraph line must keep the inserted indent visibly rendered after the
  editor surface reparses RAW source; the cursor must not be the only thing that moves.
- A stored source line that already begins with `• ` must render with the same unordered-list marker styling as `- `
  instead of falling back to plain black text in markdown preview views.
- Typing `1. item` should re-render as a numbered-list line.
- Typing `# title` should re-render as a heading-like line while preserving literal `# ` in the source text.
- Typing `` ``` `` fenced blocks should keep the fence markers visible and render the fenced body as code-styled text.
- Typing `[label](https://example.com)` should keep the literal token in source while rendering it as a link-styled
  span.
- Safe RAW entity tokens such as `&lt;`, `&gt;`, and `&amp;` must render as visible `<`, `>`, and `&` glyphs in the
  editor surface without being promoted into executable markup.
- Escaped safe text such as `&lt;bold&gt;demo&lt;/bold&gt;` must display the literal angle-bracket text in the editor
  instead of the raw entity strings, while proprietary formatting still ignores it as source text.
- Applying proprietary inline formatting to text inside a markdown bullet line must keep the stored source line prefix
  as
  `- ` instead of persisting the rendered `• ` glyph.
- Applying `Strikethrough` (or any inline style) to a range that crosses indented/list lines must not delete or
  visually collapse the line indentation; structural line prefixes must remain outside proprietary inline tags.
- Applying proprietary inline formatting to a selection that spans multiple existing proprietary tags or multiple
  logical paragraphs must rebuild the target RAW source tags over the whole selected range, not only the first RichText
  fragment that happened to survive the context-menu click.
- No editor command path should require serializing the rendered RichText surface back into `.wsnbody`.
  Formatting, typing, paste, and delete flows must all start from RAW-source ranges and finish with a re-render from
  reparsed RAW.
- If Qt hands the editor wrapper a serialized RichText document payload that begins with `<!DOCTYPE HTML` /
  `qrichtext`, the read-side plain-text bridge must strip that scaffold back to the same visible text the user saw.
- Applying `Bold`/`Italic`/`Underline`/`Highlight` shortcuts to heading, blockquote, link-literal, or code-literal text
  must still add/remove proprietary `.wsnbody` tags instead of misreading markdown presentation styling as an
  already-applied shortcut format.
- A stored `</break>` token must render as a visible divider line (not literal text) and still consume one logical
  break slot in inline-style selection coverage rebuild.
- A stored `<agenda>/<task>` block must reserve real editor text-flow height at the authored location, and task body
  text must render inside that reserved slot instead of continuing in the surrounding paragraph stream.
- A stored `<callout>...</callout>` block must reserve real editor text-flow height at the authored location, and the
  callout body text must render inside that reserved slot instead of continuing in the surrounding paragraph stream.
- A stored `<resource ... />` tag must likewise reserve a stable inline editor slot, and image resources must render
  inside that same RichText body flow instead of depending on a second overlay-only card layer.
- A stored legacy semantic block such as `<title>`, `<subTitle>`, `<eventTitle>`, `<eventDescription>`, or `<next/>`
  must render as semantic content, not as escaped literal XML text.
- The live editor surface should now expose ordinary prose as paragraph-style RichText document blocks instead of one
  flat `<br/>` chain, so inline media can share the same native document flow as surrounding text.
- When preview is disabled, mutating `sourceText` must not recompute markdown-aware preview HTML.
- For a semantic heading block in structured-flow mode, `htmlOverlayVisible` must flip to `true` even when the inner
  RAW text itself contains no `<bold>`/`<italic>`-style inline tags.
- The renderer must now expose one `htmlTokens` / `normalizedHtmlBlocks` snapshot per render pass, and that snapshot
  must stay aligned with the parser-owned block order for the same RAW source turn.
- Enabling `paperPaletteEnabled` must recolor both `editorSurfaceHtml` and `renderedHtml` without leaving screen-only
  bright foreground colors such as `#F3F5F8`, `#8CB4FF`, or `#D6AE58` in the emitted page/print HTML.
- If one note body uses inline style tags that span multiple parsed text blocks, the renderer must keep the carry-aware
  legacy document composer for that source turn instead of dropping the carried style during block-normalized HTML
  assembly.
- A source such as `<callout/>After <bold>bold</bold>` must render an empty callout block followed by ordinary text in
  both `editorSurfaceHtml` and `renderedHtml`; the self-closing callout must not swallow the trailing text just
  because the final renderer revisited the uncorrected RAW snapshot.
