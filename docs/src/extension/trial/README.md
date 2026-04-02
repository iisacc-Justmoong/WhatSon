# `src/extension/trial`

## Role
This directory contains an optional trial-entitlement kit for builds that should unlock the app only for a fixed evaluation window.

The module is intentionally isolated from the mandatory app and daemon build graph. Consumers can opt in explicitly,
and validation is handled through runtime integration flows.

## Files
- `WhatSonTrialSecureStore.hpp` / `WhatSonTrialSecureStore.cpp`: provide the optional OS secure-store bridge used by the trial module.
- `WhatSonTrialInstallStore.hpp` / `WhatSonTrialInstallStore.cpp`: persist and normalize the first local install date in both `QSettings` and the OS secure store.
- `WhatSonTrialClientIdentityStore.hpp` / `WhatSonTrialClientIdentityStore.cpp`: persist the trial-only device UUID, 32-character in-app client key, and register-integrity secret in both `QSettings` and the OS secure store.
- `WhatSonTrialRegisterXml.hpp` / `WhatSonTrialRegisterXml.cpp`: read and write `.whatson/trial_register.xml` for local `.wshub` packages, including an integrity signature.
- `WhatSonTrialClockStore.hpp` / `WhatSonTrialClockStore.cpp`: persist shutdown timestamps and track a monotonic UTC high-water mark for rollback detection.
- `WhatSonTrialActivationPolicy.hpp` / `WhatSonTrialActivationPolicy.cpp`: resolve the 90-day entitlement state and expose it through a small `QObject` API.
- `WhatSonTrialWshubAccessBackend.hpp` / `WhatSonTrialWshubAccessBackend.cpp`: block `.wshub` package access when the trial state is expired, when a local hub register key no longer matches the in-app trial key, or when the register signature fails verification.

## Policy Contract
- The first evaluation writes `extension/trial/installDate` and mirrors it into the OS secure store when that backend is available.
- Trial length is fixed to 90 calendar days.
- The app remains active from the install date through `installDate + 89 days`.
- On `installDate + 90 days`, `remainingDays` becomes `0` and `active` becomes `false`.
- Trial-only hub registration must use `.whatson/trial_register.xml`.
- The trial client persists one local `deviceUUID` value, one 32-character alpha-numeric `clientKey` value, and one register-integrity secret for the current install.
- `QSettings` acts as a mirrored cache. When secure-store data is available, the secure-store value is authoritative and repopulates `QSettings` after reinstall or local settings deletion.
- Trial hub registration writes `deviceUUID`, `key`, and a `signature` element into `.whatson/trial_register.xml`.
- The register signature is an HMAC-SHA256 over the trial payload and is verified against the current in-app register-integrity secret before any key comparison is trusted.
- Trial hub access still compares only the `key` value after integrity verification succeeds; `deviceUUID` remains informational.
- Trial hub creation flows should call `WhatSonTrialClientIdentityStore::ensureIdentity()` and then write that identity into `.whatson/trial_register.xml`.
- If `WhatSonRegisterManager::authenticated()` is `true`, trial restrictions are considered disabled and trial-only register checks must be ignored.
- Exit handlers should call `WhatSonTrialClockStore::stampExitTimestamp()` on shutdown.
- Rollback detection compares the current UTC time against `extension/trial/lastSeenTimestampUtc`, not only against the raw last exit timestamp.
- `.wshub` access backends may use `WhatSonTrialWshubAccessBackend` to reject expired trial sessions, local hubs missing `.whatson/trial_register.xml`, local hubs whose register signature fails verification, or local hubs whose register key does not match the current in-app trial key.
- The trial register filename is intentionally different from product register files such as `.register.xml`, so trial-only enforcement never collides with the licensed build path.

## Current Limits
- This is still a local-only policy.
- The secure-store bridge currently targets the host OS secure store where supported by the optional trial module build.
- It still does not resist machine migration or remote entitlement sync.
