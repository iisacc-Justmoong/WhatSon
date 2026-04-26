# `src/app/models/editor/tags/ContentsCalloutBackend.cpp`

## Responsibility
Owns callout-domain parsing and source rewrites for `<callout>` blocks.

## Current Contract
- Parse entries now include full source geometry:
  - `sourceStart`, `sourceEnd`
  - `openTagStart`, `openTagEnd`
  - `contentStart`, `contentEnd`
  - `closeTagStart`, `closeTagEnd`
  - `hasCloseTag`
- Empty callouts remain renderable because parsing still emits an entry even when the inner text is blank.
- `detectCalloutEnterReplacement(...)` remains the backend-owned "double Enter to exit" rule.

## Why It Changed
The new document-flow editor renders callouts as native `.wsnbody` blocks and rewrites their body text directly.
That requires exact source boundaries instead of display-only overlay metadata.
