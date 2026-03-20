# Android

This directory stores the Android manifest/template files that are copied into the build-tree package source.
Launcher icons are sourced from `resources/icons/app/android/<density>/AppIcon.png` and overlaid by CMake during Android
configure.
Mobile onboarding also routes hub creation through a folder picker plus synthesized `Untitled*.wshub` target path
instead of desktop-style save-file semantics, so Android shares the same create flow as iOS.
Android now keeps the native picker for existing-hub selection by opening a file picker for `*.wshub`, because the
platform folder picker cannot target a package document directly. When that picker returns a local external-storage
document URI, onboarding resolves it back to the matching shared-storage filesystem path before hub creation/loading,
because the current `.wshub` creator and runtime loader still operate on real package directories.
The Android hub-path bridge now also resolves the native Downloads tree/document provider
(`com.android.providers.downloads.documents`) to `/storage/emulated/0/Download/...`, which is the common URI emitted
by the stock folder picker when the user creates or chooses hubs from `Download`.
Android startup now routes missing-hub onboarding inside `Main.qml` through the LVRS internal page stack, so the app
does not rely on a second top-level onboarding window before entering the workspace shell.
Android mobile bootstrap now mirrors iOS route ownership: `hubLoaded` only advances the onboarding controller into
`routingWorkspace`, and `Main.qml` closes the session only after the LVRS router confirms the workspace route.
- `main.cpp` also injects `onboardingVisible` before `Main.qml` is loaded. That keeps LVRS `pageInitialPath`
  consistent with the first mobile frame instead of trying to flip the router after the workspace shell has already
  booted.
- `Main.qml` disables LVRS `mobileOversizedHeightEnabled` on Android as well. The default oversized mobile window can
  center the routed onboarding page outside the visible viewport and leave only the background fill visible.
`OnboardingHubController` also preflights the resolved `.wshub` scaffold before runtime bootstrap, so unsupported SAF
document URLs or incomplete package directories fail in-session with a targeted onboarding error instead of exploding
into a full-domain runtime load failure.
Android hub loading also participates in the shared `.whatson/write-lease.json` single-writer contract. If a desktop
or mobile WhatSon session already owns a fresh lease for the same `.wshub`, onboarding now fails early with that
explicit conflict instead of mounting a second live writer on top of the shared package.
