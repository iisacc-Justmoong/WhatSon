# `src/extension/trial`

## Role
This directory contains an optional trial-entitlement kit for builds that should unlock the app only for a fixed evaluation window.

The module is intentionally isolated from the mandatory app and daemon build graph. Consumers can opt in explicitly, while tests compile the sources directly to keep the policy covered.

## Files
- `WhatSonTrialInstallStore.hpp` / `WhatSonTrialInstallStore.cpp`: persist and normalize the first local install date in `QSettings`.
- `WhatSonTrialClientIdentityStore.hpp` / `WhatSonTrialClientIdentityStore.cpp`: persist the trial-only device UUID and 32-character in-app client key in `QSettings`.
- `WhatSonTrialRegisterXml.hpp` / `WhatSonTrialRegisterXml.cpp`: read and write `.whatson/trial_register.xml` for local `.wshub` packages.
- `WhatSonTrialClockStore.hpp` / `WhatSonTrialClockStore.cpp`: persist shutdown timestamps and track a monotonic UTC high-water mark for rollback detection.
- `WhatSonTrialActivationPolicy.hpp` / `WhatSonTrialActivationPolicy.cpp`: resolve the 90-day entitlement state and expose it through a small `QObject` API.
- `WhatSonTrialWshubAccessBackend.hpp` / `WhatSonTrialWshubAccessBackend.cpp`: block `.wshub` package access when the trial state is expired or when a local hub register key no longer matches the in-app trial key.

## Policy Contract
- The first evaluation writes `extension/trial/installDate` if it is missing.
- Trial length is fixed to 90 calendar days.
- The app remains active from the install date through `installDate + 89 days`.
- On `installDate + 90 days`, `remainingDays` becomes `0` and `active` becomes `false`.
- Trial-only hub registration must use `.whatson/trial_register.xml`.
- The trial client persists one local `deviceUUID` value and one 32-character alpha-numeric `clientKey` value for the current install.
- Trial hub registration writes both `deviceUUID` and `key` XML elements, but only the `key` value is used for access control.
- Trial hub creation flows should call `WhatSonTrialClientIdentityStore::ensureIdentity()` and then write that identity into `.whatson/trial_register.xml`.
- If `WhatSonRegisterManager::authenticated()` is `true`, trial restrictions are considered disabled and trial-only register checks must be ignored.
- Exit handlers should call `WhatSonTrialClockStore::stampExitTimestamp()` on shutdown.
- Rollback detection compares the current UTC time against `extension/trial/lastSeenTimestampUtc`, not only against the raw last exit timestamp.
- `.wshub` access backends may use `WhatSonTrialWshubAccessBackend` to reject expired trial sessions, local hubs missing `.whatson/trial_register.xml`, or local hubs whose register key does not match the current in-app trial key.
- The trial register filename is intentionally different from product register files such as `.register.xml`, so trial-only enforcement never collides with the licensed build path.

## Current Limits
- This is a basic local policy only.
- It still does not resist machine migration or remote entitlement sync.
