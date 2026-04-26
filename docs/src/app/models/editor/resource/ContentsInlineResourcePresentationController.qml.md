# `src/app/models/editor/resource/ContentsInlineResourcePresentationController.qml`

## Role
`ContentsInlineResourcePresentationController.qml` owns RichText-side inline resource presentation helpers.

## Responsibilities
- Resolves preview width from the current editor or print layout width budget.
- Converts imported resource entries into inline image HTML or placeholder block HTML.
- Rewrites `whatson-resource-block` placeholders inside the fallback RichText editor surface without mutating import or
  editor-session state.
