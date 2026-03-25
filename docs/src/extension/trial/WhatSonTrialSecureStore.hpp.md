# `src/extension/trial/WhatSonTrialSecureStore.hpp`

## Role
Declares the optional OS secure-store bridge used by the trial module to keep reinstall-resistant local state.

## Public API
- `WhatSonTrialSecureStoreStatus`: describes whether a secure-store read or write succeeded, missed, was unavailable, or failed.
- `WhatSonTrialSecureStoreReadResult` / `WhatSonTrialSecureStoreWriteResult`: small result structs for storage operations.
- `WhatSonTrialSecureStoreBackend`: backend interface that can be replaced by tests.
- `WhatSonTrialSecureStore`: value-type wrapper that scopes secret entries under one service name.

## Notes
- The trial module uses the secure store for the install date, client identity, and register-integrity secret.
- Tests are expected to inject an in-memory backend instead of touching the real host secure store.
