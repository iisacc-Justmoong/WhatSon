# `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModelSupport.hpp`

## Responsibility

This header now acts as the tags-domain facade for shared hierarchy IO helpers.

## Shared IO Delegation

`TagsSupport` re-exports these helpers from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`

The tags domain keeps its public helper namespace, but the actual `.wshub` discovery and UTF-8 file loading logic is implemented once in the shared hierarchy IO helper.

## Scope

Generic filesystem logic belongs to `WhatSon::Hierarchy::IoSupport`.
Any tags-specific parsing or runtime shaping should stay in the tags domain instead of reintroducing another copy of the shared IO code.
