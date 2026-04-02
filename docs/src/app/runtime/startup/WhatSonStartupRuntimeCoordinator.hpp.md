# `src/app/runtime/startup/WhatSonStartupRuntimeCoordinator.hpp`

## Role
`WhatSonStartupRuntimeCoordinator` orchestrates loading `.wshub` runtime data into the domain viewmodels.

## Interface Alignment
- Runtime targets now reuse `IWhatSonRuntimeParallelLoader::Targets`.
- The coordinator accepts a loader through `setParallelLoader(...)`.
- Deferred sidebar activation now depends on `IActiveHierarchySource`.
