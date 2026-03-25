# `src/app/file/hub`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/file/hub`
- Child directories: 0
- Child files: 16

## Child Directories
- No child directories.

## Child Files
- `WhatSonHubCreator.cpp`
- `WhatSonHubCreator.hpp`
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
- `WhatSonHubWriteLease.hpp`

## Notes
- `WhatSonHubCreator` is responsible for initial hub package materialization, including `.whatson/hub.json`.
- `WhatSonHubWriteLease.hpp` owns runtime write-lock coordination after a hub already exists.
