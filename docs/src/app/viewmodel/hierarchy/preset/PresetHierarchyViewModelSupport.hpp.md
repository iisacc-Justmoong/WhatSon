# `src/app/viewmodel/hierarchy/preset/PresetHierarchyViewModelSupport.hpp`

## Responsibility

This header owns preset-specific hierarchy parsing, serialization, and sanitization helpers.

## Shared IO Delegation

`PresetSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

The duplicated inline `.wshub` traversal and UTF-8 loading logic was removed from this file.

## Shared Tree Delegation

`PresetSupport` also re-exports shared tree mutation helpers from `WhatSonHierarchyTreeItemSupport.hpp`:

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)` remains as a thin wrapper around the nested insertion helper with parent expansion enabled.

## Domain Logic That Stays Local

The following helpers remain preset-specific because they shape preset hierarchy payloads:

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- the equality and builder helpers defined later in the header

`sanitizeStringList(...)` and `extractDomainLabelsFromItems(...)` now use the shared `QSet`-based
ordered dedup helpers so preset hierarchy extraction avoids repeated linear duplicate scans.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only preset-domain rules should be added to this support header.
