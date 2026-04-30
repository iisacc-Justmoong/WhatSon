# `src/app/models/file/hierarchy/WhatSonFolderIdentity.hpp`

## Responsibility

This header defines the shared folder-UUID rules used across parsing, persistence, and runtime
hierarchy logic.

## Public Contract

- `kUuidLength` is fixed at `64`.
- `normalizeFolderUuid(...)` trims input and rejects values that are not exactly 64 alphanumeric
  characters.
- `isValidFolderUuid(...)` is the boolean convenience wrapper over normalization.
- `createFolderUuid()` generates a random 64-character identifier from upper-case letters,
  lower-case letters, and digits.

## Design Notes

- The format is intentionally filesystem-safe and XML-safe, so it can move between `.wsfolders`,
  `.wsnhead`, and in-memory Qt strings without extra escaping rules.
- The helper is header-only because it has no external state and is needed in many low-level files.
- The generated UUID is not a RFC-4122 value. It is a project-specific stable folder key.

## Main Call Sites

- `WhatSonFoldersHierarchyParser.cpp` and `WhatSonFoldersHierarchyStore.cpp` sanitize or backfill
  missing UUIDs.
- `WhatSonNoteHeaderStore.cpp` normalizes folder bindings written into note headers.
- `LibraryHierarchyController.cpp` and
  `WhatSonLibraryFolderHierarchyMutationService.cpp` use the helper to keep runtime identity stable
  across rename and move operations.
