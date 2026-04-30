# `src/app/models/file/hierarchy/WhatSonHierarchyIoSupport.hpp`

## Responsibility

This header centralizes the filesystem helpers that were previously duplicated across hierarchy domains.

- normalize hub-relative and absolute paths through `WhatSon::HubPath`
- resolve `.wscontents` directories from an unpacked `.wshub` bundle
- read UTF-8 files with shared debug tracing and error reporting

## Public Helpers

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

These helpers are header-only and live in `WhatSon::Hierarchy::IoSupport` so every hierarchy domain can reuse the same implementation without adding a new runtime dependency.

## Performance Notes

`deduplicateStringsPreservingOrder(...)` and `extractDistinctLabelsFromItems(...)` replace the older
pattern of calling `QStringList::contains(...)` inside loops.

The new helpers keep insertion order but track seen values with `QSet`, which removes the repeated
O(n^2) scans from hierarchy sanitization and label extraction paths.

## Error Contract

`resolveContentsDirectories(...)` and `readUtf8File(...)` return `false` on failure and optionally populate `errorMessage`.

Failure cases covered here:

- null output pointers
- empty or missing `.wshub` paths
- malformed unpacked hub directories
- missing `.wscontents` directories
- unreadable text files

## Consumers

This helper is re-exported by:

- `LibraryHierarchyControllerSupport.hpp`
- `ProjectsHierarchyControllerSupport.hpp`
- `BookmarksHierarchyControllerSupport.hpp`
- `ProgressHierarchyControllerSupport.hpp`
- `EventHierarchyControllerSupport.hpp`
- `PresetHierarchyControllerSupport.hpp`
- `ResourcesHierarchyControllerSupport.hpp`
- `TagsHierarchyControllerSupport.hpp`

Those domain headers keep their namespace-level API stable via `using` declarations while domain-specific parsing and serialization stay local to each feature.
