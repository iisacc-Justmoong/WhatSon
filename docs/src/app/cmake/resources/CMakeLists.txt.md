# src/app/cmake/resources/CMakeLists.txt

## Purpose

This shard registers app-owned visual resources after the `WhatSon` executable target exists.

## macOS Bundle Icon

- `resources/icons/app/desktop/AppIcon.png` remains the Qt qrc icon used by in-app QML and `QWindow`.
- `resources/icons/app/desktop/AppIcon.icns` is the macOS application bundle icon. It is added to the
  `WhatSon` executable target as a source with `MACOSX_PACKAGE_LOCATION "Resources"`.
- `platform/Apple/Info.plist` names the same file through `CFBundleIconFile`, and
  `src/app/cmake/runtime/CMakeLists.txt` sets `MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"`.
- The shard also exports `WHATSON_MACOS_POST_BUILD_BUNDLE_ICON_FILE` so the owning app CMake file can copy the icon to
  `WhatSon.app/Contents/Resources/AppIcon.icns` after link. This keeps the icon visible with generators that do not
  materialize the declarative bundle source copy.

The app icon must stay on the macOS `.app` bundle path. Do not move this responsibility into Qt qrc resources.

## Other Platform Icons

  asset catalog from the Resources phase.
- Windows emits a generated `.rc` file that points at the desktop `.ico`.
