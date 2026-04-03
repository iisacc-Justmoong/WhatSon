# `src/extension/trial/WhatSonTrialClientIdentityStore.cpp`

## Role
Implements the trial-only client identity persistence layer.

## Behavior
- Invalid stored UUID, key, or register-secret values are removed during reads.
- The device UUID, client key, and register-integrity secret now live in the trial secure store only.
- Legacy plain `QSettings` values are treated as migration input only when the secure-store backend is available, then removed from `QSettings`.
- Missing device UUID values are derived from `QSysInfo::machineUniqueId()` when available, with a random UUID fallback.
- Missing client keys are generated as 32-character alpha-numeric strings.
- Missing register-integrity secrets are generated as 64-character lowercase hexadecimal strings.
- `ensureIdentity()` persists the generated values into the secure store and reloads them before reporting success.
