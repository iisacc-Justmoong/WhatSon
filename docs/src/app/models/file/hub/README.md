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

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/file/hub``  ( `docs/src/app/models/file/hub/README.md` )
- Location:  `docs/src/app/models/file/hub`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
