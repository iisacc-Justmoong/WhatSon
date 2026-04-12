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
- Worker threads may still parse and stage hub/runtime data independently, but live viewmodels and
  the shared `WhatSonHubRuntimeStore` are updated only after the entire request succeeds.
