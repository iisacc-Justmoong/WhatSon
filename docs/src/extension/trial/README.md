# `src/extension/trial`

## Role
This directory contains an optional trial-entitlement kit for builds that should unlock the app only for a fixed evaluation window.

The module is intentionally isolated from the mandatory app and daemon build graph. Consumers can opt in explicitly, while tests compile the sources directly to keep the policy covered.

## Files
- `WhatSonTrialInstallStore.hpp` / `WhatSonTrialInstallStore.cpp`: persist and normalize the first local install date in `QSettings`.
- `WhatSonTrialClockStore.hpp` / `WhatSonTrialClockStore.cpp`: persist shutdown timestamps and track a monotonic UTC high-water mark for rollback detection.
- `WhatSonTrialActivationPolicy.hpp` / `WhatSonTrialActivationPolicy.cpp`: resolve the 90-day entitlement state and expose it through a small `QObject` API.
- `WhatSonTrialWshubAccessBackend.hpp` / `WhatSonTrialWshubAccessBackend.cpp`: block `.wshub` package access when the trial state is no longer active.

## Policy Contract
- The first evaluation writes `extension/trial/installDate` if it is missing.
- Trial length is fixed to 90 calendar days.
- The app remains active from the install date through `installDate + 89 days`.
- On `installDate + 90 days`, `remainingDays` becomes `0` and `active` becomes `false`.
- Trial-only hub registration must use `.whatson/trial_register.xml`.
- Exit handlers should call `WhatSonTrialClockStore::stampExitTimestamp()` on shutdown.
- Rollback detection compares the current UTC time against `extension/trial/lastSeenTimestampUtc`, not only against the raw last exit timestamp.
- `.wshub` access backends may use `WhatSonTrialWshubAccessBackend` to reject expired trial sessions before startup hub mount or explicit hub selection.
- The trial register filename is intentionally different from product register files such as `.register.xml`, so trial-only enforcement never collides with the licensed build path.

## Current Limits
- This is a basic local policy only.
- It does not attempt to resist clock rollback, reinstall abuse, machine migration, or remote entitlement sync.
