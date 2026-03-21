# Android

This directory stores the Android manifest/template files that are copied into the build-tree package source.
Launcher icons are sourced from `resources/icons/app/android/<density>/AppIcon.png` and overlaid by CMake during Android
configure.
Mobile onboarding also routes hub creation through a folder picker plus synthesized `Untitled*.wshub` target path
instead of desktop-style save-file semantics, so Android shares the same create flow as iOS.
Android now keeps the native picker for existing-hub selection by opening a file picker for `*.wshub`, because the
platform folder picker cannot target a package document directly. The app now preserves the provider URI returned by
Android SAF instead of rewriting either `com.android.externalstorage.documents` or
`com.android.providers.downloads.documents` into synthetic `/storage/...` filesystem paths. The Android-specific
`WhatSonAndroidStorageBackend` now uses that URI as the source document, mirrors the selected `.wshub` tree into a
deterministic app-local mounted hub directory, and hands that mounted copy to the existing runtime loader. Creating a
new hub through the native folder picker follows the same backend in reverse: `WhatSonHubCreator` scaffolds a local
temporary `.wshub`, the backend exports it into the selected SAF directory, and the mounted local copy becomes the
runtime hub path.
`OnboardingHubController::localPathFromUrl()` now preserves the raw `QUrl::FullyEncoded` Android picker URI before any
desktop-style path normalization runs, so opaque SAF provider authorities still enter the Android document backend
instead of falling through to the generic "mountable local directory path" rejection branch.
Android startup now routes missing-hub onboarding inside `Main.qml` through the LVRS internal page stack, so the app
does not rely on a second top-level onboarding window before entering the workspace shell.
Android mobile bootstrap now mirrors iOS route ownership: `hubLoaded` only advances the onboarding controller into
`routingWorkspace`, and `OnboardingRouteBootstrapController` closes the session only after the LVRS router confirms the
workspace route.
- `main.cpp` now injects `onboardingRouteBootstrapController` before `Main.qml` is loaded. That coordinator owns the
  Android embedded onboarding route state and provides the `startupRoutePath` seed that `Main.qml` mirrors into LVRS
  `initialRoutePath` and `pageInitialPath`.
- Embedded Android onboarding still defers the actual route flip after `hubLoaded` / `operationFailed` with
  `Qt.callLater(...)`, but `Main.qml` now only applies the route requested by the coordinator instead of owning the
  full mobile onboarding state machine itself.
- `Main.qml` disables LVRS `mobileOversizedHeightEnabled` on Android as well. The default oversized mobile window can
  center the routed onboarding page outside the visible viewport and leave only the background fill visible.
- `Main.qml` also paints app-owned Android safe-area bands while LVRS keeps system inset delegation enabled, so the
  visible cutout and gesture-area background matches WhatSon `canvasColor` without rebinding the content root to
  fullscreen coverage.
- `Main.qml` now leaves `forcedDeviceTierPreset` in LVRS auto-detect mode (`-1`) on Android. Recent LVRS shell
  defaults already removed the old `UltraTier` startup pin, so WhatSon must not carry the stale downstream override.
`OnboardingHubController` also preflights the resolved `.wshub` scaffold before runtime bootstrap, so unsupported SAF
document URLs or incomplete package directories fail in-session with a targeted onboarding error instead of exploding
into a full-domain runtime load failure. Startup now also refreshes previously selected Android mounted hubs from their
stored source URI before bootstrapping the runtime, so the persisted app-local mount does not drift from the original
provider-backed `.wshub`.
Android hub loading also participates in the shared `.whatson/write-lease.json` single-writer contract. If a desktop
or mobile WhatSon session already owns a fresh lease for the same `.wshub`, onboarding now fails early with that
explicit conflict instead of mounting a second live writer on top of the shared package.
