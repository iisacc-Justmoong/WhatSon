# `src/app/models/file/hub`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/file/hub`
- Child directories: 0
- Child files: 17

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubCreator.cpp`
- `WhatSonHubCreator.hpp`
- `WhatSonHubMountValidator.cpp`
- `WhatSonHubMountValidator.hpp`
- `WhatSonHubParser.cpp`
- `WhatSonHubParser.hpp`
- `WhatSonHubPathUtils.hpp`
- `WhatSonHubPlacement.cpp`
- `WhatSonHubPlacement.hpp`
- `WhatSonHubPlacementStore.cpp`
- `WhatSonHubPlacementStore.hpp`
- `WhatSonHubRuntimeStore.cpp`
- `WhatSonHubRuntimeStore.hpp`
- `WhatSonHubStat.cpp`
- `WhatSonHubStat.hpp`
- `WhatSonHubStore.cpp`
- `WhatSonHubStore.hpp`

## Notes
- `WhatSonHubCreator` is responsible for initial hub package materialization, including `.whatson/hub.json`.
- `WhatSonHubMountValidator` now owns the lightweight mount/access + hub-structure preflight shared by startup and
  onboarding.
- Runtime hub writes no longer depend on a hub-level write-lease side channel.
