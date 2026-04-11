# `src/app/callout/ContentsCalloutBackend.cpp`

## Responsibility
Implements callout source parsing and insertion helpers shared by desktop/mobile editor surfaces.

## Key Behaviors
- Parses `<callout ...>` openings into QML-friendly entries (`text`, `sourceStart`, `focusSourceOffset`,
  `tagVerified`) even before a matching `</callout>` arrives, so renderer surfaces can show the callout frame as soon
  as the RAW opening tag is detected.
- Publishes parser verification reports after every parse pass:
  - `calloutOpenCount`, `calloutCloseCount`, `calloutParsedCount`
  - self-closing callout count
  - non-canonical callout attribute count
  - `wellFormed`
  - `issues`
- Separates render visibility from structural verification:
  - open-only callouts still produce a visible render entry
  - `calloutParsedCount` continues to count only fully closed callout pairs
  - missing close tags therefore remain visible while validator/runtime still receive `wellFormed=false`
- Delegates verification-map construction to `file/validator/WhatSonStructuredTagLinter`, keeping callout canonical
  syntax rules in the file/domain validation layer.
- Canonicalizes callout display text for rendering:
  - normalizes line endings
  - preserves source line breaks (`\n`) and maps `<br>` aliases to line breaks
  - decodes safe entities (`&lt;`, `&gt;`, `&amp;`, `&quot;`, `&#39;`, `&nbsp;`)
  - strips nested source tags from rendered callout text projection
- Builds canonical callout insertion payloads:
  - `<callout>...</callout>`
  - when inserted body text is empty, writes a single-space edit anchor (`<callout> </callout>`) so cursor/Enter
    logic can address callout scope through logical/source offset mapping.
  - for that empty-anchor case, cursor placement targets callout-body start (before the anchor).
- Detects callout-exit `Enter` behavior:
  - when the user presses `Enter` on a trailing empty line inside `<callout>...</callout>`
  - rewrites that span so editing exits the callout block (`</callout>\n`) instead of stacking extra empty lines.
- Emits `focusSourceOffset` at callout-body start, so renderer-hosted callout cards can send cursor focus back into the
  underlying RAW source body when tapped.

## Architectural Notes
- Callout parsing/mutation regex and entity handling are intentionally localized here so QML controllers only manage
  event flow and cursor orchestration.
- The module does not perform persistence directly; it only returns transformed source text or insertion metadata.

## Regression Checklist
- `<callout>Message</callout>` must parse as one callout entry.
- `<callout></callout>` or `<callout> </callout>` must still parse as one visible callout entry even when body text is
  empty.
- `<callout>` must still parse as one visible provisional callout entry even before `</callout>` exists.
- `<callout>Line 1` must keep the callout row visible while parser verification reports the missing close tag.
- Parser verification must report `wellFormed=false` when callout open-close counts diverge or complete block matches
  cannot confirm every callout tag pair.
- `buildCalloutInsertionPayload("abc")` must return canonical source `<callout>abc</callout>` with cursor inside
  the callout body.
- `buildCalloutInsertionPayload("")` must include a one-space body anchor and place cursor at callout-body start.
- `buildCalloutInsertionPayload(...)` must escape raw `<`, `>`, `&`, and quotes in inserted body text.
- `<callout>Line 1<br/>Line 2</callout>` must parse into display text with preserved line breaks.
- Pressing `Enter` twice at the end of a callout must exit the callout block on the second `Enter`.
