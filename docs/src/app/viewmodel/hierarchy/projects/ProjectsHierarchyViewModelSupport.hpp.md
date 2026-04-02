# `src/app/viewmodel/hierarchy/projects/ProjectsHierarchyViewModelSupport.hpp`

## Responsibility

This header owns project-specific hierarchy parsing, serialization, and sanitization helpers.

## Shared IO Delegation

`ProjectsSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

The duplicated inline `.wshub` traversal and UTF-8 loading logic was removed from this file.

## Shared Tree Delegation

`ProjectsSupport` also re-exports shared tree mutation helpers from `WhatSonHierarchyTreeItemSupport.hpp`:

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)` remains as a thin project-domain wrapper around the flat insertion helper.

## Domain Logic That Stays Local

The following helpers remain project-specific because they shape project hierarchy payloads rather than generic hub IO:

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- the equality and builder helpers defined later in the header

`sanitizeStringList(...)` and `extractDomainLabelsFromItems(...)` now delegate their dedup work to the
shared `QSet`-backed helpers instead of repeatedly scanning `QStringList`.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only project-domain rules should be added to this support header.
