# `src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.cpp`

## Responsibility
Implements RAW insertion payload generation for generated editor body tags.

## Behavior
- `structuredShortcutInsertionSpec(...)` normalizes agenda, callout, and break shortcut kinds into canonical source
  text.
- `buildStructuredShortcutInsertionPayload(...)` combines canonical tag text with the target RAW source and returns the
  next source snapshot plus the cursor source offset.
- `buildCalloutRangeWrappingPayload(...)` wraps a selected RAW source range with explicit
  `<callout>...</callout>` tags instead of inserting an empty callout when the user already has a text range selected.
- `buildRawSourceInsertionPayload(...)` is the generic raw-tag path used for resource tags and other prebuilt tag text.
- Insertions are moved to the end of an enclosing agenda or callout block when the requested cursor offset is inside
  that structured visual block, preventing nested visual body tags.
- Callout range wrapping rejects selections that overlap an existing agenda or callout block, preventing nested visual
  body tags.
- The planner inserts surrounding newlines when the target source span is adjacent to ordinary text.

## Verification
- C++ regression coverage checks canonical callout insertion, selected-range callout wrapping, nested callout-wrap
  rejection, agenda spec generation, nested block escape insertion, generic resource-tag insertion, and missing backend
  rejection.
