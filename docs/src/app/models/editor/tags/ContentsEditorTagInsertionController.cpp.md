# `src/app/models/editor/tags/ContentsEditorTagInsertionController.cpp`

## Responsibility

Implements common RAW tag insertion payload construction for editor commands.

## Current Behavior

- `tagNameForShortcutKey(...)` resolves `B`, `I`, `U`, and `E` shortcut keys to `bold`, `italic`, `underline`, and
  `highlight`.
- `tagNameForBodyShortcutKey(...)` resolves explicit body-tag shortcuts to `agenda`, `callout`, or `break`.
- `normalizedTagName(...)` canonicalizes known editor tags including inline formatting tags, `callout`, `agenda`,
  `task`, `resource`, and `break`.
- `buildTagInsertionPayload(...)` is the unified command entry point for formatting and body tags.
- `buildWrappedTagInsertionPayload(...)` clamps the current source selection and wraps it with one opening and one
  closing tag.
- A non-empty selection such as `Alpha` with `bold` becomes `<bold>Alpha</bold>`.
- A collapsed cursor inserts an empty pair such as `<bold></bold>` and returns a cursor position between the tags.
- A collapsed body-tag command inserts the canonical generated source, for example `<callout> </callout>` or
  `<agenda date="YYYY-MM-DD"><task done="false"> </task></agenda>`.
- The returned payload carries `nextSourceText`, `selectionStart`, `selectionEnd`, `cursorPosition`, `tagName`,
  `insertedSourceText`, and optional wrapped/replacement text fields.

## Boundary

The controller treats formatting tags and body tags as the same RAW insertion category. Specialized body-tag helpers
may still generate richer canonical source snippets, but they must feed the same next-source mutation path.
