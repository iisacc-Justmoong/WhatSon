# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp`

## Role
Declares WhatSon-specific QML root loading helpers backed by LVRS `appentry` APIs.

## Public Helpers
- `loadQmlRoot(...)`: loads an arbitrary QML module root and returns the LVRS `QmlRootLoadResult`.
- `loadWhatSonAppRoot(...)`: loads a root object from the `WhatSon.App` QML module and preserves the LVRS root/window
  result.
- `loadMainWindowRoot(...)`: convenience wrapper for the primary `Main` root with the full LVRS load result.
- `lastRootObject(...)`: extracts the last created root object for legacy call sites that still need a `QObject*`.
- `loadQmlRootObject(...)`, `loadWhatSonAppRootObject(...)`, and `loadMainWindow(...)`: compatibility wrappers around
  the result-returning helpers.

## Policy
Callers pass `lvrs::QmlWindowActivationPolicy` instead of manually calling `show()`, `raise()`, or
`requestActivate()`. This keeps root creation and activation aligned with the LVRS runtime bootstrap contract.

New runtime paths should keep the `QmlRootLoadResult` and feed it into LVRS lifecycle/foreground-service helpers instead
of rebuilding root/window state from a raw `QObject*`.
