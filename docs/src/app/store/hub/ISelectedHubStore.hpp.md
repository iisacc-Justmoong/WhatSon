# `src/app/store/hub/ISelectedHubStore.hpp`

## Role
`ISelectedHubStore` defines the persisted startup-hub selection contract.

## Contract
- Read current hub path and access bookmark.
- Resolve startup hub path with blueprint fallback.
- Clear or update the persisted selection.

## Notes
- Startup resolution code now depends on this interface rather than the concrete settings-backed store.
