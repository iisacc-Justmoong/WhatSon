# `src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js`

## Responsibility
Provides pure helper functions that turn editor tag-management intents into direct `.wsnbody` string splice payloads.

## Exported helpers
- `normalizeSourceText(sourceText)`: normalizes newline and placeholder characters before offset math.
- `resolveStructuredTagInsertionOffset(sourceText, requestedInsertionOffset)`: refuses to insert new agenda/callout
  wrappers into the middle of an existing structured block and instead resolves to that block tail.
- `buildRawSourceInsertionPayload(...)`: wraps a prebuilt raw tag string with boundary newlines when needed and returns
  `nextSourceText` plus the focus `sourceOffset`.
- `buildStructuredShortcutInsertionPayload(...)`: emits canonical RAW payloads for agenda, callout, and break
  shortcuts.
- `buildCalloutRangeWrappingPayload(...)`: wraps a selected RAW range with `<callout>...</callout>` when that range
  does not overlap an existing structured block and does not already contain document-block tag tokens.

## Boundary
- This file is intentionally not a QML object or C++ bridge.
- Callers own the actual `.wsnbody` write and persistence scheduling.
- Parser and renderer remain read-side consumers of the next RAW snapshot produced from these helpers.
