# `src/app/qml/window/OnboardingContent.qml`

## Role

## Dialog Routing
  derive their package name from the same user-editable source instead of hardcoding `Untitled.wshub`.
  inline Hub name field. This keeps the native folder browser/sidebar navigation available while still routing the
  final scaffold through `OnboardingHubController::createHubInDirectoryUrl(...)`.
- Windows/Linux desktop hub creation still uses a lazily created `FileDialog` in `SaveFile` mode so the final `.wshub`
  target path is not pre-instantiated before the user confirms creation, but the suggested save target now tracks the
  current hub name field.
- The Qt dialog start folder is now injected only when each dialog is opened instead of staying permanently bound to
  the onboarding default directory during navigation.
  preserving direct package-pick flows on platforms where the picker can return the package itself.
- Other desktop platforms continue to keep the folder-based existing-hub picker path, preserving directory-style
  `.wshub` selection where the package is represented as an ordinary folder.
  Files document browser with provider-friendly content types and an app-owned `Open` action.

  `.wshub` package directly or open it and choose any file or folder inside it before confirming with `Open`.
  package path under that folder, and let `OnboardingHubController` invoke the shared `WhatSonHubCreator` callback
  directly for the full `.wshub` scaffold.
- Accepted URLs from the native picker are forwarded to
  `OnboardingHubController::prepareHubSelectionFromUrl(...)`, which keeps the ancestor-remap/security-scoped restore
  path centralized in the controller instead of duplicating it in QML.
- Browser busy/error state is folded into the onboarding status label so the action links are disabled while the native
  browser is open and browser failures surface through the same status text channel as controller failures.

## Layout Note
- Desktop now keeps the branding stack, hub-name editor, and action links on separate condensed spacing tracks so the
  added creation field does not push the title block upward and the action block downward after the inline naming flow
  was introduced.
- Inline hub-name input backgrounds and vertical inset now use `LV.Theme.accentTransparent` and `LV.Theme.gapNone`
  instead of raw transparent/zero literals.
  `LV.Theme` token compositions instead of direct scaled pixel literals.
