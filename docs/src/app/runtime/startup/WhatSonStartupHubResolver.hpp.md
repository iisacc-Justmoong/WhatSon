# `src/app/runtime/startup/WhatSonStartupHubResolver.hpp`

## Role
This header defines helper functions that resolve the startup hub package and any required mount/access state.

## Interface Alignment
- `resolveStartupHubSelection(...)` now accepts `ISelectedHubStore`.
- Startup hub resolution is no longer tied to the concrete settings store implementation.
