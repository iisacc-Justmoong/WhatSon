# `src/app/models/hierarchy/WhatSonHierarchyModel.cpp`

## Purpose
- Implements the shared `QAbstractListModel` that backs all hierarchy item-model surfaces.
- Keeps the UI-facing model close to the `LV.Hierarchy` node map instead of projecting each domain through a separate
  model class.

## Behavior
- `setItems(...)` stores sanitized node maps and emits a reset only for full node replacement.
- `setItemExpanded(...)` changes one row's `expanded` value and emits `dataChanged` for `ExpandedRole`.
- Validation is intentionally generic: negative depth is corrected, labels are trimmed, and accent rows below depth 0
  are normalized out.

## Testing
- Covered by `hierarchyItemModel_usesSharedLvrsModelContract`.
- Controller adoption is locked by `hierarchyControllers_exposeSharedLvrsHierarchyModel`.
