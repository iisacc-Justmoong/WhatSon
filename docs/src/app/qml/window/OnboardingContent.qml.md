# `src/app/qml/window/OnboardingContent.qml`

## Role
This QML surface owns the shared LVRS onboarding experience for desktop and mobile startup flows.

## Dialog Routing
- The shared onboarding surface now exposes an inline `Hub name` LVRS input so both desktop and mobile creation flows
  derive their package name from the same user-editable source instead of hardcoding `Untitled.wshub`.
- Desktop hub creation still uses a lazily created `FileDialog` in `SaveFile` mode so the final `.wshub` target path
  is not pre-instantiated before the user confirms creation, but the suggested save target now tracks the current hub
  name field.
- Android and macOS existing-hub selection now use a lazily created `FileDialog` with the `*.wshub` filter,
  preserving direct package-pick flows on platforms where the picker can return the package itself.
- Other desktop platforms continue to keep the folder-based existing-hub picker path, preserving directory-style
  `.wshub` selection where the package is represented as an ordinary folder.
- iOS existing-hub selection no longer depends on `QtQuick.Dialogs`. The screen now imports
  `WhatSon.App.Internal 1.0` and routes selection through `WhatSonIosHubPickerBridge`, which presents the native
  Files document browser with provider-friendly content types and an app-owned `Open` action.

## iOS Provider Behaviour
- The iOS onboarding guidance now explicitly tells the user to browse Files or cloud storage, then either select the
  `.wshub` package directly or open it and choose any file or folder inside it before confirming with `Open`.
- Mobile hub creation now keeps the selected folder URL as the security-scoped authority, resolves the final package
  path under that folder, and lets `OnboardingHubController` invoke the shared `WhatSonHubCreator` callback directly
  for the full `.wshub` scaffold.
- Accepted URLs from the native picker are forwarded to
  `OnboardingHubController::prepareHubSelectionFromUrl(...)`, which keeps the ancestor-remap/security-scoped restore
  path centralized in the controller instead of duplicating it in QML.
- Browser busy/error state is folded into the onboarding status label so the action links are disabled while the native
  browser is open and browser failures surface through the same status text channel as controller failures.

## Layout Note
- The mobile/embedded onboarding surface still reuses the same LVRS geometry and avoids the fullscreen rounded shell
  that previously triggered iOS first-frame Metal churn.
