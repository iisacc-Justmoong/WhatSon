# `src/app/models/hierarchy`

## Status
- Dedicated model shard for hierarchy models, controllers, parsers, stores, and hierarchy-specific support helpers.
- This directory was promoted from `src/app/models/hierarchy` so hierarchy composition no longer sits under the
  persistent file-storage domain.

## Scope
- Mirrored source directory: `src/app/models/hierarchy`
- Child directories: 9
- Child files: 11

## Child Directories
- `bookmarks`
- `event`
- `folders`
- `library`
- `preset`
- `progress`
- `projects`
- `resources`
- `tags`

## Child Files
- `CMakeLists.txt`
- `IHierarchyCapabilities.hpp`
- `IHierarchyController.cpp`
- `IHierarchyController.hpp`
- `WhatSonFolderDepthEntry.hpp`
- `WhatSonFolderIdentity.hpp`
- `WhatSonHierarchyModel.cpp`
- `WhatSonHierarchyModel.hpp`
- `WhatSonHierarchyIoSupport.hpp`
- `WhatSonHierarchyNoteRecordSupport.cpp`
- `WhatSonHierarchyNoteRecordSupport.hpp`
- `WhatSonHierarchyTreeItemSupport.hpp`
- `WhatSonNamedStringHierarchySupport.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Shared Expansion Policy
- Every hierarchy domain exposes the same `WhatSonHierarchyModel` as its LVRS-facing `itemModel`. Domain-specific
  controllers keep typed mutation state internally, then publish `depthItems()` node maps into the shared model.
- `LV.Hierarchy` must bind directly to that shared `WhatSonHierarchyModel`, not to a view-owned QML projection. The
  model exposes editable item flags, role writes, `moveRows(...)`, and an `items()` snapshot so LVRS can perform row
  reorder/edit interactions against the same contract for Library, Resources, and the other hierarchy domains.
- Domain `*HierarchyModel.hpp` files are item-struct/helper headers only. They must not declare another
  `QAbstractListModel` subclass.
- Concrete hierarchy controllers keep their domain-specific classes and stores, but their right-chevron
  expand/collapse mutation should delegate the common validation/state flip to `IHierarchyController`'s protected
  `setHierarchyItemExpanded(...)` helper.
- Single-row chevron expansion should call `WhatSonHierarchyModel::setItemExpanded(...)` so `LV.Hierarchy` sees a row
  role update rather than a full model reset.
- Bulk expand/collapse implementations should use `setAllHierarchyItemsExpanded(...)` when a domain exposes a dedicated
  bulk method. Domain code should only perform the follow-up sync/persistence callback.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/hierarchy``  ( `docs/src/app/models/hierarchy/README.md` )
- Location:  `docs/src/app/models/hierarchy`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the independent hierarchy model shard.
- Current responsibility: All hierarchy domains share exactly one `WhatSonHierarchyModel` as the display item model. The common validation/state flip for right-chevron expand/collapse is owned by `IHierarchyController` protected helper, and single row update is processed by `WhatSonHierarchyModel::setItemExpanded(...)`. `LV.Hierarchy` must bind directly to this shared model, and does not place a QML view-owned projection array in between.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- Current: Hierarchy implementation is placed in  `src/app/models/hierarchy`  instead of  `src/app/models/hierarchy`
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
