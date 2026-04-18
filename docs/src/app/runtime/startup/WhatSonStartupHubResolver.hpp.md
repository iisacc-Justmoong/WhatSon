# `src/app/runtime/startup/WhatSonStartupHubResolver.hpp`

## Role
This header defines helper functions that resolve the startup hub package and any required mount/access state.

## Interface Alignment
- `resolveStartupHubSelection(...)` now accepts `ISelectedHubStore`.
- Startup hub resolution is no longer tied to the concrete settings store implementation.
- Startup selection now models only explicitly persisted hub choices.
  Missing or invalid persisted selections intentionally surface as `mounted == false` so onboarding remains the only
  startup fallback.
