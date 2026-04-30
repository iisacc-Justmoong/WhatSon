# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

## Role
`WhatSonStartupRuntimeCoordinator` orchestrates loading `.wshub` runtime data into the domain controllers.

## Interface Alignment
- Runtime targets now reuse `IWhatSonRuntimeParallelLoader::Targets`.
- The coordinator accepts a loader through `setParallelLoader(...)`.
- The public startup surface is reduced to normal full hub loading plus resource-domain reloads; persisted startup
  scheduling is owned by `main.cpp` after the workspace root is visible.
- The old deferred sidebar-activation bootstrap path was removed so startup has one runtime load route instead of a
  pre-window partial load plus follow-up hierarchy loads.
