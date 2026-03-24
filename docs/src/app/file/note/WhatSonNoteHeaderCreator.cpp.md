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
