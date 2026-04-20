# `src/app/store/hub/SelectedHubStore.hpp`

## Role
`SelectedHubStore` is the default settings-backed implementation of `ISelectedHubStore`.

## Interface Alignment
- Implements the persisted hub selection contract used during startup resolution.
- Keeps all path/selection-URL normalization and validation behavior local to the concrete class.
- Exposes the startup hub path directly from persisted selection state without adding a blueprint fallback.
