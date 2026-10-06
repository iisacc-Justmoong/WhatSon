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

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/register``  ( `docs/src/app/register/README.md` )
- Location:  `docs/src/app/register`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
