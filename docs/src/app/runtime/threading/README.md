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
- LVRS `BootstrapParallel` now owns the worker pool for requested domain loads. Live controllers and the shared
  `WhatSonHubRuntimeStore` are still updated only after the entire request succeeds.
- Persisted startup no longer requests any runtime domains before the workspace root is visible. `main.cpp` schedules
  the normal full runtime load as an LVRS `QmlAppLifecycleStage::AfterFirstIdle` bootstrap task.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/runtime/threading``  ( `docs/src/app/runtime/threading/README.md` )
- Location:  `docs/src/app/runtime/threading`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
