# `src/app/store/hub/ISelectedHubStore.hpp`

## Role
`ISelectedHubStore` defines the persisted startup-hub selection contract.

## Contract
- Read current hub path and access bookmark.
- Clear or update the persisted selection.

## Notes
- Startup resolution code depends on this interface for persisted selection only.
- Blueprint fallback policy now lives in `WhatSonStartupHubResolver`, not in the settings-backed store.
