# `src/app/store/hub/ISelectedHubStore.hpp`

## Role
`ISelectedHubStore` defines the persisted startup-hub selection contract.

## Contract
- Read current hub path, current selection URL, and access bookmark.
- Resolve the startup hub path/URL from the persisted selection only.
- Clear or update the persisted selection.

## Notes
- Startup resolution code now depends on this interface rather than the concrete settings-backed store.
- iOS startup restore now consumes the persisted selection URL together with the bookmark so provider-backed `.wshub`
  picks can be restored even when the previous local mount path is no longer stable.
