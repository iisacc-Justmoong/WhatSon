# `CMakeLists.txt`

## Responsibility
- Owns repository-wide toolchain setup, product build options, and the top-level `add_subdirectory(...)` graph.
- Keeps application, daemon, CLI, and `test/` enablement close to the root configuration entrypoint.
- Delegates large custom-target groups to `cmake/root/*/CMakeLists.txt` so the root file remains an orchestration surface instead of a full target catalog.

## Current Root Split
- `cmake/root/build/CMakeLists.txt`: maintained regression targets such as `whatson_build_regression` and `whatson_regression`.
- `cmake/root/dev/CMakeLists.txt`: developer tooling targets such as `whatson_qmllint`, `whatson_qmlformat_*`, and `whatson_clang_tidy`.
- `cmake/root/runtime/CMakeLists.txt`: run, healthcheck, mobile launch/export aliases, and iOS Xcode-project generation.
- `cmake/root/distribution/CMakeLists.txt`: install/export/package targets and the dedicated `build-trial` mirror flow.

## Invariants
- Keep option declarations, package discovery, and primary product `add_subdirectory(...)` calls in the root file.
- Keep grouped custom targets in `cmake/root/*` and avoid moving product-level source ownership back into the root file.
- Reuse `build/` for configure/build/test flows; the nested `build-trial` path remains opt-in packaging infrastructure only.
- User-built local libraries under `~/.local` are surfaced through cacheable root prefixes. `iiXml` and `iiHtmlBlock`
  are required when the app or regression suite is enabled, and are discovered through their CMake package configs
  before child target wiring runs. iOS builds must use platform packages, not the host macOS dylib prefixes: provide
  `WHATSON_IIXML_IOS_PREFIX` and `WHATSON_IIHTMLBLOCK_IOS_PREFIX`, or install them under
  `~/.local/iiXml/platforms/ios` and `~/.local/iiHtmlBlock/platforms/ios`.
- Keep iOS export defaults centralized in root cache variables (`WHATSON_IOS_SDK`, `WHATSON_IOS_ARCHITECTURES`,
  `WHATSON_IOS_DEVELOPMENT_TEAM`, `WHATSON_IOS_CODE_SIGN_IDENTITY`, `WHATSON_IOS_CODE_SIGN_STYLE`,
  `WHATSON_IOS_QT_PERMISSION_PLUGIN_POLICY`, `WHATSON_IIXML_IOS_PREFIX`, and
  `WHATSON_IIHTMLBLOCK_IOS_PREFIX`) so generated Xcode projects do not depend on manual Xcode Build Settings edits or
  script-local signing logic. The default export profile should stay device-first (`iphoneos`) unless the maintained
  Qt iOS kit proves simulator-safe again.

## Verification Notes
- Run `cmake -S . -B build` after structural root-CMake changes.
- Run `cmake --build build --target whatson_build_regression -j` after root-target refactors.
- Run `cmake --build build --target whatson_regression -j` when regression-target wiring changes.
- Run `cmake --build build --target whatson_generate_ios_xcodeproj -j` after iOS export-option or Xcode-project
  generation changes.
