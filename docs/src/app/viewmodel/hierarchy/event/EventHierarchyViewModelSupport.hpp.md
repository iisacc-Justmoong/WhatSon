# `src/app/viewmodel/hierarchy/event/EventHierarchyViewModelSupport.hpp`

## Responsibility

This header owns event-specific hierarchy parsing, serialization, and sanitization helpers.

## Shared IO Delegation

`EventSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

The duplicated inline `.wshub` traversal and UTF-8 loading logic was removed from this file.

## Shared Tree Delegation

`EventSupport` also re-exports shared tree mutation helpers from `WhatSonHierarchyTreeItemSupport.hpp`:

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)` remains as a thin wrapper around the nested insertion helper with parent expansion disabled.
Folder creation must not change expansion state unless the user explicitly expands or collapses a row.

## Domain Logic That Stays Local

The following helpers remain event-specific because they define how event rows are interpreted and rebuilt:

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- the equality and builder helpers defined later in the header

`sanitizeStringList(...)` and `extractDomainLabelsFromItems(...)` now use the shared `QSet`-based
ordered dedup helpers so event reloads no longer pay repeated `QStringList::contains(...)` scans.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only event-domain rules should be added to this support header.
