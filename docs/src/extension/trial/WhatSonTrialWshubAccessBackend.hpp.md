# `src/extension/trial/WhatSonTrialWshubAccessBackend.hpp`

## Role
Declares the optional backend that blocks `.wshub` access after the local 90-day trial expires.

## Public API
- `evaluateAccess(...)`: returns a structured decision for a local `.wshub` path or a document URI that points to a `.wshub` package.
- `canAccess(...)`: convenience boolean wrapper that also exposes a denial message.

## Decision Model
- Non-`.wshub` targets are ignored and remain allowed.
- If `WhatSonRegisterManager::authenticated()` is `true`, the backend bypasses every trial-specific restriction.
- `.wshub` targets are allowed only while `WhatSonTrialActivationPolicy` reports an active trial state.
- Expired decisions include the install-derived trial state and a user-facing denial string.
- Local `.wshub` targets also inspect `.whatson/trial_register.xml`.
- Local `.wshub` targets are denied when the trial register file is missing.
- The register payload reads both `deviceUUID` and `key`, but only the `key` comparison controls access.
- When the local hub key differs from the persisted in-app trial key, the decision is denied even if the device UUID still matches.
