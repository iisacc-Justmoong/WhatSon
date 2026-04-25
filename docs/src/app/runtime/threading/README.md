# `src/app/runtime/threading`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/runtime/threading`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `WhatSonRuntimeDomainSnapshots.cpp`
- `WhatSonRuntimeDomainSnapshots.hpp`
- `WhatSonRuntimeParallelLoader.cpp`
- `WhatSonRuntimeParallelLoader.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Recent Notes
- Startup/reload snapshot application is now all-or-nothing across the requested domain set.
- LVRS `BootstrapParallel` now owns the worker pool for requested domain loads. Live viewmodels and the shared
  `WhatSonHubRuntimeStore` are still updated only after the entire request succeeds.
- Deferred startup Event/Preset hierarchy prefetch is scheduled by `main.cpp` as an LVRS
  `QmlAppLifecycleStage::AfterFirstIdle` bootstrap task after the workspace root is visible.
