# `src/app/store/hub/SelectedHubStore.hpp`

## Role
`SelectedHubStore` is the default settings-backed implementation of `ISelectedHubStore`.

## Interface Alignment
- Implements the persisted hub selection contract used during startup resolution.
- Keeps all path normalization and validation behavior local to the concrete class.
- Exposes only an explicitly persisted, still-valid `.wshub` path as the startup selection.
  Invalid or missing selections now collapse to an empty startup path so the app routes into onboarding instead of
  silently promoting a blueprint sample hub.
- For local desktop paths, “valid” now also requires that the `.wshub` directory still exists on disk.
