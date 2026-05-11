# `src/app/models/file/note/hub/WhatSonHubNoteMutationSupport.hpp`

## Role
`WhatSonHubNoteMutationSupport.hpp` exposes narrow note-domain helpers used by note CRUD, versioning, package
normalization, and library-record synchronization.

## Public API
- `currentNoteTimestamp()` returns the local note timestamp format used by header mutations.
- `indexOfNoteRecordById(...)` and `createUniqueNoteId(...)` support library note create/delete flows.
- `ensureDirectoryPath(...)`, `readUtf8File(...)`, `writeUtf8File(...)`, `removeFilePath(...)`,
  `removeDirectoryPath(...)`, and `pathExists(...)` provide the note-domain filesystem surface after the shared IO object
  layer was removed.
- `resolveNoteHeaderPath(...)` resolves the materialized `.wsnhead` path for a library record.
- `syncNoteRecordFromDocument(...)` copies persisted note document fields back into an in-memory record.

## Dependency Notes
- The helper is a namespace-only C++ surface and exposes no QObject or QML binding.
- It depends on `WhatSonLocalNoteDocument.hpp`, not `WhatSonLocalNoteFileStore.hpp`, so file-store code can reuse the
  helper without introducing a header cycle.
