# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp`

## Implementation Notes
- Loader use is now interface-driven through `m_parallelLoader`.
- The coordinator refuses to load when no loader implementation is injected.
- Startup no longer has a separate pre-window runtime load or deferred hierarchy bootstrap path. `main.cpp` mounts the
  persisted hub for routing, then schedules the normal full runtime load after the first workspace idle turn.
- Hub-runtime side effects (`libraryViewModel.setHubStore(...)` and tag-depth propagation) are now
  applied only after the entire requested load succeeds, so a failed domain load cannot partially
  retarget the live runtime session.
