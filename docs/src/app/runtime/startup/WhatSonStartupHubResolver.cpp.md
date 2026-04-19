# `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`

## Implementation Notes
- Startup selection resolution now reads persisted hub state through `ISelectedHubStore`.
- Startup selection no longer retries a `blueprint/*.wshub` fallback when the persisted selection is empty or cannot be mounted.
- Mount and validation behavior for the persisted selection are otherwise unchanged.
