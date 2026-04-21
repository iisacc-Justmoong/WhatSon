# `src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyStore.cpp`

## Responsibility

This file owns the persisted `Folders.wsfolders` read/write boundary for generic folder hierarchies.
Its main job after the UUID migration is to guarantee that every stored row carries a valid stable
folder UUID.

## UUID Handling

- Rows loaded from disk are normalized before the caller sees them.
- If a row has no UUID, or if the value is malformed, the store synthesizes a new 64-character
  UUID before the data is persisted again.
- The store does not treat path changes as identity changes. `id` may change during a rename, but
  `uuid` is preserved.

## Persistence Role

The store sits above the parser/creator pair:

1. Parse raw JSON-like `.wsfolders` content into `WhatSonFolderDepthEntry` rows.
2. Sanitize each row, especially the `uuid` field.
3. Serialize the sanitized list back when callers save the hierarchy.

## Path Normalization

- `id` is normalized through the shared folder-path escape rules before the caller sees it.
- If only `label` is available, the store synthesizes a canonical one-segment `id` from that label, escaping any
  literal `/` so the folder still stays one node.
- If only `id` is available, the store derives `label` from the decoded leaf segment instead of exposing persisted
  escape markers directly.

## Why This Matters

The rest of the library pipeline now depends on folder UUIDs to reconnect note headers to the
renamed or moved folder tree. If the store allowed invalid or missing UUIDs to leak through,
runtime filtering would fall back to path comparisons and reintroduce the original bug.
