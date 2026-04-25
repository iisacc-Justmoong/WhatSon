# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

## Role
`WhatSonStartupRuntimeCoordinator` orchestrates loading `.wshub` runtime data into the domain viewmodels.

## Interface Alignment
- Runtime targets now reuse `IWhatSonRuntimeParallelLoader::Targets`.
- The coordinator accepts a loader through `setParallelLoader(...)`.
- Deferred sidebar activation now depends on `IActiveHierarchySource`.
- Deferred startup hierarchy loads return a success flag so lifecycle prefetch tasks can surface failed follow-up domain
  loads without making sidebar activation callers handle the result.
