# `src/app/file/note/WhatSonNoteHeaderParser.cpp`

## Responsibility

This parser reads `.wsnhead` XML and populates `WhatSonNoteHeaderStore`.

## Folder Parsing Rules

- `<folder>Path</folder>` remains valid legacy input.
- `<folder uuid="...">Path</folder>` is the modern form.
- Folder UUIDs are extracted and normalized together with the visible folder paths.
- Parsed bindings are stored through `setFolderBindings(...)`, not by mutating path and UUID lists
  separately.

## Migration Behavior

Older notes without folder UUIDs continue to load because the parser treats the path as a readable
binding and leaves the UUID empty. Later rewrite paths, such as folder rename or note reassignment,
can then upgrade the note header to the new attribute-based format.

## Progress Parsing Rules

- Numeric `<progress>` content still maps directly to the stored enum integer.
- Enum-label progress content still resolves through the `enums={...}` attribute.
- The `enums={...}` attribute is now stored on `WhatSonNoteHeaderStore` through
  `setProgressEnums(...)`, not discarded after integer resolution.
- An empty `<progress>` element is now treated as an explicit cleared state and parses to `-1` instead of silently falling back to `0`.
- Unknown `<progress>` text that cannot be resolved as a numeric value or enum label now also
  resolves to `-1` (`No progress`) instead of coercing to `0` (`First draft`).

## File Statistics Parsing Rules

- The parser now reads the `.wsnhead <fileStat>...</fileStat>` block.
- Missing statistic tags default to `0` for backwards compatibility with older note headers.
- Negative or invalid numeric payloads are normalized back to `0`.
- Existing top-level metadata (`project`, `folders`, `tags`, `created`, `lastModified`, `lastOpened`) remains the
  source of truth for the non-numeric Figma rows; the `fileStat` block only stores explicit numeric
  counters.

## Activity Parsing Rules

- `<lastOpened>` is optional and parses to an empty string for older note headers.
- The parser stores `lastOpened` on `WhatSonNoteHeaderStore` so inactivity sensors can reason from RAW note headers
  without loading body/editor projections.
