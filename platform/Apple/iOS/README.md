# iOS

This directory contains iOS platform-specific bundle configuration.

- `Info.plist` must not declare the legacy `CFBundleIconFile` key. The app icon is provided by the Xcode asset
  catalog generated from `resources/icons/app/ios/*` via `src/app/CMakeLists.txt`, and that `.xcassets` directory must
  be attached to the bundle `Resources` phase so Xcode emits `Assets.car`.
- `Info.plist` keeps `UIRequiresFullScreen` enabled because the current iOS target only declares portrait /
  landscape-left / landscape-right orientations.
- When no startup `.wshub` is available on mobile, app bootstrap still loads `Main.qml` first and lets the LVRS page
  stack open `/onboarding` inside the root application window.
- `Main.qml` must not import macOS-only `Qt.labs.platform` directly. The native menu bar lives in
  `src/app/qml/window/MacNativeMenuBar.qml` so iOS static QML linking does not pull that module into the root shell.
- `src/app/CMakeLists.txt` intentionally keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for the clean Xcode export path,
  so the app target must explicitly link the static QML plugin targets for `QtQuick`, `QtQuick.Window`,
  `QtQuick.Layouts`, `QtQuick.Controls`, and `QtQuick.Dialogs`. Otherwise the mobile onboarding route exits at runtime
  with `module "QtQuick" plugin "qtquick2plugin" not found`.
- `src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm` must start a security-scoped resource session for Files
  picker URLs before onboarding creates or loads a `.wshub`. The native iOS document picker returns document-provider
  URLs, but Qt does not start access automatically for app-level file I/O, so the app must retain that access itself
  for the rest of the session.
- iOS native file dialogs only implement the open path (`QIOSFileDialog::show()` returns `false` for save mode), so
  mobile onboarding must create hubs through a folder picker and synthesize a unique `Untitled*.wshub` path inside the
  selected directory before calling `WhatSonHubCreator`.
- iOS keeps the native Files folder picker for existing-hub selection as well. When the chosen folder contains multiple
  `.wshub` packages, `OnboardingHubController::prepareHubSelectionFromUrl()` exposes those candidates back to
  `Onboarding.qml` so the user can finish choosing the hub inside the onboarding session after the picker returns.
- iOS startup now routes onboarding through `Main.qml` and the LVRS internal page stack when no hub has been restored.
  The app keeps a single `ApplicationWindow` alive and only commits `/onboarding` -> `/` after both hub load success
  and LVRS route confirmation. `hubLoaded` now advances the onboarding controller into `routingWorkspace`, while the
  final `ready` state is only reached after `Main.qml` receives the workspace navigation callback.
- `OnboardingHubController` also validates that the resolved iOS selection is a mountable local `.wshub` directory
  before runtime bootstrap starts. Unsupported document-provider URLs or incomplete package scaffolds now stay inside
  onboarding with an explicit error instead of collapsing the app session after the picker closes.
- iOS participates in the same `.whatson/write-lease.json` single-writer lease as desktop and Android. A hub cannot be
  mounted for writing while another live WhatSon session is still refreshing that lease, so onboarding surfaces a
  conflict error instead of silently allowing concurrent package mutation.
