# `src/app/viewmodel/content/ContentsLogicalTextBridge.cpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/content/ContentsLogicalTextBridge.cpp`
- Source kind: C++ implementation
- File name: `ContentsLogicalTextBridge.cpp`
- Approximate line count: 127

## Runtime Line-Metric Contract

- The bridge now normalizes incoming editor source through the canonical `.wsnbody` serializer /
  parser path (`serializeBodyDocument(...)` + `plainTextFromBodyDocument(...)`) before deriving
  logical offsets.
- This keeps logical text identical to the same plain-text projection used by editor persistence,
  so gutter/minimap/selection geometry cannot drift when RichText whitespace or block markup is
  interpreted differently from the note-body parser.
- The bridge no longer rewrites markdown list markers (`- ` / `* ` / `+ `) into `• `.
  Logical text now preserves raw markdown markers so the source editor and `.wsnbody` format stay aligned.
- `logicalLineCharacterCountAt(...)` uses normalized plain-text length (`m_logicalText`) instead of
  raw rich-text source length. This prevents gutter/minimap geometry drift when the source contains
  markup tokens that do not map 1:1 to cursor offsets.
- The implementation now also emits a QML-visible `logicalText` projection, giving selection-formatting code access to
  the exact plain-text string that the offset table was built from.
- The implementation now also builds a logical-text-to-source offset table:
  - inline tags are treated as zero-width source tokens
  - `<br>` counts as one logical line-break character
  - common HTML entities (`&lt;`, `&amp;`, `&#39;`, etc.) collapse to one logical character
- That entity-collapse rule now intentionally matches the editor RichText surface, which renders RAW-safe entity tokens
  as their real glyphs instead of exposing the literal escape strings.
- `sourceOffsetForLogicalOffset(...)` exposes that table to QML so selection/context-menu formatting can mutate the
  correct source slice even when the editor cursor operates on rendered plain-text offsets.
- `logicalToSourceOffsets()` now exports the cached table in one QML-visible array so the typing controller can reseed
  its incremental mapping state after each idle/blur presentation commit instead of asking C++ for one offset per
  keystroke.
- `adoptIncrementalState(...)` now lets QML push the already-spliced live typing state back into the bridge:
  - source text
  - logical text
  - logical line-start offsets
  - logical-to-source offsets
- That adoption path updates the cached state and emits the same observable signals only when the pushed values actually
  differ, so typing keeps bridge consumers synchronized without routing every character through `refreshTextState()`.
- `logicalLengthForSourceText(...)` now reuses the same normalization path for ad-hoc fragments, giving QML list-toggle
  code a stable way to measure rewritten source lines in logical editor coordinates before restoring selection/cursor
  state.
- Source-offset clamping is now normalized through an explicit `int`-bounded QString size helper before returning to
  QML, which avoids libc++ `std::clamp` template mismatches between `int` and `qsizetype` on Apple toolchains.
- Cached offset export and incremental vector adoption now also normalize Qt container sizes through the same bounded
  integer helper before `reserve(...)`, so Apple libc++ does not see mixed `int` / `qsizetype` `std::max(...)`
  candidates while the bridge snapshots live typing state.

## Regression Checks

- When a stored note line begins with `- `, `* `, or `+ `, the bridge logical text must expose the same raw marker.
- Fenced code blocks delimited by `` ``` `` must keep their raw markdown markers in `logicalText`.
- When the source contains RAW-safe entities such as `&lt;tag&gt;` or `Tom &amp; Jerry`, `logicalText` must expose the
  visible `<tag>` / `Tom & Jerry` glyph sequence so cursor mapping stays aligned with the editor surface.
- When a single rewritten source line contains inline tags, entities, or resource tags, `logicalLengthForSourceText(...)`
  must still return the rendered logical length that the editor selection uses after the rewrite.
- `logicalToSourceOffsets()` must stay aligned with the same normalized logical text that `logicalText` exposes, so a
  post-idle typing-session reseed cannot shift source splices away from the intended logical cursor positions.
- `adoptIncrementalState(...)` must not emit redundant change signals or rebuild offset tables when QML pushes an
  identical live-typing snapshot.
- `logicalToSourceOffsets()` and `buildIntVector(...)` must keep compiling when Qt container `size()` returns
  `qsizetype` on Apple toolchains.

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
