# `src/app/qml/window/OnboardingContent.qml`

## Role
This QML surface owns the shared LVRS onboarding experience for desktop and mobile startup flows.

## Dialog Routing
- Desktop hub creation still uses a lazily created `FileDialog` in `SaveFile` mode so the `.wshub` target path is not
  pre-instantiated before the user confirms creation.
- Android existing-hub selection still uses a lazily created `FileDialog` with the `*.wshub` filter, preserving the
  direct package-pick flow on that platform.
- iOS existing-hub selection no longer depends on `QtQuick.Dialogs`. The screen now imports
  `WhatSon.App.Internal 1.0` and routes selection through `WhatSonIosHubPickerBridge`, which presents the native
  Files document picker with provider-friendly content types.

## iOS Provider Behaviour
- The iOS onboarding guidance now explicitly tells the user to browse Files or Box, open the `.wshub` package,
  select a file inside it, and confirm with `Open`.
- Accepted URLs from the native picker are forwarded to
  `OnboardingHubController::prepareHubSelectionFromUrl(...)`, which keeps the ancestor-remap/security-scoped restore
  path centralized in the controller instead of duplicating it in QML.
- Picker busy/error state is folded into the onboarding status label so the action links are disabled while the native
  picker is open and picker failures surface through the same status text channel as controller failures.

## Layout Note
- The mobile/embedded onboarding surface still reuses the same LVRS geometry and avoids the fullscreen rounded shell
  that previously triggered iOS first-frame Metal churn.
