# `src/app/CMakeLists.txt`

## Responsibility
- Consumes the root-declared `WhatSon` target and keeps Qt Quick, LVRS, source, and QML registration close to the app entrypoint.
- Provides helper functions that child domains use to register sources, include paths, and QML assets.
- Delegates domain ownership through explicit `add_subdirectory(...)` entries.

## Current Directory Split
- `src/app/models/calendar/CMakeLists.txt`
- `src/app/models/content/CMakeLists.txt`
- `src/app/models/detailPanel/CMakeLists.txt`
- `src/app/models/file/CMakeLists.txt`
- `src/app/models/navigationbar/CMakeLists.txt`
- `src/app/models/onboarding/CMakeLists.txt`
- `src/app/models/panel/CMakeLists.txt`
- `src/app/models/sensor/CMakeLists.txt`
- `src/app/models/sidebar/CMakeLists.txt`
- `src/app/permissions/CMakeLists.txt`
- `src/app/platform/CMakeLists.txt`
- `src/app/policy/CMakeLists.txt`
- `src/app/qml/CMakeLists.txt`
- `src/app/register/CMakeLists.txt`
- `src/app/runtime/CMakeLists.txt`
- `src/app/store/CMakeLists.txt`

## Build Shards
- `src/app/cmake/resources/CMakeLists.txt`: desktop app icon resources, onboarding illustration resources, Apple bundle icon staging, and Windows icon RC generation.
- `src/app/cmake/defaults/CMakeLists.txt`: LVRS defaults and desktop Apple plist / entitlements forwarding.
- `src/app/cmake/runtime/CMakeLists.txt`: link libraries, output properties, host Apple framework links, LVRS runtime import overlays, and `lvrs_configure_qml_app(WhatSon)`.

## Helper Surface
- `whatson_app_register_sources(...)`: validates file existence before attaching sources to `WhatSon`.
- `whatson_app_register_directory_sources(...)`: registers C/C++ sources and headers for child directories.
- `whatson_app_register_directory_include_directories(...)`: expands recursive header-owner directories into include paths.
- `whatson_app_register_qml_entries(...)` and `whatson_app_register_directory_qml(...)`: collect `src/app`-relative QML and JS paths for the root QML module finalization.

## Current Notes
- QML ownership is isolated to `src/app/qml/CMakeLists.txt`.
- The former note editor and body-persistence shards remain removed until a new document model contract is introduced.
- Desktop trial builds pull dedicated trial activation sources from `src/extension/trial` and define `WHATSON_IS_TRIAL_BUILD=1`.
- The app target links `iiXml::iiXml` and `iiHtmlBlock::iiHtmlBlock`; root CMake owns package discovery.
- This repository no longer owns separated platform app packaging, launch, or export wiring.

## Verification Notes
- Build-system refactors touching this file should run `cmake --build build --target whatson_build_regression -j`.
- If compilation reachability changes, also run `cmake --build build --target whatson_regression -j`.
