# `src/app/register/WhatSonRegisterManager.cpp`

## Role
Implements the persisted authentication manager on top of `QSettings`.

## Behavior
- The default settings key is `register/authenticated`.
- Construction reloads the persisted flag immediately, so fresh manager instances reflect the current app authentication state.
- Trial builds no longer trust a plain boolean for the authenticated bypass path.
- The persisted trial-build value is a signed JSON record in `QSettings`, keyed by `register/authenticated` and verified against the secure-store-backed trial integrity secret.
- Invalid, malformed, or unsigned legacy values are rejected and cleared instead of unlocking the trial policy.
