# `src/app/models/file/hierarchy/bookmarks/BookmarksHierarchyControllerSupport.hpp`

## Responsibility

This header owns bookmark-specific hierarchy parsing, serialization, and sanitization helpers.

## Shared IO Delegation

`BookmarksSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

The duplicated inline `.wshub` traversal and UTF-8 loading logic was removed from this file.

## Shared Tree Delegation

`BookmarksSupport` also re-exports shared tree mutation helpers from `WhatSonHierarchyTreeItemSupport.hpp`:

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)` remains as a thin wrapper around the nested insertion helper without forced parent expansion.

## Domain Logic That Stays Local

The following helpers remain bookmark-specific because they shape bookmark hierarchy payloads:

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- the equality and builder helpers defined later in the header

`sanitizeStringList(...)` and `extractDomainLabelsFromItems(...)` now use the shared `QSet`-based
ordered dedup helpers so bookmark hierarchy rebuilds avoid repeated linear duplicate checks.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only bookmark-domain rules should be added to this support header.
