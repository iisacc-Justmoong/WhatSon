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
Once a mobile hub load succeeds, `main.cpp` keeps the standalone onboarding window alive until the workspace `QWindow`
has been loaded and activated, so Android does not fall into a transient no-window state during the onboarding handoff.
