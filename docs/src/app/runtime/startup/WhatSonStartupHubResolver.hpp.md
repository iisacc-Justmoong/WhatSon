# `src/app/runtime/startup/WhatSonStartupHubResolver.hpp`

## Role
This header defines helper functions that resolve the startup hub package and any required mount/access state.

## Interface Alignment
- `resolveStartupHubSelection(...)` accepts `ISelectedHubStore` together with `WhatSonHubMountValidator` and owns the
  startup-source policy.
- `StartupHubSelection` reports the resolved source (`PersistedSelection`) together with an optional failure message
  when the persisted startup hub cannot be mounted.
- Startup hub resolution is no longer tied to the concrete settings store implementation and no longer contains any
  packaged fallback path.
