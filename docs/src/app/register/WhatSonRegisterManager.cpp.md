# `src/app/register/WhatSonRegisterManager.cpp`

## Role
Implements the persisted authentication manager on top of `QSettings`.

## Behavior
- The default settings key is `register/authenticated`.
- Construction reloads the persisted flag immediately, so fresh manager instances reflect the current app authentication state.
- Writing the flag updates both the in-memory state and the persisted `QSettings` value.
