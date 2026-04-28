# `src/app/models/editor/format/ContentsRawInlineStyleMutationSupport.js`

## Responsibility

Builds direct RAW `.wsnbody` inline-style mutations from string input and logical selection offsets.

## Current Behavior

- Accepts ordinary inline-format requests for `bold`, `italic`, `underline`, `strikethrough`, `highlight`, and
  `plain`.
- Resolves the selected visible-text boundaries back into RAW source offsets without routing through a renderer QObject.
- For ordinary styles, it inserts one opening tag on the left boundary and one closing tag on the right boundary.
- Existing opening wrappers such as `<bold>` or `<weblink ...>` are skipped before insertion so the new tag wraps the
  selected content itself instead of the wrapper token.
- `plain` is intentionally narrower: it only removes inline-style tags that directly wrap the selected RAW span.
- The helper does not rebuild global style coverage or canonicalize unrelated surrounding markup.
- The helper is self-contained and pure. QML controllers call it and then write `nextSourceText` back into the note
  session; it does not own persistence, focus, or rendering state.
