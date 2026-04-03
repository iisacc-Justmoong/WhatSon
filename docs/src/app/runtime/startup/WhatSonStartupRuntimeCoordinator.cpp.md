# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.cpp`

## Implementation Notes
- Loader use is now interface-driven through `m_parallelLoader`.
- The coordinator refuses to load when no loader implementation is injected.
- Startup hub loading no longer acquires or refreshes a hub write lease during runtime bootstrap.
- Deferred startup loading still uses the same hierarchy index rules and domain-specific load paths.
