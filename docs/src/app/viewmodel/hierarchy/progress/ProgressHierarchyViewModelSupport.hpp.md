# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp`

## Responsibility

This header owns progress-specific hierarchy parsing, serialization, and sanitization helpers.

## Shared IO Delegation

`ProgressSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

The duplicated inline `.wshub` traversal and UTF-8 loading logic was removed from this file.

## Shared Tree Delegation

`ProgressSupport` also re-exports shared tree mutation helpers from `WhatSonHierarchyTreeItemSupport.hpp`:

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)` remains as a thin wrapper around the nested insertion helper without forced parent expansion.

## Domain Logic That Stays Local

The following helpers remain progress-specific because they shape progress hierarchy payloads:

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- the equality and builder helpers defined later in the header

`sanitizeStringList(...)` and `extractDomainLabelsFromItems(...)` now use the shared `QSet`-based
ordered dedup helpers so large progress state lists no longer degrade with repeated `contains(...)` scans.

## Current Build Rules

- `sanitizeStringList(...)` trims and deduplicates raw state labels.
- `buildSupportedTypeItems(...)` returns the product-defined ten-row progress taxonomy:
  `First draft`, `Modified draft`, `In Progress`, `Pending`, `Reviewing`, `Waiting for approval`,
  `Done`, `Lagacy`, `Archived`, and `Delete review`.
- The first four rows intentionally keep `showChevron=true` to preserve the current LVRS/Figma
  interaction contract even without child rows.
- Serialized depth items include the row label and structural metadata needed by the shared
  hierarchy QML layer, while the viewmodel adds `progressValue` and `itemId` on top of that output.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only progress-domain rules should be added to this support header.
