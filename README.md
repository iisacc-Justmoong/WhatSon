# WhatSon

WhatSon is an LVRS-based Qt Quick application.

## Structure

- `src/app`: LVRS-based UI application
- `src/daemon`: background daemon skeleton

## Adaptive Layout

- `src/app/qml/Main.qml` classifies runtime platform via `Qt.platform.os`, resolves an explicit
  `desktop/mobile` main-layout branch, and selects `MobileNormalLayout` on iOS/Android with media-query fallback for
  desktop mobile breakpoints.

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

- Sidebar routing no longer depends on a shared `SidebarSelectionStore`; QML binds directly to each hierarchy
  viewmodel context (`libraryHierarchyViewModel`, `projectsHierarchyViewModel`, etc.).
- `WhatSonBackendBridge` (`src/app/viewmodel/bridge`) is exposed to QML as `backendBridge` and provides a thin
  signal/slot interface layer (`backendEvent`, `commandRequested`) between view and backend.
- All hierarchy models and viewmodels expose at least one explicit hook slot/signal pair
  (`requestModelHook`/`modelHookRequested`, `requestViewModelHook`/`viewModelHookRequested`) for future runtime
  interception workflows.
- Panel-level MVVM is now explicit for every QML panel under `src/app/qml/view/panels/**`:
  `main.cpp` injects `panelViewModelRegistry`, and each panel binds its own key
  (`panelViewModelRegistry.panelViewModel("<panel-key>")`) to a dedicated `PanelViewModel` instance.
- Navigation mode state is centralized in `src/app/viewmodel/navigationbar/NavigationModeViewModel.*`:
  `main.cpp` injects `navigationModeViewModel`, and the navigation bar mode combo binds to the dedicated enum-backed
  `View/Edit/Control/Presentation` state plus its per-mode QObject viewmodels.
- Editor view mode state is centralized in `src/app/viewmodel/navigationbar/EditorViewModeViewModel.*`:
  `main.cpp` injects `editorViewModeViewModel`, and the navigation bar editor-view combo binds to the dedicated
  enum-backed `Plain/Page/Print/Web` state plus its per-view QObject viewmodels.
- Figma navigation frames are split into dedicated QML files under `src/app/qml/view/panels/navigation/`:
  `NavigationPropertiesBar.qml`, `NavigationInformationBar.qml`, `NavigationModeBar.qml`,
  and `NavigationEditorViewBar.qml`.
- The right-side application area is also split by navigation mode into dedicated QML files:
  `NavigationApplicationViewBar.qml`, `NavigationApplicationEditBar.qml`,
  `NavigationApplicationControlBar.qml`, and `NavigationApplicationPresentationBar.qml`.
- `Main.qml` binds a global `Tab` shortcut that cycles `View/Edit/Control/Presentation` only when no text input or
  text editor currently owns focus.
- All QML view files under `src/app/qml/view` and root `Main.qml` expose a common hook pair
  (`viewHookRequested`, `requestViewHook()`).
- Main runtime wires hierarchy selection changes and note-list model resets into `backendBridge.publish(...)`
  so view-side listeners can observe backend state transitions without coupling to concrete stores.
- View can issue lightweight commands through `backendBridge.request(...)`; current shallow routing supports
  `bridge.ping` and `hierarchy.select`.
- Sidebar hierarchy rendering is driven by a single enum-like toolbar state (`0..7`), and the current state selects
  both the active hierarchy viewmodel and the displayed sidebar variant.
- Toolbar index ownership is top-level only (`Main.qml`), and sidebar components emit index-change requests upward
  without mutating their own bound index.
- `SidebarHierarchyView` applies `depthItems` to the bound viewmodel only when explicit non-empty external depth data
  is provided, preventing accidental overwrite of runtime-loaded models.
- Hierarchy viewmodels expose a common CRUD-facing surface (`renameEnabled`, `createFolderEnabled`,
  `deleteFolderEnabled`, `itemLabel`, `renameItem`, `createFolder`, `deleteSelectedFolder`).
- Hierarchy/list models (`FlatHierarchyModel`, `LibraryHierarchyModel`, `TagsHierarchyModel`,
  `LibraryNoteListModel`) now expose validation hooks for backend/UI interception:
  `strictValidation`, `correctionCount`, `lastValidationCode`, `lastValidationMessage`,
  `validationIssueRaised(code, message, context)`, and `itemCorrected(code, context)`.
- In non-strict mode, invalid payload fields are corrected and emitted as hook events; in strict mode, the first
  validation issue raises a C++ `std::runtime_error` before mutating model state.
- Bucket header rows (`accent=true`, `depth=0`) are treated as structural labels and are excluded from
  `renameItem`/`deleteSelectedFolder` targets.
- Hierarchy list chevrons are derived from parsed depth relationships only (visible only when a direct child exists),
  and sidebar indentation uses a fixed `8px` step per depth level.
- Chevron click now toggles fold/unfold through LVRS `HierarchyItem.expanded`, and sidebar delegates follow
  `HierarchyItem.rowVisible` for effective height/visibility so collapsed descendants do not reserve row space.
- `library`: `WhatSonLibraryHierarchy{Store,Parser,Creator}` (`Library.wslibrary/index.wsnindex`)
- `projects`: `WhatSonProjectsHierarchy{Store,Parser,Creator}` (`Folders.wsfolders`)
- `bookmarks`: `WhatSonBookmarksHierarchy{Store,Parser,Creator}` (`Bookmarks.wsbookmarks`)
- `tags`: `WhatSonTagsHierarchy{Store,Parser,Creator}` (`Tags.wstags`, tree/flat JSON with preserved hierarchy depth)
- `resources`: `WhatSonResourcesHierarchy{Store,Parser,Creator}` (`Resources.wsresources`)
- `progress`: `WhatSonProgressHierarchy{Store,Parser,Creator}` (`Progress.wsprogress`)
- `event`: `WhatSonEventHierarchy{Store,Parser,Creator}` (`Event.wsevent`)
- `preset`: `WhatSonPresetHierarchy{Store,Parser,Creator}` (`Preset.wspreset`)

Folders hierarchy file behavior (Library sidebar):

- `Folders.wsfolders` supports tree-style JSON with `schema: "whatson.folders.tree"` and
  `folders: [{id, label, children: [...] }]`.
- `depth` is not a persisted file field; it is computed at parse/view-model mapping time for sidebar rendering.
- Legacy list/object formats are still accepted and normalized into runtime depth entries.

Library runtime classification behavior:

- `All`: indexes `.wsnindex` entries and enriches them with `.wsnhead` metadata (`id`, created/modified
  timestamps, and related fields)
- `All`: reads each note's `.wsnbody`, extracts text inside `<body>...</body>`, and uses it as note-list summary text
- `All`: scans both fixed `Library.wslibrary` and dynamic `*.wslibrary` roots under each `*.wscontents`
- `Draft`: filters notes where `<folders>` resolves to an empty list
- `Today`: filters notes where `<created>` or `<lastModified>` matches the current date

Runtime IO components (`src/app/file/IO`):

- `WhatSonIoEventListener`: accepts LVRS/runtime event names with prefix filtering and queues IO events.
- `WhatSonSystemIoGateway`: owns filesystem UTF-8 read/write/append/remove and directory utility operations.
- `WhatSonIoRuntimeController`: bridges queued IO events to system IO operations (`io.ensureDir`,
  `io.writeUtf8`, `io.appendUtf8`, `io.readUtf8`, `io.removeFile`) and stores structured last-result output.

Bookmarks runtime behavior:

- `.wsnhead` `<bookmarks state="...">` is parsed into bookmark state (`bool`) and bookmark colors (`string list`)
- Bookmark colors support name tokens and hex tokens; both are normalized to hex for note-list rendering
- Bookmarks hierarchy list is derived from runtime note records and includes only notes where `bookmarked == true`
- `WhatSonBookmarksHierarchyStore` maintains a canonical 9-color hex criteria set that matches `.wsnhead` bookmark color
  tokens:
  `#EF4444`, `#F97316`, `#F59E0B`, `#EAB308`, `#22C55E`, `#14B8A6`, `#3B82F6`, `#8B5CF6`, `#EC4899`

## Unified Build And Launch Automation

`scripts/build_all.py` now orchestrates platform-split scripts:

- `scripts/build_host.py`: build + launch on the current development machine
- `scripts/build_android.py`: build + install + launch on Android, then export Android Studio artifact
- `scripts/build_ios.py`: build + install + launch on connected iOS physical device

Default run (sequential to avoid peak CPU saturation):

```bash
python3 scripts/build_all.py
```

Run a single platform script directly:

```bash
python3 scripts/build_host.py
python3 scripts/build_android.py
python3 scripts/build_ios.py --ios-device "<UDID-or-Device-Name>"
```

Task selection through orchestrator:

```bash
python3 scripts/build_all.py --tasks host,android,ios
python3 scripts/build_all.py --tasks host --no-host-run
python3 scripts/build_all.py --tasks ios --ios-device "<UDID-or-Device-Name>"
python3 scripts/build_all.py --tasks host,android,ios --parallel
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

- Runs `build_all.py` sequentially with clean build directories (which internally calls platform-split scripts).
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
