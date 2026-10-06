# `src/app/models/hierarchy/resources`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/hierarchy/resources`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ResourcesHierarchyController.cpp`
- `ResourcesHierarchyController.hpp`
- `ResourcesHierarchyControllerSupport.hpp`
- `ResourcesHierarchyModel.hpp`
- `ResourcesListModel.cpp`
- `ResourcesListModel.hpp`
- `WhatSonResourcePackageSupport.hpp`
- `WhatSonResourcesHierarchyCreator.cpp`
- `WhatSonResourcesHierarchyCreator.hpp`
- `WhatSonResourcesHierarchyParser.cpp`
- `WhatSonResourcesHierarchyParser.hpp`
- `WhatSonResourcesHierarchyStore.cpp`
- `WhatSonResourcesHierarchyStore.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes

- The singular `.wsresource` package contract now includes three package-local artifacts:
  - the original imported asset
  - `resource.xml`
  - `annotation.png` as a transparent bitmap canvas reserved for future resource-editor sketch/annotation overlay work
- `WhatSonResourcePackageSupport.hpp` owns both sides of that package contract:
  - metadata XML creation/parsing for the new `annotationPath`
  - empty annotation bitmap generation/writing for package creation paths
- `ResourcesHierarchyModel.hpp` only declares the resources hierarchy item struct and icon helper.
- `ResourcesHierarchyController::syncModel()` still publishes `depthItems()` into the shared
  `WhatSonHierarchyModel`, preserving the common controller/model contract used by sidebar providers.
- The Resources sidebar render path intentionally consumes the controller's `hierarchyNodes` snapshot, matching the
  older LVRS hierarchy behavior. A chevron click therefore toggles only the clicked LVRS item instead of committing
  `setItemExpanded(...)` back through the shared model and triggering the broad `QAbstractItemModel` invalidation path.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/hierarchy/resources``  ( `docs/src/app/models/hierarchy/resources/README.md` )
- Location:  `docs/src/app/models/hierarchy/resources`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Current rule: The resource hierarchy controller continues to publish to the common `WhatSonHierarchyModel` from `depthItems()`. However, the sidebar display path passes the controller's `hierarchyNodes` snapshot to `LV.Hierarchy` as in the past method. Therefore, single click of chevron does not return to `setItemExpanded(...)` of the common model but ends with LVRS row-local toggle.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
