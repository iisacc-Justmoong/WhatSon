# `src/app/register`

## Role
This directory contains the app-core registration state objects that are shared with optional entitlement modules such as the trial extension.

## Files
- `WhatSonRegisterManager.hpp` / `WhatSonRegisterManager.cpp`: persist the app authentication-complete state and expose it as a small `QObject` manager.

## Contract
- `authenticated == true` means the app has completed the product authentication flow.
- Trial-only modules may read this manager and disable trial restrictions entirely when the flag is `true`.
- Trial builds persist the authenticated state as a signed `QSettings` record that is verified with the secure-store-backed trial integrity secret.
- Legacy plain `QSettings` booleans are no longer trusted as an authenticated bypass source.
