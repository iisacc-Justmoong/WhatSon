# `src/app/viewmodel/hierarchy/progress/ProgressHierarchyViewModelSupport.hpp`

## Responsibility

This support header contains the utility functions used to load, sanitize, serialize, and rebuild
progress hierarchy rows.

## Current Build Rules

- `sanitizeStringList(...)` trims and deduplicates raw state labels.
- `buildSupportedTypeItems(...)` returns the product-defined ten-row progress taxonomy:
  `First draft`, `Modified draft`, `In Progress`, `Pending`, `Reviewing`, `Waiting for approval`,
  `Done`, `Lagacy`, `Archived`, and `Delete review`.
- The first four rows intentionally keep `showChevron=true` to preserve the current LVRS/Figma
  interaction contract even without child rows.
- Serialized depth items include the row label and structural metadata needed by the shared
  hierarchy QML layer, while the viewmodel adds `progressValue` and `itemId` on top of that output.
