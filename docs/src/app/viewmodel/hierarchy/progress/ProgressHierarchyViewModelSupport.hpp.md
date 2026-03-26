# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp`

## Responsibility

This support header contains the utility functions used to load, sanitize, serialize, and rebuild
progress hierarchy rows.

## Current Build Rules

- `sanitizeStringList(...)` trims and deduplicates raw state labels.
- `buildSupportedTypeItems(...)` no longer fabricates a fixed ten-row taxonomy. It now produces one
  flat `ProgressHierarchyItem` per sanitized progress state defined by `Progress.wsprogress`.
- Serialized depth items include the row label and structural metadata needed by the shared
  hierarchy QML layer, while the viewmodel adds `progressValue` and `itemId` on top of that output.
