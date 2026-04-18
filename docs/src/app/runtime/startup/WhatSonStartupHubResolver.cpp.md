# `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`

## Implementation Notes
- Startup selection resolution now reads persisted hub state through `ISelectedHubStore`.
- Startup workspace mounting now requires an explicit persisted `.wshub` selection.
  If no valid hub is persisted, the resolver returns an unmounted selection so the app can route directly into
  onboarding instead of falling back to a blueprint sample hub.
- Mount and validation behavior for a real persisted hub are otherwise unchanged.
