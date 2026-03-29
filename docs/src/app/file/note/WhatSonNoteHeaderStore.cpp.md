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

## Progress State

- The store still treats non-negative integers as concrete progress enum values.
- `-1` is now a valid in-memory sentinel for “no progress selected”, so detail-panel clear actions can round-trip an empty progress field through `.wsnhead` without being forced back to `0`.
- `setProgressEnums(...)` now preserves the raw enum-label list from `.wsnhead`, so later writes can
  serialize the same progress taxonomy instead of falling back to the default product labels.
