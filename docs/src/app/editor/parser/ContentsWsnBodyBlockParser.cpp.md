# `src/app/editor/parser/ContentsWsnBodyBlockParser.cpp`

## Responsibility
Implements one-pass top-level `.wsnbody` parsing for the structured editor renderer path.

## Key Behavior
- Recognizes supported document blocks from RAW source in one scan:
  - semantic text blocks such as `paragraph`, `p`, `title`, `subTitle`, `eventTitle`, `eventDescription`
  - `resource`
  - `agenda`
  - `callout`
  - `break`
- Collects those blocks into one ordered `renderedDocumentBlocks` list by source position, then fills plain-text gaps
  between explicit blocks as ordinary `type=text` entries.
- Builds agenda/callout payloads directly during that same scan, including task text, done-state attributes,
  source-span metadata, and verification counters.
- Recovers one malformed top-level open block before a sibling explicit block starts.
  This keeps the parser from dropping the earlier block entirely when the source is missing a close tag.
- Semantic text blocks now deliberately split wrapper geometry from editable content geometry:
  - `blockSourceStart` / `blockSourceEnd` keep the full outer tag span
  - `contentStart` / `contentEnd` and `sourceStart` / `sourceEnd` point at the inner editable payload
  - `sourceText` therefore contains only the inner content, including inline style tags but excluding the outer
    semantic wrapper
- Resource blocks keep stable `resourceIndex`, `resourceId`, `resourcePath`, `resourceType`, and `resourceFormat`
  fields so QML can reconcile them with `ContentsBodyResourceRenderer`.
- The parser still delegates canonicalization and verification to `WhatSonStructuredTagLinter`; it does not invent a
  second source-normalization authority.

## Why It Exists
The structured editor is being moved toward a single document-block engine rather than a hybrid of per-tag overlay
parsers. This parser is the first dedicated layer in that replacement path: it gives the renderer one canonical block
projection to publish into QML, which is necessary before the remaining legacy whole-note editor paths can be retired.
