# `src/app/models/file/hierarchy/folders/WhatSonFoldersHierarchyCreator.cpp`

## Responsibility

This file serializes `WhatSonFolderDepthEntry` rows back into the persisted `Folders.wsfolders`
document.

## UUID Persistence Rules

- Every serialized folder node includes a `uuid` field.
- If the caller forgot to provide a UUID, the creator generates one before writing.
- The creator preserves the separation between:
  - `id`: the current readable full path
  - `uuid`: the stable identity used by runtime folder relationships

## Output Invariant

After this creator runs, a modern `.wsfolders` file is expected to keep enough information for both:

- reconstructing the visible tree from `id`, `label`, and `depth`
- reconnecting note headers and selection state through `uuid` even when a parent path changes

## Main Collaborators

- `WhatSonFoldersHierarchyStore.cpp`: sanitizes rows before save.
- `WhatSonHubParser.cpp`: exposes folder entries during runtime bootstrap.
- `LibraryHierarchyController.cpp`: persists edited hierarchy rows through this creator.
