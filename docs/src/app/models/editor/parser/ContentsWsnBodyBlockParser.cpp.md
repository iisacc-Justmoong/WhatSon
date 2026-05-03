# `src/app/models/editor/parser/ContentsWsnBodyBlockParser.cpp`

## Responsibility
Implements one-pass top-level `.wsnbody` parsing for the structured editor renderer path.

## Key Behavior
- Recognizes supported document blocks from RAW source through `iiXml::Parser::TagParser` when the current source forms
  a parseable XML-tag projection:
  - semantic text blocks such as `paragraph`, `p`, `title`, `subTitle`, `eventTitle`, `eventDescription`
  - `resource`
  - `agenda`
  - `callout`
  - `break`
- The iiXml tree is now the only explicit block-span authority. The previous regular-expression tag scanner and its
  malformed-open-block recovery path have been removed, so semantic block classification no longer forks between two
  parser implementations.
- Collects those blocks into one ordered `renderedDocumentBlocks` list by source position, then fills plain-text gaps
  between explicit blocks as paragraph-like prose blocks instead of one monolithic gap fragment.
- Plain prose that is not already wrapped in an explicit semantic block is now split on logical source newlines.
  Blank lines therefore survive as empty paragraph blocks, which lets the structured editor move/focus/edit below
  inline resources without collapsing multiple authored paragraphs into one giant `type=text` payload.
- Builds agenda/callout payloads directly during that same scan, including task text, done-state attributes,
  source-span metadata, and verification counters.
- Malformed transient edit states that iiXml cannot parse are left as ordinary RAW prose projection until the linter or
  the next edit produces a parseable structured tag tree.
- Semantic text blocks now deliberately split wrapper geometry from editable content geometry:
  - `blockSourceStart` / `blockSourceEnd` keep the full outer tag span
  - `contentStart` / `contentEnd` and `sourceStart` / `sourceEnd` point at the inner editable payload
  - `sourceText` therefore contains only the inner content, including inline style tags but excluding the outer
    semantic wrapper
- Every emitted block now also carries one generic block-trait payload for the structured QML host:
  - `plainText`
  - `textEditable`
  - `atomicBlock`
  - `gutterCollapsed`
  - `logicalLineCountHint`
  - `minimapVisualKind`
  - `minimapRepresentativeCharCount`
  This lets the flow host treat paragraph/resource/agenda/callout/break blocks uniformly as document blocks even
  before the concrete QML delegate has mounted.
  `logicalLineCountHint` is intentionally normalized to an `int`-sized value before publishing so the parser does not
  leak `qsizetype`-dependent width differences into the QML payload contract.
- Resource blocks keep stable `resourceIndex`, `resourceId`, `resourcePath`, `resourceType`, and `resourceFormat`
  fields so QML can reconcile them with `ContentsBodyResourceRenderer`.
- Separator newlines around explicit blocks are no longer treated the same as ignorable indentation-only whitespace.
  The parser now discards only horizontal formatting whitespace between neighboring explicit blocks; authored newline
  runs are preserved as actual paragraph slots in the document projection.
- When a prose gap touches an explicit block boundary, the parser now trims only the single boundary-adjacent
  whitespace-only line that comes from XML pretty-print formatting.
  Interior empty lines are still preserved as real blank paragraphs.
- The parser still delegates canonicalization and verification to `WhatSonStructuredTagLinter`; it does not invent a
  second source-normalization authority.

## Why It Exists
The structured editor is being moved toward a single document-block engine rather than a hybrid of per-tag overlay
parsers. This parser is the first dedicated layer in that replacement path: it gives the renderer one canonical block
projection to publish into QML, which is necessary before the remaining legacy whole-note editor paths can be retired.
