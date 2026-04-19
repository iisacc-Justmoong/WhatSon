# `src/app/runtime/startup/WhatSonStartupHubResolver.cpp`

## Implementation Notes
- Startup selection resolution now reads persisted hub state through `ISelectedHubStore`.
- Startup selection no longer retries a `blueprint/*.wshub` fallback when the persisted selection is empty or cannot be mounted.
- On iOS, bookmark restore now promotes the restored path back to a concrete `.wshub` package before validating it, and
  falls back to the persisted selection URL when the old local path is no longer stable.
- The iOS-restored package validation and the generic normalized-path validation stay as separate local checks so the
  resolver can validate both branches independently before it returns a mountable `.wshub` path.
