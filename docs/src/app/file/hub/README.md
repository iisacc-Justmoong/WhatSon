# `src/app/file/hub`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/hub`
- Child directories: 0
- Child files: 17

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubCreator.cpp`
- `WhatSonHubCreator.hpp`
- `WhatSonHubPackager.cpp`
- `WhatSonHubPackager.hpp`
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
- `WhatSonHubPackager` owns `.wshub` package-root creation and Apple package-presentation hints.
- `WhatSonHubCreator` now owns only scaffold writes inside an already materialized package root, including `.whatson/hub.json`.
- Runtime hub writes no longer depend on a hub-level write-lease side channel.
