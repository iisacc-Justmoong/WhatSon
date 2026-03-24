# `src/app/file/note/WhatSonNoteHeaderStore.cpp`

## Responsibility

This implementation normalizes note-header metadata before it is consumed by the rest of the
application.

## Folder Binding Normalization

The folder-related setters do more than plain assignment:

- trim incoming values,
- normalize UUIDs through `WhatSon::FolderIdentity`,
- keep folder paths and UUIDs aligned by index,
- deduplicate equivalent bindings,
- clear invalid UUIDs instead of storing malformed identifiers.

`setFolders(...)` remains for legacy callers, but it routes through the shared binding logic so new
invariants are not bypassed.

## Why This Matters

The library hierarchy now filters and rewrites notes by folder UUID. If the store allowed path and
UUID arrays to drift apart, the application could silently reassign a note to the wrong folder after
a rename or drag-and-drop mutation.
