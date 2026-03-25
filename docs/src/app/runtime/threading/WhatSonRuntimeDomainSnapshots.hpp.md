# `src/app/runtime/threading/WhatSonRuntimeDomainSnapshots.hpp`

## Responsibility

`WhatSonRuntimeDomainSnapshots.hpp` declares the worker-thread payload types used during runtime
bootstrap and hub reload.

## Notable API

- `loadLibrary(...)`: indexes the library domain and returns the note records, smart buckets, and
  parsed folder hierarchy needed by `LibraryHierarchyViewModel`.
- `buildBookmarks(...)`: derives the bookmarks snapshot from an already indexed library note set.
- `loadBookmarks(...)`: fallback path used only when the bookmarks domain is requested without the
  library domain.

## Architectural Note

The header now encodes the "index once, derive again" rule for bookmarks. `buildBookmarks(...)`
exists so `WhatSonRuntimeParallelLoader` can reuse the shared library snapshot instead of forcing a
second hub traversal.
