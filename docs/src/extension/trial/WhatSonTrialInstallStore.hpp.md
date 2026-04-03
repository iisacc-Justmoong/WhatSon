# `src/extension/trial/WhatSonTrialInstallStore.hpp`

## Role
Declares the persistence helper that owns the local install-date key for the optional trial extension.

## Public API
- `defaultInstallDateSettingsKey()`: returns the canonical `QSettings` key used by the trial module.
- `loadInstallDate()`: reads the persisted date and returns an invalid `QDate` when nothing valid is stored.
- `ensureInstallDate(...)`: lazily creates the install date on first use.
- `storeInstallDate(...)`: writes an explicit install date in ISO format.
- `clear()`: removes the stored value.

## Notes
- The store signs the persisted `QSettings` payload with the secure-store-backed register-integrity secret.
- A legacy secure-store install date can still be migrated forward, but the secure store is no longer used as a mirrored copy after migration.
- Consumers are expected to set the usual Qt organization and application names before using `QSettings`.
