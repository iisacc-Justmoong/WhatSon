# `src/app/file/hierarchy/resources`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/hierarchy/resources`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
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
