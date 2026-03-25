# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp`

## Responsibility

`WhatSonRuntimeParallelLoader.cpp` fans out domain snapshot work onto worker threads and then applies
those immutable snapshot payloads back onto the main-thread viewmodels.

## Shared Library Snapshot Rule

When both the library and bookmarks domains are present, the loader now indexes the library domain
once and derives bookmarks from that shared library snapshot.

This removes the previous duplicate `.wshub` traversal where the bookmarks task reparsed the same
library note set independently.

## Failure Behavior

- If the library snapshot fails, the derived bookmarks result fails with the same error.
- If the library domain is absent, the loader falls back to the standalone bookmarks snapshot path.
