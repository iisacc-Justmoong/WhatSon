# `src/app/CMakeLists.txt`

## Status
- Documentation phase: live structural overview.
- Detail level: build-orchestration summary for the current split layout.

## Source Metadata
- Source path: `src/app/CMakeLists.txt`
- Source kind: CMake build definition
- File name: `CMakeLists.txt`
- Approximate line count: 312

## Responsibility
- Owns the `WhatSon` app target shell and keeps the LVRS / Qt Quick baseline close to the app entrypoint.
- Provides shared helper functions that child domain directories use to register sources, include paths, and QML assets.
- Delegates source ownership to top-level app domains through `add_subdirectory(...)` instead of one monolithic recursive file list.

## Current Directory Split
- `src/app/agenda/CMakeLists.txt`
- `src/app/calendar/CMakeLists.txt`
- `src/app/callout/CMakeLists.txt`
- `src/app/editor/CMakeLists.txt`
- `src/app/file/CMakeLists.txt`
- `src/app/permissions/CMakeLists.txt`
- `src/app/platform/CMakeLists.txt`
- `src/app/policy/CMakeLists.txt`
- `src/app/qml/CMakeLists.txt`
- `src/app/register/CMakeLists.txt`
- `src/app/runtime/CMakeLists.txt`
- `src/app/store/CMakeLists.txt`
- `src/app/viewmodel/CMakeLists.txt`

## Build Shards
- `src/app/cmake/resources/CMakeLists.txt`: app icon resources, onboarding illustration resources, Apple bundle icon staging, Android package resource mirroring, and Windows icon RC generation.
- `src/app/cmake/defaults/CMakeLists.txt`: LVRS project-default wiring and Apple plist / entitlements forwarding.
- `src/app/cmake/runtime/CMakeLists.txt`: target link libraries, output properties, Apple framework links, LVRS runtime import overlays, and the final `lvrs_configure_qml_app(WhatSon)` call.

## Helper Surface
- `whatson_app_register_sources(...)`: validates file existence before attaching sources to `WhatSon`.
- `whatson_app_register_directory_sources(...)`: recursively or non-recursively registers C/C++ sources and headers for a child directory.
- `whatson_app_register_directory_include_directories(...)`: expands recursive header-owner directories into include paths so existing short include statements remain valid.
- `whatson_app_register_qml_entries(...)` and `whatson_app_register_directory_qml(...)`: collect QML and JS assets into global properties before the final `qt_add_qml_module(...)` call.

## Current Notes
- QML ownership is now isolated to `src/app/qml/CMakeLists.txt`, which keeps the QML module collection separate from C++ domain registration.
- Platform packaging, LVRS defaults, and runtime-linking details are now delegated again under `src/app/cmake/*`, while the final `qt_add_qml_module(...)` call stays in `src/app/CMakeLists.txt` because it finalizes the same `WhatSon` target created there.
- New top-level app domains now require an explicit sibling `CMakeLists.txt` plus a matching `add_subdirectory(...)` entry in `src/app/CMakeLists.txt`; this is intentional so architecture changes stay visible in code review.
- Existing child shards still register sources recursively inside their owned directory, so intra-domain file additions do not require another parent-file edit.
- Desktop trial builds pull in the dedicated trial activation sources from `src/extension/trial` and define `WHATSON_IS_TRIAL_BUILD=1` for the app target.
- Android and iOS builds intentionally skip the trial sources because the mobile app does not participate in the desktop trial flow.
- On Apple desktop trial builds, the app target also links the `Security` framework because the trial secure-store implementation uses the host keychain.
- The app target now declares `Qt6::Pdf` and `Qt6::PdfQuick` as first-class dependencies because `ContentsResourceViewer.qml` imports `QtQuick.Pdf` for direct `.wsresource` PDF rendering.
- iOS keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for the clean Xcode export flow, so the app now carries an explicit static QML plugin closure instead of relying on top-level imports alone.
- That closure includes the QML runtime foundation (`Qt6::qmlplugin`, `Qt6::modelsplugin`, `Qt6::workerscriptplugin`), the controls/dialog implementation chain (`Qt6::qtquicktemplates2plugin`, `Qt6::qtquickcontrols2implplugin`, `Qt6::qtquickcontrols2basicstyleimplplugin`, `Qt6::qtquickcontrols2iosstyleimplplugin`, `Qt6::qtquickdialogs2quickimplplugin`), and the feature-facing plugins already used by app QML.
- `QtQuick.Pdf` is not self-contained on iOS in this static-plugin setup: `PdfMultiPageView.qml` also imports `QtQuick.Shapes`, so `Qt6::qmlshapesplugin` must stay in the same manual plugin link set or the QML engine aborts during startup.
- This defensive list mirrors the transitive plugin closure hidden behind `QT_IS_PLUGIN_GENEX` inside Qt's imported plugin targets, which makes future `module \"...\" is not installed` regressions less likely when `PdfQuick`, dialogs, or control styles pull nested QML imports.
## Verification Notes
- Build-system refactors that touch this file should run `cmake --build build --target whatson_build_regression -j`.
- If the change can affect test wiring or compilation reachability, also run `cmake --build build --target whatson_regression -j`.
