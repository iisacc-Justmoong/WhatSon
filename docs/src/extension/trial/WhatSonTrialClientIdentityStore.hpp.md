# `src/extension/trial/WhatSonTrialClientIdentityStore.hpp`

## Role
Declares the trial-only in-app identity store that owns the persisted device UUID, 32-character client key, and register-integrity secret for the current install.

## Public API
- `loadDeviceUuid()` / `loadClientKey()`: return normalized persisted values or clear invalid stored data.
- `loadRegisterIntegritySecret()`: returns the normalized register-signing secret for the current install.
- `loadIdentity()`: returns the normalized pair of values without generating replacements.
- `ensureIdentity()`: guarantees that the device UUID, client key, and register-integrity secret exist.
- `ensureRegisterIntegritySecret()`: lazily creates the HMAC secret used for `trial_register.xml`.
- `storeIdentity(...)`: replaces the stored identity with normalized values.
- `clear()`: removes the trial-only identity and register-integrity secret from both storage layers.

## Data Contract
- `deviceUuid` is stored separately from the client key so future trial flows can inspect it without parsing XML.
- `key` must always be a 32-character alpha-numeric string.
- The register-integrity secret is a 64-character lowercase hexadecimal string.
