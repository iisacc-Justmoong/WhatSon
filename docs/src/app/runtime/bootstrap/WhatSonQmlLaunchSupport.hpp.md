# `src/app/runtime/bootstrap/WhatSonQmlLaunchSupport.hpp`

## Role
Declares WhatSon-specific QML root loading helpers backed by LVRS `appentry` APIs.

## Public Helpers
- `loadQmlRootObject(...)`: loads an arbitrary QML module root through `lvrs::loadQmlRootObjects(...)`.
- `loadWhatSonAppRootObject(...)`: loads a root object from the `WhatSon.App` QML module.
- `loadMainWindow(...)`: convenience wrapper for the primary `Main` root.

## Policy
Callers pass `lvrs::QmlWindowActivationPolicy` instead of manually calling `show()`, `raise()`, or
`requestActivate()`. This keeps root creation and activation aligned with the LVRS runtime bootstrap contract.
