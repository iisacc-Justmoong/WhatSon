# `src/app/runtime/threading/WhatSonRuntimeParallelLoader.cpp`

## Responsibility

`WhatSonRuntimeParallelLoader.cpp` fans out domain snapshot work onto worker threads and then applies
those immutable snapshot payloads back onto the main-thread viewmodels.

## Requested Domain Selection

The loader now accepts an explicit requested-domain mask. Startup can therefore load only the
critical first-frame domains while leaving only low-priority hierarchies for deferred follow-up
work.

## Shared Library Snapshot Rule

When both the library and bookmarks domains are present, the loader now indexes the library domain
once and derives bookmarks from that shared library snapshot.

This removes the previous duplicate `.wshub` traversal where the bookmarks task reparsed the same
library note set independently.

## Failure Behavior

- If the library snapshot fails, the derived bookmarks result fails with the same error.
- If the library domain is absent, the loader falls back to the standalone bookmarks snapshot path.
- The loader now stages every requested domain, including `hub.runtime`, and only applies snapshots
  back into live viewmodels/runtime stores when every requested domain succeeded.
- If any requested domain fails, the loader returns failure without partially mutating the current
  runtime state.
