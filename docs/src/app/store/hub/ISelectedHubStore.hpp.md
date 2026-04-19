# `src/app/store/hub/ISelectedHubStore.hpp`

## Role
`ISelectedHubStore` defines the persisted startup-hub selection contract.

## Contract
- Read current hub path and access bookmark.
- Resolve the startup hub path from the persisted selection only.
- Clear or update the persisted selection.

## Notes
- Startup resolution code now depends on this interface rather than the concrete settings-backed store.
