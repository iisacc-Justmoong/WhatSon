# `src/extension/trial/WhatSonTrialSecureStore.hpp`

## Role
Declares the optional OS secure-store bridge used by the trial module to keep reinstall-resistant local state.

## Public API
- `WhatSonTrialSecureStoreStatus`: describes whether a secure-store read or write succeeded, missed, was unavailable, or failed.
- `WhatSonTrialSecureStoreReadResult` / `WhatSonTrialSecureStoreWriteResult`: small result structs for storage operations.
- `WhatSonTrialSecureStoreBackend`: backend interface that can be swapped for platform-specific adapters.
- `WhatSonTrialSecureStore`: value-type wrapper that scopes secret entries under one service name.

## Notes
- The trial module uses the secure store for the client identity and register-integrity secret.
- A legacy secure-store install date can still be migrated forward, but new install-date writes no longer keep a mirrored secure-store copy.
- Non-production flows can inject an in-memory backend instead of touching the real host secure store.
