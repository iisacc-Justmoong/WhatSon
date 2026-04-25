# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp`

## Implementation Notes
- Loader use is now interface-driven through `m_parallelLoader`.
- The coordinator refuses to load when no loader implementation is injected.
- Startup hub loading no longer acquires or refreshes a hub write lease during runtime bootstrap.
- Deferred startup loading still uses the same hierarchy index rules and domain-specific load paths.
- `ensureDeferredStartupHierarchyLoaded(...)` now returns whether the requested deferred domain loaded or was already
  satisfied, allowing the LVRS `AfterFirstIdle` prefetch task to report non-fatal lifecycle task failures.
- Hub-runtime side effects (`libraryViewModel.setHubStore(...)` and tag-depth propagation) are now
  applied only after the entire requested load succeeds, so a failed domain load cannot partially
  retarget the live runtime session.
