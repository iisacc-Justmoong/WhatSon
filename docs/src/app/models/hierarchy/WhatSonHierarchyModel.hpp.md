# `src/app/models/hierarchy/WhatSonHierarchyModel.hpp`

## Purpose
- Declares the single LVRS-facing hierarchy item model used by every hierarchy domain.
- Domain controllers still own their typed mutation state, parser/store, and selection policy, but they expose
  `WhatSonHierarchyModel* itemModel` for the common `LV.Hierarchy` row contract.

## Contract
- `setItems(...)` accepts the `QVariantList` node shape already returned by each controller's `depthItems()`.
- Role names mirror the keys consumed by `LV.Hierarchy` and sidebar bridges: `label`, `depth`, `expanded`,
  `showChevron`, `key`, `itemKey`, `iconName`, `count`, drag flags, and resource/progress metadata.
- `setItemExpanded(...)` updates only the `ExpandedRole` for the changed row and emits `dataChanged`, avoiding a full
  model reset for single chevron fold/unfold changes.

## Notes
- Domain-specific `*HierarchyModel.hpp` files now only keep typed item structs and icon helpers for controller internals.
- New hierarchy domains must feed this model from their LVRS node serialization rather than introducing another
  `QAbstractListModel` subclass.
