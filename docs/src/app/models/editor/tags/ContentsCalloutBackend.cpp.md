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
- `detectCalloutEnterReplacement(...)` is the backend-owned plain-Enter exit rule:
  - the callout content before the cursor remains inside `<callout>...</callout>`
  - the callout content after the cursor is moved after `</callout>\n`
  - the returned cursor offset lands in the paragraph created after the callout

## Why It Changed
The new document-flow editor renders callouts as native `.wsnbody` blocks and rewrites their body text directly.
That requires exact source boundaries instead of display-only overlay metadata. Callout line breaks are now explicit
Shift+Enter native text edits; plain Enter is a body-tag command that closes the callout at the current source cursor.
