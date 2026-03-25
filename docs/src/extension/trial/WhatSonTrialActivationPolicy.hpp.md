# `src/extension/trial/WhatSonTrialActivationPolicy.hpp`

## Role
Declares the 90-day activation policy for the optional trial entitlement flow.

## Public Surface
- `WhatSonTrialActivationState`: immutable-style snapshot with install date, last active date, elapsed days, remaining days, active flag, and an authentication-bypass flag.
- `refreshForDate(...)`: evaluates the policy for a supplied calendar day and updates the cached state.
- `refresh()`: slot-oriented runtime entrypoint that evaluates against `QDate::currentDate()`.
- `stateChanged()`: emitted only when the cached activation state changes.

## QML/Runtime Use
- The class is a `QObject` so an app can expose it to QML or startup orchestration later without redesigning the API.
- `refresh()` is the intended startup hook: call it once, then gate app activation on `active`.
- When `WhatSonRegisterManager::authenticated()` is `true`, `active` stays `true` and `bypassedByAuthentication` becomes `true` so downstream code can ignore trial expiration.
