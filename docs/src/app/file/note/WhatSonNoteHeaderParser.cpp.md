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
