# iOS

This directory contains iOS platform-specific bundle configuration.

- `Info.plist` must not declare the legacy `CFBundleIconFile` key. The app icon is provided by the Xcode asset
  catalog generated from `resources/icons/app/ios/*` via `src/app/CMakeLists.txt`, and that `.xcassets` directory must
  be attached to the bundle `Resources` phase so Xcode emits `Assets.car`.
- `Info.plist` keeps `UIRequiresFullScreen` enabled because the current iOS target only declares portrait /
  landscape-left / landscape-right orientations.
- When no startup `.wshub` is available on iOS, app bootstrap still loads `Main.qml` first and keeps onboarding inside
  the root application window instead of forking to a second top-level session.
- `Main.qml` must not import macOS-only `Qt.labs.platform` directly, and it must not hold a static
  `WindowView.MacNativeMenuBar` reference either. The native menu bar must be lazy-loaded from
  `src/app/qml/window/MacNativeMenuBar.qml` through `Qt.resolvedUrl(...)`, otherwise iOS still parses the
  `Qt.labs.platform` dependency during root-shell creation and exits before onboarding appears.
- `src/app/CMakeLists.txt` intentionally keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for the clean Xcode export path,
  so the app target must explicitly link the static QML plugin targets for `QtQuick`, `QtQuick.Window`,
  `QtQuick.Layouts`, `QtQuick.Controls`, and `QtQuick.Dialogs`. Otherwise the mobile onboarding route exits at runtime
  with `module "QtQuick" plugin "qtquick2plugin" not found`.
- `src/app/platform/Apple/AppleSecurityScopedResourceAccess.mm` must start a security-scoped resource session for Files
  picker URLs before onboarding creates or loads a `.wshub`. The native iOS document picker returns document-provider
  URLs, but Qt does not start access automatically for app-level file I/O, so the app must retain that access itself
  for the rest of the session.
- `Info.plist` declares `.wshub` as the exported package document type `com.iisacc.whatson.hub`, marks the app as an
  in-place document opener, and advertises the package in `CFBundleDocumentTypes`. This keeps document providers such
  as Box eligible for direct package picks while still allowing nested-file picks when a provider exposes packages as
  browsable directories.
- The same bridge now persists an implicit iOS bookmark for the selected Files-backed hub scope and restores that
  access during startup preflight before `main.cpp` decides whether onboarding is required. WhatSon therefore reopens
  the previously selected external `.wshub` on cold relaunch as long as the bookmark still resolves successfully.
- iOS native file dialogs only implement the open path (`QIOSFileDialog::show()` returns `false` for save mode), so
  mobile onboarding must create hubs through a folder picker instead of a save dialog. `OnboardingContent.qml`
  therefore collects the hub name inline, and `OnboardingHubController` resolves the final `.wshub` package path
  inside the selected directory before calling `WhatSonHubCreator`.
- That mobile create flow must start security-scoped access on the selected directory URL itself and keep that scope
  alive while `WhatSonHubCreator` writes the full `.wshub` scaffold, instead of bouncing through a synthetic child URL
  that was never returned by the Files provider.
- iOS existing-hub selection now goes through `WhatSonIosHubPickerBridge`, which presents the native
  `UIDocumentBrowserViewController` with a package-aware type list: the exported `.wshub` package type,
  `UTTypePackage`, and generic file content types.
- The bridge also injects an app-owned `Open` browser action into the native document browser navigation bar/menu, so
  WhatSon can keep a consistent explicit confirmation affordance even when a cloud provider does not expose the system
  picker `Open` button for package-internal browsing.
- That custom action now intentionally keeps the default `UIDocumentBrowserAction` content-type scope
  (`UTTypeItem.identifier`) instead of narrowing itself to `.wshub`-specific types, so provider-exposed nested files
  and folders can still activate `Open` and be remapped back to the enclosing hub root.
- Providers that honor package declarations can still open the `.wshub` package directly, while providers that expose
  the package as a directory can return an inner file that
  `OnboardingHubController::prepareHubSelectionFromUrl()` remaps back to the enclosing `.wshub`.
- iOS startup now routes onboarding through `Main.qml` itself when no hub has been restored. The app keeps a single
  `ApplicationWindow` alive from first frame, pins the LVRS page stack to `/`, and lets the workspace page loader show
  `IosInlineOnboardingSequence.qml` until a hub has been loaded successfully.
- `main.cpp` must inject `onboardingRouteBootstrapController` before `Main.qml` is loaded. `Main.qml` mirrors the
  coordinator-owned onboarding visibility into either the routed onboarding page (desktop/Android) or the iOS inline
  onboarding host.
- Embedded iOS onboarding still defers the actual workspace-transition acknowledgement by one event turn after
  `hubLoaded` / `operationFailed`, keeping the state transition out of the native Files picker teardown stack.
- Startup onboarding now consults the persisted hub selection as a mount candidate first instead of deleting that
  selection during settings validation. If the stored iOS `.wshub` path can still be mounted, the app enters the
  workspace directly; otherwise startup retries the bundled blueprint fallback before reopening onboarding.
- `Main.qml` also disables LVRS `mobileOversizedHeightEnabled` on iOS. The default oversized mobile surface can place
  the routed onboarding page host outside the visible viewport, leaving only the dark `windowColor` fill on screen.
- `Main.qml` disables LVRS `delegateMobileInsetsToSystem` and forces `forceFullWindowAreaOnMobile` on iOS, so the
  status-bar and home-indicator safe-area bands are part of the routed WhatSon content surface instead of a
  background-only fallback.
- `Main.qml` now leaves `forcedDeviceTierPreset` in LVRS auto-detect mode (`-1`) on iOS. Recent LVRS shell defaults no
  longer pin startup to `UltraTier`, so WhatSon must not keep the stale mobile override from the older bootstrap path.
- `OnboardingHubController` also validates that the resolved iOS selection is a mountable local `.wshub` directory
  before runtime bootstrap starts. Unsupported document-provider URLs or incomplete package scaffolds now stay inside
  onboarding with an explicit error instead of collapsing the app session after the picker closes.
- iOS now allows the same `.wshub` to be mounted concurrently with desktop and Android sessions. The old
  `.whatson/write-lease.json` gate is treated as a legacy cleanup artifact only and no longer blocks onboarding or
  runtime filesystem access.
