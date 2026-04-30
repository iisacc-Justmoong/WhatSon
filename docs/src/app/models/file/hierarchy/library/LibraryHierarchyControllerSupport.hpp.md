# `src/app/models/file/hierarchy/library/LibraryHierarchyControllerSupport.hpp`

## Responsibility

This header now acts as the library-domain facade for shared hierarchy IO helpers.

## Shared IO Delegation

`LibrarySupport` re-exports these helpers from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`

This keeps existing call sites stable while removing duplicated `.wshub` and UTF-8 file logic from the library domain.

## Scope

The library support header no longer owns an inline copy of the shared filesystem implementation.
Any future library-specific parsing or transformation logic should stay in the library domain, while generic hub IO remains in `WhatSon::Hierarchy::IoSupport`.
