# `src/app/file/note/WhatSonNoteHeaderCreator.cpp`

## Responsibility

This file serializes `WhatSonNoteHeaderStore` back into `.wsnhead` XML.

## Folder Serialization Rules

- Each folder binding is emitted as a `<folder>` element.
- When a valid UUID is present at the same index, the serializer writes it as `uuid="..."`.
- The element body keeps the readable folder path.

This dual representation lets humans inspect a note header while the application still relies on a
stable machine identity.

## Output Expectations

Callers should arrive with normalized data. The creator does not try to infer missing UUIDs from
paths; that responsibility belongs to higher-level services that know the current folder tree.

## Scaffold Path Policy

- `requiredRelativePaths()` is intentionally empty.
- Header creation no longer requires or emits `.meta` subdirectories inside `.wsnote` packages.

## Progress Serialization Rules

- The serializer now preserves `WhatSonNoteHeaderStore::progressEnums()` in the `<progress enums="...">`
  attribute, so custom note-local progress labels round-trip without being rewritten to the default
  `Ready/Pending/InProgress/Done` set.
- Non-negative progress values are serialized as the `<progress>` element body.
- A cleared progress state (`-1`) is serialized as an empty `<progress ...></progress>` element so the parser can round-trip the absence of progress instead of coercing it back to `0`.
