# `src/app/main.cpp`

## Runtime Wiring Notes
- `WhatSonRuntimeParallelLoader` is now instantiated in `main.cpp` and injected into `WhatSonStartupRuntimeCoordinator` through `setParallelLoader(...)`.
- Calendar and system calendar stores continue to be instantiated concretely here, but downstream collaborators now consume interface types.
- `main.cpp` now also keeps the concrete `CalendarBoardStore` synchronized with the currently loaded hub path, so local
  note mutations and sync reloads refresh calendar note projections without replacing manual calendar entries.
- Hub selection wiring now updates sync/import state directly without a background write-lease heartbeat timer.
- This keeps the application bootstrap as the composition root for concrete object selection.
