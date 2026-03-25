# `src/extension/trial/WhatSonTrialClientIdentityStore.hpp`

## Role
Declares the trial-only in-app identity store that owns the persisted device UUID and 32-character client key for the current install.

## Public API
- `loadDeviceUuid()` / `loadClientKey()`: return normalized persisted values or clear invalid stored data.
- `loadIdentity()`: returns the normalized pair of values without generating replacements.
- `ensureIdentity()`: guarantees that both values exist by generating missing entries and persisting them to `QSettings`.
- `storeIdentity(...)`: replaces the stored identity with normalized values.
- `clear()`: removes the trial-only identity from `QSettings`.

## Data Contract
- `deviceUuid` is stored separately from the client key so future trial flows can inspect it without parsing XML.
- `key` must always be a 32-character alpha-numeric string.
