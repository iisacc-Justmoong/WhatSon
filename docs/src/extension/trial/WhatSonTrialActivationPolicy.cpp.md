# `src/extension/trial/WhatSonTrialActivationPolicy.cpp`

## Role
Implements the basic local trial-window calculation on top of `WhatSonTrialInstallStore`.

## Evaluation Rules
- Install date is created lazily on the first refresh when no stored value exists.
- `lastActiveDate` is `installDate + 89 days`.
- `elapsedDays` clamps negative clock drift to `0` instead of producing a larger entitlement window.
- `active` is `true` while `elapsedDays < 90`.
- `remainingDays` is `90 - elapsedDays` while active, otherwise `0`.
- When `WhatSonRegisterManager::authenticated()` is `true`, the policy reports an authenticated-bypass state only after the signed authenticated marker verifies successfully.

## Change Signaling
- `stateChanged()` is emitted only when the newly computed snapshot differs from the cached one.
- This keeps future UI bindings stable and avoids redundant reactions on repeated refreshes within the same day.
