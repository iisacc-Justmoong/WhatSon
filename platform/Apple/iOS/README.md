# iOS

This directory contains iOS platform-specific bundle configuration.

- `Info.plist` must not declare the legacy `CFBundleIconFile` key. The app icon is provided by the Xcode asset
  catalog generated from `resources/icons/app/ios/*` via `src/app/CMakeLists.txt`, but iOS must also keep explicit
  `CFBundleIcons` / `CFBundleIcons~ipad` fallback entries that point at bundle-root PNG files.
- `src/app/cmake/resources/CMakeLists.txt` exports the existing fallback icon list, and `src/app/CMakeLists.txt`
  copies `WHATSON_IOS_BUNDLE_ICON_FILES` into the iOS bundle root with a `POST_BUILD` step. Xcode does not reliably
  attach loose PNG sources to the app `Resources` phase when they only carry `MACOSX_PACKAGE_LOCATION "."`, so the
  copy step is the reliable fallback path.
- If Xcode compiles `WhatSonIcons.xcassets`, the app still resolves the `AppIcon` asset catalog entry; if the
  generated `.xcodeproj` drops that asset catalog from the `Resources` phase, the manual `CFBundleIconFiles` fallback
  still keeps the installed home-screen icon visible on device.
- `cmake/root/runtime/CMakeLists.txt` now runs `cmake/patch_whatson_ios_xcodeproj.py` immediately after
  `whatson_generate_ios_xcodeproj` configures the clean iOS export tree.
  CMake's Xcode generator currently emits the `WhatSonIcons.xcassets` file reference and `PBXBuildFile`, but it still
  omits that asset catalog from the `WhatSon` app target's `PBXResourcesBuildPhase`. The post-export patch inserts the
  missing build-phase entry so Xcode actually compiles `Assets.car`; on simulator exports that same patch step also
  strips Qt `permissions` plugin object/library link inputs when `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY` is not
  `include`, because the current Qt iOS kit can still contribute device-only arm64 permission objects to the
  simulator link line. That patch path is intentionally idempotent so repeated export runs do not regress back to the
  broken simulator link flags. The manual PNG fallback above remains the second line of defense when the asset-catalog
  path does not survive a direct Xcode rebuild.
- The same root runtime shard now owns the iOS export knobs through CMake cache variables instead of requiring manual
  Xcode Build Settings edits:
  `WHATSON_IOS_SDK`, `WHATSON_IOS_ARCHITECTURES`, `WHATSON_IOS_DEVELOPMENT_TEAM`,
  `WHATSON_IOS_CODE_SIGN_IDENTITY`, `WHATSON_IOS_CODE_SIGN_STYLE`, and
  `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY`.
  `whatson_export_xcodeproj` translates those into the early `CMAKE_OSX_*` toolchain settings and the final
  `CMAKE_XCODE_ATTRIBUTE_*` signing metadata for the generated project. The default SDK is now `iphoneos`, because the
  current static Qt iOS kit is reliable on device export first; simulator export remains opt-in through
  `WHATSON_IOS_SDK=iphonesimulator`.
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
- `src/app/cmake/runtime/CMakeLists.txt` also routes Qt permission plugin import through the CMake
  `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY` knob. The default `auto` mode excludes Qt `permissions` plugins for
  `iphonesimulator` exports so Apple Silicon simulator links do not pull Qt's device-only arm64 permission objects,
  while `iphoneos` exports still keep the permission plugins enabled for physical-device runs.
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
  selection during settings validation. If the stored iOS `.wshub` path can still be mounted and passes the shared hub
  structure validator, the app enters the workspace directly; otherwise startup reopens onboarding with that failure.
  A restored hub that mounts but still fails the first runtime load now also falls back to onboarding so the user can
  re-pick the hub from the embedded mobile onboarding surface immediately.
- `Main.qml` also disables LVRS `mobileOversizedHeightEnabled` on iOS. The default oversized mobile surface can place
  the routed onboarding page host outside the visible viewport, leaving only the dark `windowColor` fill on screen.
- `Main.qml` disables LVRS `delegateMobileInsetsToSystem` and forces `forceFullWindowAreaOnMobile` on iOS, so the
  status-bar and home-indicator safe-area bands are part of the routed WhatSon content surface instead of a
  background-only fallback.
- `Main.qml` now leaves `forcedDeviceTierPreset` in LVRS auto-detect mode (`-1`) on iOS. Recent LVRS shell defaults no
  longer pin startup to `UltraTier`, so WhatSon must not keep the stale mobile override from the older bootstrap path.
- `WhatSonHubMountValidator` validates that the resolved iOS selection is a mountable local `.wshub` directory with the
  required hub entries before runtime bootstrap starts. Unsupported document-provider URLs or incomplete package
  scaffolds now stay inside onboarding with an explicit error instead of collapsing the app session after the picker
  closes.
- iOS now allows the same `.wshub` to be mounted concurrently with desktop and Android sessions. The old
  `.whatson/write-lease.json` gate is treated as a legacy cleanup artifact only and no longer blocks onboarding or
  runtime filesystem access.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `iOS` (`platform/Apple/iOS/README.md`)
- 위치: `platform/Apple/iOS`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
