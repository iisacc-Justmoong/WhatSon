# WhatSon

WhatSon is an LVRS-based Qt Quick application.

## Structure

- `src/app`: LVRS-based UI application
- `src/daemon`: background daemon skeleton

## Adaptive Layout

- `src/app/qml/Main.qml` selects `MobileNormalLayout` when runtime OS is iOS/Android, with media-query fallback for
  mobile breakpoints.

## Search Input Behavior

- Status bar search uses `LV.InputField` in `searchMode` and exposes editable state via QML properties/signals.
- Sidebar hierarchy search uses `LV.InputField` in `searchMode` and filters visible hierarchy rows in real time.

## LVRS Integration Pattern

The app CMake file follows the minimum recommended LVRS configuration.

```cmake
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)

qt_add_executable(WhatSon main.cpp)
qt_add_qml_module(WhatSon
    URI WhatSon.App
    VERSION 1.0
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES qml/Main.qml
)

lvrs_configure_qml_app(WhatSon)
```

If LVRS cannot be found, pass the LVRS prefix through `CMAKE_PREFIX_PATH` at configure time.

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$HOME/.local/LVRS
```

LVRS platform-layout reference:

- macOS: `~/.local/LVRS/platforms/macos`
- iOS: `~/.local/LVRS/platforms/ios`
- Android: `~/.local/LVRS/platforms/android`
- WASM: `~/.local/LVRS/platforms/wasm` (if installed)

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

## Unified Root CMake Targets

Platform build, launch, export, and package targets are centralized in the root `CMakeLists.txt`.

```bash
cmake --build build --target whatson_build_all
cmake --build build --target whatson_run_app
cmake --build build --target whatson_healthcheck_daemon
cmake --build build --target whatson_export_binaries
cmake --build build --target whatson_package
```

Mobile launch targets from root:

```bash
cmake --build build --target whatson_launch_ios
cmake --build build --target whatson_launch_android
cmake --build build --target whatson_launch_wasm
```

Mobile artifact export targets from root:

```bash
cmake --build build --target whatson_export_xcodeproj
cmake --build build --target whatson_export_android_studio
cmake --build build --target whatson_export_wasm_site
```

LVRS-native aliases are still available:

```bash
cmake --build build --target launch_WhatSon_ios
cmake --build build --target launch_WhatSon_android
cmake --build build --target launch_WhatSon_wasm
cmake --build build --target export_WhatSon_xcodeproj
cmake --build build --target export_WhatSon_android_studio
cmake --build build --target export_WhatSon_wasm_site
```

## Bootstrap All Platforms

The script below applies local environment fixes before LVRS bootstrap.

- Uses `arm64` as the default iOS simulator architecture.
- Detects Qt iOS mixed-slice kits and reports clear remediation guidance when arm64 simulator linking is broken.
- Detects Android NDK from `${ANDROID_SDK_ROOT}/ndk/*` and exports Android toolchain env vars.
- Cleans stale iOS bootstrap output to avoid stale `Debug` bundle install mismatches.

```bash
./scripts/bootstrap_whatson.sh all
```

Platform-only runs:

```bash
./scripts/bootstrap_whatson.sh macos
./scripts/bootstrap_whatson.sh ios
./scripts/bootstrap_whatson.sh android
```

Optional temporary fallback:

```bash
WHATSON_IOS_ALLOW_X86_FALLBACK=1 ./scripts/bootstrap_whatson.sh ios
```

Use this only for diagnosis. Modern iOS simulator runtimes are arm64-only, so `x86_64` apps may fail to install.

## Run

```bash
cmake --build build --target whatson_run_app
./build/src/daemon/WhatSon_daemon --healthcheck
```

## Debug Trace Mode

The app now supports high-visibility runtime tracing for note/header indexing and hub runtime flow.
Trace logs are printed with `[whatson:debug]` and `[wsnhead:index]` prefixes.

Current trace coverage includes:

- app startup and blueprint discovery
- hub runtime / placement / tags state load path
- tags file reader / JSON parser / depth flattener
- note header parser / store setters
- library and tags hierarchy model/viewmodel sync path
- library right-panel note list model sync path (All/Draft/Today bucket filtering)
- sidebar hierarchy per-section item model instantiation and activation path
- sidebar selection store state transition and capabilities updates
- Apple permission bridge request/callback flow
- library note indexing and runtime classification lifecycle (`All`, `Draft`, `Today`)

- Default: enabled
- Disable explicitly:

```bash
WHATSON_DEBUG_MODE=0 cmake --build build --target whatson_run_app
```

- Enable explicitly:

```bash
WHATSON_DEBUG_MODE=1 cmake --build build --target whatson_run_app
```

Filter only debug lines during a run:

```bash
./build/src/app/bin/WhatSon.app/Contents/MacOS/WhatSon 2>&1 | rg "\\[whatson:debug\\]|\\[wsnhead:index\\]"
```

## Hierarchy IO Components

`src/app/file/hierarchy` now includes per-domain getter/setter store + parser + creator components
for hub/note hierarchy payloads.

- `library`: `WhatSonLibraryHierarchy{Store,Parser,Creator}` (`Library.wslibrary/index.wsnindex`)
- `projects`: `WhatSonProjectsHierarchy{Store,Parser,Creator}` (`Folders.wsfolders`)
- `bookmarks`: `WhatSonBookmarksHierarchy{Store,Parser,Creator}` (`Bookmarks.wsbookmarks`)
- `tags`: `WhatSonTagsHierarchy{Store,Parser,Creator}` (`Tags.wstags`, tree/flat JSON with preserved hierarchy depth)
- `resources`: `WhatSonResourcesHierarchy{Store,Parser,Creator}` (`Resources.wsresources`)
- `progress`: `WhatSonProgressHierarchy{Store,Parser,Creator}` (`Progress.wsprogress`)
- `event`: `WhatSonEventHierarchy{Store,Parser,Creator}` (`Event.wsevent`)
- `preset`: `WhatSonPresetHierarchy{Store,Parser,Creator}` (`Preset.wspreset`)

Library runtime classification behavior:

- `All`: indexes `.wsnindex` entries and enriches them with `.wsnhead` metadata (`id`, `title`, created/modified
  timestamps, and related fields)
- `All`: scans both fixed `Library.wslibrary` and dynamic `*.wslibrary` roots under each `*.wscontents`
- `Draft`: filters notes where `<folders>` resolves to an empty list
- `Today`: filters notes where `<created>` or `<lastModified>` matches the current date

## Unified Build And Launch Automation

`scripts/build_all.py` is a single entrypoint that runs the currently validated workflow:

- Build and launch on the current development machine.
- Build, install, and launch on a connected Android physical device.
- Build `.app`, install, and launch on a connected iOS physical device.
- Export an Android Studio project artifact.

Default run (parallel):

```bash
python3 scripts/build_all.py
```

Task selection:

```bash
python3 scripts/build_all.py --tasks host,android,ios
python3 scripts/build_all.py --tasks host --no-host-run
python3 scripts/build_all.py --tasks ios --sequential
python3 scripts/build_all.py --tasks ios --ios-device "<UDID-or-Device-Name>"
```

Behavior by OS:

- macOS: runs host + Android + iOS flows.
- Linux/Windows: iOS task is skipped automatically; host and Android flows still run.

Logs are written to `build/automation-logs/*.log` by default.
Default artifacts are generated at:

- iOS Xcode project: `build/ios-xcode-artifact/WhatSon.xcodeproj`
- Android Studio project: `build/android-studio-artifact`

You can override artifact locations:

```bash
python3 scripts/build_all.py \
  --ios-project-dir build/ios-xcode-artifact \
  --android-studio-dir build/android-studio-artifact
```

## Runtime Smoke Matrix

`scripts/runtime_smoke_matrix.py` provides an execution-focused verification layer on top of `build_all.py`.
It is intended to prove that the same UI codebase is built and launched across platforms with clean state.

What it does:

- Runs `build_all.py` sequentially with clean build directories.
- Verifies shell-layout QML cache files exist in host and Android build outputs.
- Runs host runtime smoke (launch + short liveness window).
- Verifies Android runtime state (`com.lvrs.whatson` resumed) and captures screenshot artifact.
- Runs iOS simulator smoke when possible (build/install/launch/screenshot).
- Auto-skips iOS runtime smoke for known Qt kit slice mismatch unless strict mode is enabled.

Examples:

```bash
python3 scripts/runtime_smoke_matrix.py
python3 scripts/runtime_smoke_matrix.py --tasks host,android --skip-ios-smoke
python3 scripts/runtime_smoke_matrix.py --tasks ios --strict-ios-smoke
```

Outputs:

- Logs: `build/runtime-matrix-logs/*.log`
- Artifacts: `build/runtime-matrix-artifacts/`
