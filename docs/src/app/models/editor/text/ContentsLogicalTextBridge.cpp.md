# `src/app/models/content/ContentsLogicalTextBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/models/content/ContentsLogicalTextBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsLogicalTextBridge.cpp`
- Approximate line count: 127

## Runtime Line-Metric Contract

- The bridge now normalizes incoming editor source through one editor-local scanner that preserves RAW markdown
  markers, decodes safe entities, skips proprietary tags, and expands proprietary block tags into the same logical
  line structure that the live editor surface exposes.
- The bridge no longer rewrites markdown list markers (`- ` / `* ` / `+ `) into `• `.
  Logical text now preserves raw markdown markers so the source editor and `.wsnbody` format stay aligned.
- `logicalLineCount` is derived from normalized plain text and exposed only as a count for document-scale consumers.
  Per-row start offsets, character counts, and offset-to-row queries are not part of this public bridge.
- The implementation now also emits a QML-visible `logicalText` projection, giving selection-formatting code access to
  the exact plain-text string that the offset table was built from.
- The implementation now also builds a logical-text-to-source offset table:
  - inline tags are treated as zero-width source tokens
  - `<br>`, canonical `</break>`, and legacy `<hr>` each count as one logical line-break character
  - inside `<agenda>`, each later `<task>` start now also emits one synthetic logical line-break step before that
    task body, matching the plain-text projection produced by `NoteBodyPersistence`
  - `<resource ... />` now reserves exactly one logical line, matching the structured block parser and fallback editor
    projection instead of expanding into a multi-line placeholder span
  - `<tag>` emits one logical `#` glyph so tag markers still occupy cursor space
  - common HTML entities (`&lt;`, `&amp;`, `&#39;`, etc.) collapse to one logical character
- That entity-collapse rule now intentionally matches the editor RichText surface, which renders RAW-safe entity tokens
  as their real glyphs instead of exposing the literal escape strings.
- `sourceOffsetForLogicalOffset(...)` exposes that table to QML so selection/context-menu formatting can mutate the
  correct source slice even when the editor cursor operates on rendered plain-text offsets.
- `logicalOffsetForSourceOffset(...)` performs the reverse cursor projection from an authoritative RAW cursor offset
  against the whole source snapshot. Positions inside zero-width inline tag tokens map to the current visible logical
  boundary, so projected caret placement does not drift while native cursor movement passes through hidden RAW tags.
- Those logical offsets now also advance past any immediately adjacent closing proprietary inline-style tags
  (`</bold>`, `</italic>`, `</underline>`, `</strikethrough>`, `</highlight>`).
  A visible boundary that sits after the last styled glyph therefore resolves to the RAW position after the zero-width
  closing tag instead of to the interior edge just before it.
- `logicalToSourceOffsets()` exports the cached table in one QML-visible array so visible editor offsets can be mapped
  back into RAW source without asking C++ for one offset per character.
- `logicalLengthForSourceText(...)` now reuses the same normalization path for ad-hoc fragments, giving QML list-toggle
  code a stable way to measure rewritten source lines in logical editor coordinates before restoring selection/cursor
  state.
- Source-offset clamping is now normalized through an explicit `int`-bounded QString size helper before returning to
  QML, which avoids libc++ `std::clamp` template mismatches between `int` and `qsizetype` on Apple toolchains.
- Cached offset export normalizes Qt container sizes through the same bounded integer helper before `reserve(...)`, so
  Apple libc++ does not see mixed `int` / `qsizetype` candidates.

## Regression Checks

- When a stored note line begins with `- `, `* `, or `+ `, the bridge logical text must expose the same raw marker.
- Fenced code blocks delimited by `` ``` `` must keep their raw markdown markers in `logicalText`.
- When the source contains RAW-safe entities such as `&lt;tag&gt;` or `Tom &amp; Jerry`, `logicalText` must expose the
  visible `<tag>` / `Tom & Jerry` glyph sequence so cursor mapping stays aligned with the editor surface.
- When a single rewritten source line contains inline tags, entities, or resource tags, `logicalLengthForSourceText(...)`
  must still return the rendered logical length that the editor selection uses after the rewrite.
- `logicalToSourceOffsets()` must stay aligned with the same normalized logical text that `logicalText` exposes, so
  source splices do not shift away from the intended logical cursor positions.
- `logicalToSourceOffsets()` export must keep compiling when Qt container `size()` returns `qsizetype` on Apple
  toolchains.
- When source contains canonical `</break>` divider tags, `logicalToSourceOffsets()` must still expose one logical
  character step per divider so selection-to-source splices stay aligned.
- When source contains multiple `<task>` children inside one `<agenda>`, `logicalToSourceOffsets()` must expose one
  logical line-break step between adjacent task bodies so cursor restoration and source splices can land inside the
  intended task instead of drifting to surrounding text.
- When source contains `<resource ... />`, `logicalText` and `logicalToSourceOffsets()` must reserve the same
  single-line slot that the structured block parser and fallback RichText projection use, so mobile plain-text editing
  and desktop overlay positioning both align to the authored resource tag.
- When the visible caret sits just after a proprietary inline-style run, `sourceOffsetForLogicalOffset(...)` must not
  point back in front of that closing style tag.
  Pressing `Enter` or typing more prose there must not split `</highlight>` or expose RAW tag tails in the editor.
- When the native RAW cursor is inside an opening or closing inline-style tag, `logicalOffsetForSourceOffset(...)` must
  keep reporting the adjacent visible boundary. Moving through hidden tag bytes must not move the projected overlay
  caret through unrelated logical text.

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: no

### Classes and Structs
- None detected during scaffold generation.

### Enums
- None detected during scaffold generation.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
