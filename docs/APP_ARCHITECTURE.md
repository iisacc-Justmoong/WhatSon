# WhatSon Application Architecture Analysis

Last Updated: 2026-03-18  
Repository: `WhatSon`  
Scope: Full application architecture (build, runtime, data, UI, eventing, and test contracts)

---

## 1. Executive Summary

WhatSon is a cross-platform Qt 6.5+ application built with QML and LVRS 1.0. The codebase is split into:

- An LVRS-based GUI app (`WhatSon`)
- A minimal background daemon (`WhatSon_daemon`)
- A Rust CLI launcher (optional target, with `whatson onboard` for onboarding-only window launch)
- A test suite validating file-format parsing, runtime stores, hierarchy view-model behavior, and QML wiring contracts

At runtime, the app auto-discovers the first `blueprint/*.wshub` package, parses hierarchy domains in worker threads,
applies snapshots to dedicated view-models on the main thread, and exposes those view-models to QML through context
properties. When no startup hub is available, the onboarding surface stays in charge of native create/select dialogs,
can scaffold a new `.wshub` package via `WhatSonHubCreator`, and then loads the resulting hub into the same runtime
view-model graph.

The dominant architecture pattern is:

- File/domain store + parser layer (`src/app/file/**`)
- View-model/model layer (`src/app/viewmodel/**`)
- LVRS QML composition layer (`src/app/qml/**`)

---

## 2. Build and Target Architecture

## 2.1 Root Build Graph

Defined in `CMakeLists.txt`:

- Optional build toggles:
    - `WHATSON_BUILD_APP` (ON)
    - `WHATSON_BUILD_DAEMON` (ON)
    - `WHATSON_BUILD_CLI` (ON)
    - `WHATSON_ENABLE_TESTING` (ON)
    - `WHATSON_RUN_TESTS_ON_BUILD` (ON)
- Aggregated custom targets:
    - `whatson_build_all`
    - `whatson_test`
    - `whatson_run_app`
    - `whatson_run_daemon`
    - `whatson_export_binaries`
    - `whatson_package`
    - mobile/export helpers for iOS/Android
- CLI behavior:
    - `build/cargo/release/whatson` launches the desktop app directly when a prebuilt executable exists
- `build/cargo/release/whatson onboard` forwards `--onboarding-only` into the app and loads
  `src/app/qml/window/Onboarding.qml` without workspace bootstrap, then promotes into `Main.qml` after the user
  creates or selects a hub
- iOS/Android startup now stays inside `src/app/qml/Main.qml` when no startup `.wshub` can be restored. The mobile
  shell uses LVRS `ApplicationWindow` page-stack routing (`/onboarding` -> `/`) instead of bootstrapping a second
  top-level onboarding window and then handing off to the workspace shell
- `OnboardingHubController` resolves existing hubs from the `.wshub` root, from a parent directory that contains a
  single `.wshub`, and from any nested directory/file path inside a `.wshub` bundle so mobile document pickers can
  still promote package-internal selections to the runtime hub root
- On iOS `OnboardingHubController` starts a security-scoped resource session for Files-picker URLs before hub create/load
  operations, and retains access to the resolved `.wshub` root so the runtime loader can mount unpacked hub directories
  returned by the native document picker
- Mobile onboarding no longer relies on desktop-style save-file semantics for hub creation. `Onboarding.qml` opens a
  directory picker on iOS/Android, then `OnboardingHubController::createHubInDirectoryUrl()` synthesizes a unique
  `Untitled*.wshub` package path inside that folder before delegating to the normal hub-creation callback
- Existing hub selection now diverges by mobile platform: iOS keeps the native folder picker and
  `OnboardingHubController::prepareHubSelectionFromUrl()` candidate flow, while Android uses a native file picker that
  can target a `.wshub` package document directly
- Android hub create/load bootstrap no longer rewrites SAF `content://` document URLs from either
  `com.android.externalstorage.documents` or `com.android.providers.downloads.documents` into synthetic
  `/storage/...` filesystem paths
- Android onboarding now routes those provider URIs through `src/app/platform/Android/WhatSonAndroidStorageBackend.*`.
  The backend keeps the SAF URI as the source document, materializes the selected `.wshub` package into a stable
  app-local mounted directory, and passes that mounted local path into the existing runtime loader so the loader can
  keep its directory-based contract
- iOS keeps the opposite strategy inside the active app session: `src/app/platform/Apple/AppleSecurityScopedResourceAccess.*`
  starts and retains a security-scoped resource session for the selected `.wshub`, and the runtime keeps reading and
  writing the real package directory in place like a Files-backed vault instead of using a mirrored local mount. The
  selected Files scope is also persisted as an implicit bookmark and restored during startup preflight, so cold
  relaunch no longer falls back to onboarding when the bookmark remains valid
- `OnboardingHubController::localPathFromUrl()` now preserves the raw fully-encoded Android picker URI before desktop
  path normalization runs, so opaque SAF providers still enter the Android document backend instead of falling through
  to the generic non-mountable-provider rejection branch
- Android hub creation uses the same backend in reverse: `WhatSonHubCreator` scaffolds a temporary local `.wshub`,
  `WhatSonAndroidStorageBackend::exportLocalHubToDirectory()` writes it into the selected SAF directory, and the
  mounted local copy becomes the hub path that runtime/state stores and `SelectedHubStore` persist
- `Main.qml` now registers both the workspace shell route (`/`) and a mobile onboarding route (`/onboarding`) on the
  LVRS internal `PageRouter`, but the embedded mobile route state itself is owned by
  `OnboardingRouteBootstrapController` rather than by ad-hoc `Main.qml` booleans
- `main.cpp` now injects `onboardingHubController`, `desktopOnboardingWindowVisible`, and
  `onboardingRouteBootstrapController` before loading `Main.qml`. The coordinator provides the app-owned
  `startupRoutePath`, and `Main.qml` mirrors that seed into LVRS `initialRoutePath` and `pageInitialPath` so mobile
  startup does not depend on a later post-load route correction
- `Main.qml` explicitly disables LVRS `mobileOversizedHeightEnabled` for the app root. The default oversized mobile
  window contract centers the live page host inside a much taller synthetic surface, which is incompatible with
  WhatSon's full-screen routed onboarding surface and can leave only the background fill visible on iOS/Android
- `Main.qml` disables LVRS `delegateMobileInsetsToSystem` and forces `forceFullWindowAreaOnMobile`, so the routed app
  surface itself owns the visible safe-area bands on iOS and Android. The old backdrop override remains only as a
  fallback path for non-full-window mobile hosts
- `Main.qml` now keeps LVRS `forcedDeviceTierPreset` in auto-detect mode (`-1`). Current LVRS bootstrap defaults
  already removed the old `UltraTier` shell pin, so WhatSon must not retain a stale downstream preset override
- `main.cpp` re-acquires iOS Files access during startup hub preflight through
  `AppleSecurityScopedResourceAccess::ensureAccessForPath(...)` before it decides `startupHubMounted`. The stored
  `.wshub` path is checked first and its parent directory is retried as a fallback, reducing false onboarding
  fallbacks after reopening the app with a previously selected Files-backed hub
- `OnboardingHubController` still owns the session state
  `idle -> resolvingSelection -> loadingHub -> hubLoaded -> routingWorkspace -> ready`, but
  `OnboardingRouteBootstrapController` now owns the LVRS route commit state that sits around that session
- Embedded mobile onboarding still defers the actual route flip one event turn after `hubLoaded` / `operationFailed`,
  but that deferral is now emitted from `OnboardingRouteBootstrapController::routeSyncRequested(...)` instead of being
  hand-coded in multiple `Main.qml` branches
- Mobile hub selection now performs a `.wshub` mount preflight before the runtime loader starts. On Android that
  preflight runs through `WhatSonAndroidStorageBackend`, which verifies the provider-backed `.wshub`, mirrors it into
  the deterministic app-local mount root, and only then hands the mounted directory to the runtime parallel loader. If
  the provider document is unreadable or the unpacked hub scaffold is structurally incomplete, onboarding fails
  in-session with a targeted error instead of handing a broken path to the runtime loader
- `Onboarding.qml` keeps the Figma `OnboardWindowDesktop` split layout on desktop, and switches to the Figma
  `OnboardWindowMobile` single-surface layout through the shared `OnboardingContent.qml` surface on mobile/adaptive
  shells: a centered `209px` content stack with
  `144px` app icon, `48px` title, `12px` version label (`75px` width), and an `180px` CTA column separated by
  `32px` / `24px` gaps. The previous lower status panel was removed from the mobile branch, and the fallback design
  frame is now the exact Figma `470x762` node.
- Startup hub resolution restores the last selected `.wshub` startup candidate from the app session store first,
  defers final validity to the mount preflight itself, and only then falls back to `blueprint/*.wshub` if the
  persisted candidate can no longer be mounted.
- `SelectedHubStore` now normalizes Android mounted-hub paths back to their provider source URI before persistence, so
  the next app launch can re-materialize the hub even if the previous app-local mount directory was transient.
- Startup onboarding is now driven by a `startupHubMounted` preflight state rather than by full runtime-domain load
  success. `main.cpp` first resolves the persisted selection into a mountable startup hub path, and only then decides
  whether onboarding should own the first route.
- Runtime hub change detection now lives in `src/app/sync/WhatSonHubSyncController.*`. `main.cpp` wires that
  controller to the existing `loadHubIntoRuntime(...)` callback so the active workspace can accept on-disk hub changes
  without restarting the app.
- `WhatSonHubSyncController` combines a periodic poll, a recursive filesystem watcher over the mounted `.wshub`, and
  app interaction hints (`TouchBegin`, pointer press, and app activation) into one debounced filesystem watcher /
  reload path.
- Local app-owned note/folder writes are acknowledged separately through `hubFilesystemMutated()` signals from
  `LibraryHierarchyViewModel` and `BookmarksHierarchyViewModel`, allowing the sync controller to refresh its baseline
  after self-writes instead of treating every save as an external runtime reload.

Dependency discovery baseline:

- Qt 6.5 (`Core` at root, `Quick`/`QuickControls2` in app target)
- LVRS via `find_package(LVRS CONFIG REQUIRED)`
- LVRS default prefix fallback: `~/.local/LVRS` (if `LVRS_PREFIX` is not set)
- `QT_ROOT_PATH` is appended to `CMAKE_PREFIX_PATH` on native desktop hosts so direct macOS/Windows/Linux configure
  can point at a specific Qt kit prefix

## 2.2 App Target (`src/app/CMakeLists.txt`)

Key behaviors:

- Recursive source/header/QML collection
- `qt_add_qml_module` for all QML under `src/app/qml/**`
- `lvrs_configure_project_defaults(...)` + `lvrs_configure_qml_app(WhatSon)`
- Apple-specific framework linkage (EventKit, Photos, ApplicationServices on macOS host)
- Platform icon wiring from `resources/icons/app/`:
    - macOS bundle embeds `desktop/AppIcon.icns`
    - iOS generates an Xcode app icon catalog from the shipped PNG variants under `ios/`, attaches that `.xcassets`
      tree to the bundle `Resources` phase so Xcode emits `Assets.car`, and keeps `UIRequiresFullScreen` aligned with
      the declared portrait / landscape-left / landscape-right orientations
    - Android package staging overlays density-specific launcher icons from `android/<density>/AppIcon.png`
    - Windows executable embeds `desktop/AppIcon.ico`
- Host/iOS LVRS module fallback/overlay logic for runtime QML import stability
- Native desktop install/export path now stages a self-contained deploy tree:
    - macOS: `cmake --install ...` and `whatson_export_binaries` emit `WhatSon.app` at the install prefix root and
      deploy Qt/LVRS runtime content into the bundle
    - Windows: `cmake --install ...` and `whatson_export_binaries` emit `bin/WhatSon.exe` with deployed Qt/LVRS
      runtime DLLs, QML imports, plugins, and `qt.conf`
    - Linux: `cmake --install ...` and `whatson_export_binaries` deploy Qt/LVRS runtime libraries and QML imports,
      plus the desktop launcher metadata and app icon install paths

This confirms LVRS is not a style add-on; it is a first-class runtime dependency in build and QML module configuration.

## 2.3 Daemon Target (`src/daemon/CMakeLists.txt`)

- Single executable target with Qt Core linkage
- Healthcheck mode prints `status=ok`
- Native Windows install/export now deploys daemon Qt runtime dependencies into the standard install tree
- Linux install/export path keeps daemon runtime lookup aligned with the app deploy tree through install-time RPATH
- No scheduled background jobs yet (skeleton state)

---

## 3. Runtime Bootstrap and Composition

Primary entrypoint: `src/app/main.cpp`

Runtime sequence:

1. Create `QGuiApplication`
2. Configure LVRS import behavior (`qml_register_types_LVRS` or dynamic import path mode)
3. Instantiate `QQmlApplicationEngine`
4. Construct domain view-models:
    - `LibraryHierarchyViewModel`
    - `ProjectsHierarchyViewModel`
    - `BookmarksHierarchyViewModel`
    - `TagsHierarchyViewModel`
    - `ResourcesHierarchyViewModel`
    - `ProgressHierarchyViewModel`
    - `EventHierarchyViewModel`
    - `PresetHierarchyViewModel`
5. Construct runtime services:
    - `WhatSonHubRuntimeStore`
6. Resolve first blueprint package path (`blueprint/*.wshub`)
7. Parse each runtime domain from `.wshub` in parallel worker threads via
   `runtime/threading/WhatSonRuntimeParallelLoader` + `runtime/threading/WhatSonRuntimeDomainSnapshots`, then apply
   parsed snapshots to view-models on the main thread
8. Register hierarchy view-model context properties into QML root context
9. Load QML module root (`WhatSon.App`, `Main`)
10. Start permission bootstrap pipeline (Qt permission API + Apple permission callbacks)

Important behavior:

- Runtime loading is optimistic per domain: one domain failure does not stop all others.
- Startup runtime loading is parallelized by domain (library/projects/bookmarks/tags/resources/progress/event/preset
  and runtime hub store), while view-model mutation remains on the main thread for Qt object-safety.
- Operational state is observable through structured debug traces and domain view-model signal/slot propagation.

---

## 4. Layered Architecture

## 4.1 Domain/File Layer (`src/app/file/**`)

Responsibilities:

- Parse and normalize workspace files (`*.wshub`, `*.wscontents`, `*.wsfolders`, `*.wstags`, `*.wsnhead`, `*.wsnbody`,
  `index.wsnindex`)
- Store normalized in-memory state
- Generate output artifacts for persisted hierarchy structures

Main pattern repeated by domain modules:

- `Store`: normalized state representation
- `Parser`: permissive input parsing and schema variation handling
- `Creator`: canonical output generation

Examples:

- Projects: `WhatSonProjectsHierarchyStore/Parser/Creator`
- Bookmarks: `WhatSonBookmarksHierarchyStore/Parser/Creator`
- Resources/Progress/Event/Preset: same structure
- Tags: parser + flattener + path resolver + depth provider

## 4.2 Runtime Hub State Layer

`WhatSonHubRuntimeStore` aggregates:

- `WhatSonHubParser` + `WhatSonHubStore` + `WhatSonHubStat` (hub root topology + `.wsstat` metadata)
- `WhatSonHubPlacementStore` (hub coordinate state from `.whatson/hub.json`)
- `WhatSonHubTagsStateStore` (flattened tag depth state per hub)
- Runtime load policy: `loadFromWshub()` is all-or-nothing; any parse/placement/tags failure leaves previously committed
  runtime state unchanged.

Hub parser behavior:

- Validate unpacked `.wshub` directory contract
- Resolve primary contents/resources roots (`.wscontents|*.wscontents`, `.wsresources|*.wsresources`)
- Require `*.wsstat` as a mandatory hub metadata source; accept empty file as valid and keep default in-memory stat
  values
- Normalize and expose stat getters/setters (`noteCount`, `resourceCount`, `characterCount`, timestamp fields,
  participant list, profile-scoped last-modified map)
- Expose Qt signal/slot bridge for downstream model/viewmodel binding:
    - Slot: `requestParseFromWshub(path)`
    - Signals: `hubParsed`, `hubStatParsed`, `hubDomainsParsed`, `parseFailed`

Placement extraction behavior:

- Reads `coordinate.x/y` if present
- Falls back to root `x/y`
- Defaults to `(0.0, 0.0)` when manifest is absent

Tags depth behavior:

- Preferred source: `Tags.wstags`
- Fallback source: scan note headers (`*.wsnhead`) for tags
- Deduplicates and materializes depth entries

## 4.3 View-Model/Model Layer (`src/app/viewmodel/**`)

Responsibilities:

- Expose QObject/QAbstractListModel interfaces for QML
- Manage selection state, item CRUD, load status, and derived lists
- Normalize malformed input in non-strict mode
- Emit structured CRUD debug traces per domain view-model, including:
    - `*.begin` for operation entry (`setDepthItems`, `loadFromWshub`, `renameItem`, `createFolder`,
      `deleteSelectedFolder`)
    - `*.success` for committed model/store updates
    - `*.rejected` for policy-driven no-op paths (immutable/read-only hierarchies, invalid selection/state)
    - `*.failed.*` for read/parse/write failures with path and reason fields
- Global trace envelope is emitted by `file/WhatSonDebugTrace.hpp` and now includes:
    - monotonically increasing `seq`
    - process uptime `ms`
    - `pid` and `tid`
    - normalized source location (`src=<file>:<line>`)
    - owner class/function split (`owner=<class|global>`, `method=<name>`)
    - normalized function name (`fn=<signature>`)
    - one-line detail payload sanitization (`\n`, `\r` escaped, overlong payload truncated with original length)
- Instance-aware tracing is available via `traceSelf(this, ...)`:
    - `selfPtr` (instance pointer)
    - `selfClass` (derived from function signature owner)
    - for QObject-based instances: `selfQObjectClass`, `selfObjectName`
- Coverage policy:
    - default all instance member methods log with `traceSelf(this, ...)`
    - use plain `trace(...)` only in free/static contexts where `this` does not exist (parser-local helpers,
      lambdas without object capture, global bootstrap functions)
- Trace activation policy:
    - tracing is disabled by default to reduce runtime overhead
    - set `WHATSON_DEBUG_MODE=1` to enable trace emission
    - `traceSelf(...)` short-circuits before building detail payload when tracing is disabled
- System I/O trace points (`WhatSonSystemIoGateway`) emit begin/success/fail/skip with:
    - normalized absolute path
    - byte counts for read/write/append
    - host error strings (`QFile::errorString`) on open/write/remove failures
    - file/directory listing previews (`values=[a, b, ...]`) with total count

Domain-isolated support:

- Each hierarchy domain owns its model + support utilities in its own directory under
  `src/app/viewmodel/hierarchy/<domain>/`.
- Shared cross-domain flat hierarchy model/support files were removed to prevent accidental single-path coupling.
- Panel-level support is centralized in `src/app/viewmodel/panel/`:
    - `PanelViewModel` provides per-panel hook observability (`panelKey`, `hookRequestCount`,
      `requestViewModelHook`, `viewModelHookRequested`).
    - `PanelViewModelRegistry` owns dedicated `PanelViewModel` instances for every QML panel under
      `src/app/qml/view/panels/**` and exposes lookup to QML through `panelViewModelRegistry`.
- Navigation bar mode support is centralized in `src/app/viewmodel/navigationbar/`:
    - `NavigationModeViewModel` owns the enum-backed `View/Edit/Control` state exposed to QML.
    - `NavigationModeSectionViewModel` provides one dedicated QObject case per mode plus the active case object.
    - `EditorViewModeViewModel` owns the enum-backed `Plain/Page/Print/Web/Presentation` editor view state exposed to
      QML.
        - `EditorViewSectionViewModel` provides one dedicated QObject case per editor view plus the active case object.
        - `main.cpp` exposes the runtime instances to QML as `navigationModeViewModel` and
          `editorViewModeViewModel`, while `NavigationPropertiesBar.qml` composes the split Figma frame files
          `NavigationInformationBar.qml`, `NavigationModeBar.qml`, and `NavigationEditorViewBar.qml`.
        - `NavigationBarLayout.qml` switches the right-side application bar through the active navigation mode and loads
          one of `navigation/view/NavigationApplicationViewBar.qml`,
          `navigation/edit/NavigationApplicationEditBar.qml`, or
          `navigation/control/NavigationApplicationControlBar.qml`.
        - `NavigationBarLayout.qml` compact mode is now the mobile node `174:4986` top-bar contract: the shared frame
          stays `24px` high on `panelBackground10`, uses `8px/2px` internal padding from LVRS tokens, keeps a `4px`
          settings-to-mode gap on the left, and a `12px` menu-to-new-folder gap on the right while still resolving the
          application surface through the active navigation mode.
        - Control-only child bars (`NavigationCalendarBar.qml`, `NavigationAppControlBar.qml`,
          `NavigationExportBar.qml`)
          now live under `navigation/control/`, while shared bars such as `NavigationAddNewBar.qml` and
          `NavigationPreferenceBar.qml` stay at the navigation root.
        - `navigation/control/NavigationApplicationControlBar.qml` compact mode is reduced to the collapsed menu button
          only; that `LV.IconMenuButton` now uses the `generalsearch` glyph, and tapping it reuses the existing `LV.ContextMenu` item set instead of introducing a second mobile-only
          control strip. The popup anchors from the trigger's bottom-right point, and action-only entries disable the
          default LVRS shortcut placeholder column so label width matches the intended mobile control-menu layout.
        - `navigation/control/NavigationApplicationControlBar.qml` preserves the Figma child order `Calendar -> AppControl -> Export ->
          AddNew -> Preference`, which keeps the create control on the right side of the control-mode application bar.
            - The non-control application bars currently provide the shared baseline `NavigationPreferenceBar.qml`,
              matching
              the default preference controls shown in the view/edit mode layouts.
            - `Main.qml` derives the desktop sidebar initial width from the effective hierarchy-toolbar width.
                - Current Figma contract: `200px` toolbar track + `2px` left/right insets => `204px` sidebar base width.
                - `Main.qml` owns the global `Tab` shortcut and cycles navigation mode only when the focused object is
                  not a
                  text
                  input/editor (`text`, `cursorPosition`, `selectedText` focus contract).
            - `NavigationModeBar.qml` and `NavigationEditorViewBar.qml` render the current `Control` / `Plain` design
              case
              through those active state objects, but click selection is menu-driven through `LV.ComboBox` +
              `LV.ContextMenu`, not direct next-state cycling.
            - `NavigationAddNewBar.qml`, `NavigationApplicationControlBar.qml`, and `MobileHierarchyPage.qml` all forward
              the same `create-note` view hook into `LibraryHierarchyViewModel::createEmptyNote()`.
            - `Main.qml` binds the platform-native New shortcut (`Cmd+N` on macOS, `Ctrl+N` elsewhere) to that same
              `create-note` hook path instead of duplicating note-creation policy in a second shortcut-only code path.
            - `StatusBarLayout.qml` compact mode is now the node `174:4986` mobile bottom bar: it resolves to a `20px`
              frame, keeps the field flush with the shared `370px` content span, reserves the trailing
              `controlHeightMd` slot for the add-note affordance, and still emits `createNoteRequested`, which
              `MobileHierarchyPage.qml` forwards into `MainWindowInteractionController::createNoteFromShortcut()` so the
              bottom add-note affordance stays on the same shared creation path as desktop navigation controls. The
              compact search input itself now uses the LVRS `shapeCylinder` type so the mobile bottom bar keeps the
              shared pill-shaped search affordance without a custom field fork.
            - `MobilePageScaffold.qml` keeps the compact navigation bar and compact status bar mounted around every
              routed mobile workspace page, so those top/bottom controls are not re-declared per page.
            - `MobilePageScaffold.qml` follows the Figma `174:4987` VStack contract directly:
              `left/right = 16`, `top = 48`, `bottom = 16`, and a shared `gap2` VStack spacing between navigation,
              body, and status sections.
            - `HierarchySidebarLayout.qml` still exposes search/inset geometry inputs for shared use, and the routed
              `MobileHierarchyPage.qml` now applies the node `174:5026` geometry explicitly with LVRS tokens:
              `horizontalInset = 0`, `verticalInset = 0`, `searchHeaderHorizontalInset = 0`,
              `searchHeaderMinHeight = 18`, `searchHeaderVerticalInset = 0`, `searchHeaderTopGap = 2`, and
              `searchListGap = 2`.
            - `MobileHierarchyPage.qml` disables `usePlatformSafeMargin`, keeps the hierarchy on the root
              `panelBackground01` canvas, composes token-only page paddings (`16 / 48 / 16` via LVRS gap
              composition), while the scaffold keeps a reduced `gap2` VStack spacing between the compact
              navigation bar, hierarchy shell, and compact status bar.
            - `MobilePageScaffold.qml` now mounts the routed mobile body through `LV.PageRouter`, and
              `MobileHierarchyPage.qml` feeds that stack with explicit `/mobile/hierarchy` and
              `/mobile/note-list` routes instead of a private page enum plus copied route history.
            - `MobilePageScaffold.qml` now sets `LV.PageRouter.interactiveTransitionSettleDuration = 0`, so
              interactive back-swipe commit does not replay a second stack transition after the gesture-driven motion
              already reached the target page.
            - `MobileHierarchyPage.qml` now suppresses the compact leading action on the note-list route, so the mobile
              list top bar matches the Figma frame and leaves page undo to swipe navigation instead of a visible back
              button.
            - `MobileHierarchyPage.qml` now drives left-edge page undo through `LV.PageTransitionController` and
              `LV.EventListener` touch events (`touchStarted`, `touchUpdated`, `touchEnded`, `touchCancelled`), so
              back-swipe policy follows the patched LVRS routing stack and gesture runtime instead of a local
              `DragHandler`.
            - `ListBarHeader.qml` now exposes LVRS `shapeCylinder` and `shapeRoundRect` search-field styles. The list
              header still defaults to the cylindrical pill, while `SidebarHierarchyView.qml` overrides the hierarchy
              search header to `shapeRoundRect`.
            - `LibraryHierarchyViewModel::createEmptyNote()` emits `emptyNoteCreated(noteId)`, and
              `ContentsDisplayView.qml` keeps a pending focus token until the selected note changes to that id, then
              transfers keyboard focus into `LV.TextEditor` so immediate body typing works after creation.
            - If the current note-list search filter would hide the new note, `LibraryHierarchyViewModel` clears that
              search before selecting the created note so the editor focus handoff still succeeds.
            - `NavigationModeBar.qml`, `NavigationEditorViewBar.qml`, `ListBarLayout.qml`, and `MobileHierarchyPage.qml`
              use LVRS label styles/theme tokens for local typography and avoid panel-local hardcoded font families or
              text-color literals.
                - `ListBarLayout.qml` now composes the dedicated `ListBarHeader.qml` frame (Figma node `134:3180`),
                  where
                  the
                  search field keeps LVRS defaults and only applies the minimum inline-variant overrides through local
                  header
                  properties (`transparent` background, `18px` height, `7px/3px` insets, `12px` text line box) while
                  using
                  `LV.InputField.searchMode` to keep the built-in leading search icon. The trailing buttons use LVRS
                  icon
                  assets `cwmPermissionView` and `sortByType` without extra color-token overrides. The active query is
                  pushed into the active domain note-list model (`LibraryNoteListModel` or `BookmarksNoteListModel`), so
                  visible note cards filter against runtime-parsed note body text without reparsing `.wsnote` content on
                  each edit. The inline search header also recenters the underlying `LV.InputField.inputItem` from its
                  live
                  `contentHeight`, which prevents `Space`/`Backspace` transitions from snapping the text/caret between
                  two
                  fixed vertical positions. `ListBarLayout.qml` now owns the note-list `ListView` directly, including
                  the
                  bidirectional selection bridge between the visible `ListView` and the active domain note-list model,
                  so
                  note taps and runtime selection changes stay aligned even when the active hierarchy domain swaps the
                  underlying note-list model instance. The same layout also composes `FocusedNoteDeletionBridge`, which
                  captures the visually focused note id on press and uses it for focused-list `Backspace`/`Delete`
                  keystrokes before falling back to the bound note-list model. Destructive note deletion still stays
                  delegated through `LibraryHierarchyViewModel` and executed in the file layer by
                  `WhatSonHubNoteDeletionService`, while bookmark-specific state only mirrors the result. Note-card
                  delegates bind `roleModel.<role>` directly from the runtime
                  role object instead of routing every field through a dynamic extractor helper, which keeps the card
                  data
                  path
                  simple and avoids silent blank bindings when helper functions disappear. Note drags are also app-wired
                  rather than LVRS-blocked: delegates
                  advertise `whatson.library.note` plus copy semantics, `Drag.Internal`, and explicit note-id mime data
                  while forcing `DragHandler` pointer takeover, while the
                  note-card tap handler stays on `TapHandler.DragThreshold`, but press now only marks a transient visual
                  candidate row and selection is committed on tap release, then reasserted once through a short-lived
                  pending-selection replay so drag startup is not canceled by note-model refresh. The surrounding
                  `NoteListItem.qml` surface keeps that contract split explicitly: persistent current-note highlight
                  flows
                  through a dedicated `active` binding from `ListView.currentIndex`, while transient pointer/drag
                  feedback
                  stays in `pressed`, preventing the active card from dropping its highlight as soon as the pointer is
                  released.
                  A dedicated overlay-parented `NoteListItem` preview mirrors the dragged row in the forced active
                  state, using the pointer press position as the drag hot spot so the actual card component follows the
                  cursor instead of the platform text tooltip.
                  The surrounding
                  `ListView` also yields drag-scrolling while a note row is pressed, so the delegate `DragHandler` can
                  start the `whatson.library.note` drag before the viewport steals the pointer.
                  `SidebarHierarchyView.qml`
                  now binds the higher-level LVRS `Hierarchy` surface directly to each domain view-model's standard
                  `hierarchyModel` property, normalizes that C++ `QVariantList` into a JS array before it reaches LVRS
                  editable drag logic, and the mounted sidebar now reintroduces folder reordering through a dedicated
                  `HierarchyDragDropBridge` on top of the same direct LVRS model contract. The same mounted sidebar also
                  accepts `whatson.library.note` drops directly over hierarchy rows through `assignNoteToFolder(...)`.
                - `ContentViewLayout.qml` is now only the panel wrapper for the center editor slot, while
                  `view/content/editor/ContentsDisplayView.qml` implements the actual Figma `ContentsDisplayView`
                  editing
                  surface by composing dedicated SRP modules:
                  `view/content/editor/ContentsGutterLayer.qml`,
                  `view/content/editor/ContentsMinimapLayer.qml`, and
                  `view/content/editor/ContentsDrawerSplitter.qml`. The editor keeps `LV.TextEditor` in the parent
                  surface
                  and exposes shared calculations for those child modules through explicit resolver bindings. The gutter
                  computes visible line
                  numbers from the same `editorText` source, cursor line, and editor render metrics. `wrapMode:
              TextEdit.Wrap` is enabled, but gutter numbering still stays on logical document lines by mapping each
                  logical line start through `editorItem.positionToRectangle(...)`; wrapped visual rows therefore do not
                  change the line-number sequence. The editor surface itself keeps Fill height even when the current
                  body
                  is empty, and the editable body block is top-left aligned with `48px` top inset plus `16px`
                  horizontal /
                  bottom inset, overriding the LVRS internal centered multi-line placement. Figma node `155:5345` is
                  treated as the source of truth for
                  the gutter token contract: `panelBackground04` surface, `#4E5157` inactive caption numbers, `#9DA0A8`
                  active line number, `2px` horizontal frame inset, `x=14` line-number column origin, right-aligned
                  number
                  text mirrored against the editor's `16px` left inset, and the fixed `18px` icon-rail anchor at `x=40`.
                  Figma node `155:5352` is treated as the source of truth for body typography (`LV.Theme.fontBody`,
                  `12px`,
                  medium, zero letter spacing) while the text selection highlight intentionally reuses the standard LVRS
                  input accent (`LV.Theme.accent`) instead of a panel-local darker blue. `editorText` is projected from
                  the
                  selected note's `.wsnbody` `<body>`
                  plain-text payload through the active note-list model's `currentBodyText`, and persisted back through
                  the
                  active hierarchy view-model's `saveBodyTextForNote(...)` after a short debounce so typing does not
                  force
                  a full `.wsnbody` rewrite and note-list refresh on every keystroke.
                  The surrounding editor panel and the embedded `LV.TextEditor` fill both use
                  `LV.Theme.panelBackground06`, while the lower drawer keeps `LV.Theme.panelBackground08`.
                  If no concrete note is selected, the editor surface must not fabricate a fresh unsaved draft or a
                  helper
                  prompt. Instead, the whole center surface stays blank until selection resolves to a real note.
                  The gutter depends on the logical-line offset array maintained by `ContentsLogicalTextBridge`; if that
                  backend
                  offset model breaks, the editor can still paint body text while the line-number column goes empty.
                  The same rule now applies to every helper that feeds child view state into the gutter/minimap/editor
                  shell: those helpers are treated as total functions and must return an explicit fallback value instead
                  of
                  leaking `undefined`.
                  The MVVM side of the editor now delegates note-selection/count/persistence contracts to
                  `ContentsEditorSelectionBridge`, logical-line parsing to `ContentsLogicalTextBridge`, external
                  gutter-marker
                  normalization to `ContentsGutterMarkerBridge`, and debounce/select-to-editor synchronization to
                  `ContentsEditorSession.qml`. `ContentViewLayout.qml` still resolves the incoming content view-model
                  and
                  note-list model first, but `ContentsDisplayView.qml` now consumes those backend-derived capability
                  flags
                  and
                  text metrics instead of recomputing them inline in QML.
                  Cursor-line lookup and visible-line enumeration must use that offset table plus viewport-derived
                  search
                  instead of reparsing the entire body or rescanning line `1..N` for every edit.
                  The first visible gutter row is derived by mapping the current viewport `contentY` through
                  `logicalLineNumberForDocumentY(...)`, which keeps the lookup simpler while still matching the actual
                  top
                  visible logical text row.
                  Because `LV.TextEditor` can settle its internal layout after note rebinding and surface re-entry, the
                  gutter must also expose an explicit refresh revision with a short multi-pass timer; note switches,
                  resurfacing, viewport changes, and late `contentHeight` updates should all request another sampling
                  pass
                  so the gutter rebinds to the editor's final geometry instead of staying stretched from an older note.
                  The current-line gutter marker must follow the cursor's active visual row and occupied text height
                  rather
                  than the whole trailing fill area of the editor, otherwise the blue marker can incorrectly extend from
                  the last logical line to the bottom of the panel.
                  The editor theme contract follows the Figma `ContentsDisplayView` tokens through LVRS aliases:
                  `subSurface`
                  (`panelBackground04`) for the gutter, `surfaceAlt` (`panelBackground06`) for the single main editor surface,
                  explicit `Body` typography via `LV.Theme.bodyColor` plus `12px` medium text, and the lower drawer on
                  `panelBackground08`.
                  A right-side Xcode-style minimap is also attached to the same editor document. It resolves the
                  internal
                  `TextEditor` flickable, draws a compressed body-text silhouette on `Canvas`, and maps each bar through
                  the
                  editor's actual content height plus text-start offset instead of distributing rows evenly across the
                  rail.
                  That keeps short notes top-aligned and visually tied to the text body rather than the gutter marker
                  lane.
                  The minimap paint path now rebuilds actual wrapped visual-row segments from
                  `TextEditor.positionToRectangle(...)`
                  instead of stretching one logical-line rectangle across the whole wrapped block; each row is painted
                  as a
                  thinner centered stroke so the silhouette reads like text rather than a carved slab. Those rows are
                  now
                  packed with a fixed `1px` gap between bars so the minimap remains visually denser than the main text
                  column instead of spreading rows apart across the whole rail.
                  The minimap stays borderless and inline: the visible viewport is only a translucent fill without an
                  outline, the current cursor line is reduced to the active text-line silhouette width, and click/drag
                  plus
                  `LV.WheelScrollGuard`-routed scrolling remain available.
                  The left rounded marker rail is state-driven: implicit current-line marker `current -> blue
              (LV.Theme.primary)`, externally supplied `changed -> #FFF567`, and externally supplied `conflict ->
              LV.Theme.danger`. Sync and conflict producers are not wired yet, but the QML contract already accepts
                  `gutterMarkers` entries with `type`, `startLine`, and `lineSpan` or `endLine`.
                  The syntax-guard test suite also treats malformed standalone expressions as a contract violation:
                  standalone string literals inside `Binding` blocks and bare dotted expressions such as
                  `noteListItem.imageSource` are rejected so view corruption is caught before runtime.
                  The same guard suite also asserts that data-driven views keep their explicit MVVM normalization layer:
                  assembly views use `resolved*` contract properties and editor/detail views keep explicit capability or
                  normalization helpers before consuming model/view-model state.
- Async timer/scheduler support is centralized in `src/app/runtime/scheduler/`:
    - `WhatSonCronExpression` parses/matches cron-like 5-field expressions (`minute hour day month weekday`).
    - `WhatSonUnixTimeAnalyzer` maps unix epoch seconds to stable local/UTC analysis fields.
    - `WhatSonAsyncScheduler` owns the runtime `QTimer` tick loop, schedule hook/unhook API, and emits
      `scheduleTriggered(...)` events.
    - `main.cpp` exposes the scheduler instance to QML as context property `asyncScheduler`.

Library-specific modeling:

- `LibraryAll`: full note index and metadata assembly
- `LibraryDraft`: filter notes without folders
- `LibraryToday`: filter notes by `createdAt/lastModifiedAt == today`
- `LibraryHierarchyViewModel` exposes `All Library`, `Draft`, and `Today` as immutable depth-0 system folders and
  synchronizes
  note-list filtering from those buckets plus persisted library folders
- The same system buckets now emit `draggable`, `dragAllowed`, `movable`, and `dragLocked` in the serialized LVRS
  hierarchy payload, and `applyHierarchyNodes(...)` preserves the `All Library -> Draft -> Today` prefix even if an
  incoming editable-drag payload tries to move user folders across that fixed boundary.
- Folder selection/filtering is path-based (`Folders.wsfolders.id` / normalized folder path), so nested folders and
  duplicate leaf labels do not alias each other. When note headers persist nested folder membership as split ancestor /
  leaf `<folder>` entries or leaf-only values such as `/Competitor`, the view-model rebuilds candidate full paths from
  the active hierarchy before applying note-list filters.
- Library folder drag moves that change a folder path now rewrite affected note-header `<folders>` values to the new
  canonical leaf path before committing the reordered `Folders.wsfolders` tree, so subtree extraction/reparenting does
  not orphan note assignments on stale folder IDs.
- Selection reapplication is identity-aware rather than raw-index-only for the library sidebar. If a structural edit
  keeps the same numeric row focused but swaps in a different visible folder (for example after deleting the focused
  folder and collapsing to its neighbor), `LibraryHierarchyViewModel` re-emits selection state and rebuilds the note
  list for the newly focused folder.
- System-bucket behavior is identity-based rather than label-based; user folders named `All`, `Draft`, or similar are
  still treated as normal folders unless they are one of the runtime-injected system buckets.
- Accent depth-0 library headers are treated as protected structural roots for rename/delete/move targets even when
  they are not runtime-injected system buckets.
- `LibraryHierarchyViewModel::createEmptyNote()` creates a blank note scaffold inside the active `.wslibrary` using a
  mixed-case alphanumeric `16-16` note ID, persists `.wsnhead`, `.wsnbody`, link/backlink manifests, and attachment
  metadata through the existing note creator stack, then rewrites `index.wsnindex` plus hub stat metadata before
  focusing the new note in the current folder scope.
- `WhatSonHubNoteDeletionService` is the file-layer authority for destructive note removal. It deletes the focused
  `.wsnote`, rewrites `index.wsnindex`, updates hub stat metadata, and returns the remaining indexed notes plus updated
  stat state to the caller. Integrity-sensitive path resolution and repair now live under `src/app/file/validator/`:
  `WhatSonHubStructureValidator` resolves `.wshub` structure, `WhatSonNoteStorageValidator` resolves materialized note
  storage, and `WhatSonLibraryIndexIntegrityValidator` owns orphan pruning plus index rewrite. If a stale index entry
  has no materialized `.wsnote` directory anymore, the same service now treats it as index-only cleanup and still
  removes the orphan id from the rewritten index/stat state.
- `LibraryHierarchyViewModel::deleteNoteById()` is now a thin orchestration wrapper around
  `WhatSonHubNoteDeletionService`: it forwards the active note request into the file layer, rebuilds the `All Library`
  / `Draft` / `Today` buckets from the returned note set, restores visible neighbor selection, and emits
  `noteDeleted(noteId)` so bookmark-only projections can drop the same note without owning file-system deletion
  themselves.
- `LibraryAll::indexFromWshub()` no longer keeps index-only ghost notes alive when `index.wsnindex` mentions ids whose
  `.wsnote` storage has disappeared. It now delegates that repair to `WhatSonLibraryIndexIntegrityValidator`, so
  orphan entries are pruned from the runtime note set and the library index is rewritten from the remaining
  materialized note directories, keeping note-list counts aligned with the actual filesystem.
- Library note cards can be dragged from the list pane onto editable library folders; a successful drop appends the
  resolved folder path to the note header `<folders>` list, updates `lastModified`, and rebuilds the `All Library` /
  `Draft` / `Today` bucket snapshots.
- `LibraryAll` body parser extracts `bodyPlainText` and `bodyFirstLine` from `.wsnbody` `<body>` content, strips inline
  tags
  (including custom tags such as `<Bold>`), and decodes XML entities before view-model consumption.
- The same `.wsnbody` parser now also detects the first `<resource ...>` entry, resolves its preview path against the
  note directory / enclosing `.wshub`, and exposes `bodyHasResource` plus `bodyFirstResourceThumbnailUrl` on
  `LibraryNoteRecord`.
- Note primary text is derived only from the top slice of `bodyPlainText`; when body text is empty the preview remains
  blank, so internal identifiers and filesystem stems never leak into the note card.
- `LibraryNoteListModel.searchText` filters the current visible note set against preassembled searchable user-facing
  text sourced from `LibraryNoteRecord.bodyPlainText`, the visible preview slice, and folder/tag labels, excluding
  internal note IDs and filesystem-derived metadata.
- Empty folder metadata is promoted to the user-facing `Draft` label before note-list roles are built, so cards with no
  real folder assignment still render a visible folder row and the immutable `Draft` hierarchy bucket uses the same
  classification rule as the note-card metadata.
- `BookmarksHierarchyViewModel` uses its own `BookmarksNoteListModel`; Bookmarks must not reuse
  `LibraryNoteListModel` because note-list contracts remain domain-owned even when role shapes are similar.
- `BookmarksHierarchyViewModel` may remove a bookmarked note from its own projection state after receiving
  `LibraryHierarchyViewModel::noteDeleted(noteId)`, but it must not delete note files or rewrite the shared library
  index itself.
- `LibraryNoteListModel` now exposes note-card roles as a stable view contract:
    - `id` (string)
    - `primaryText` (string)
    - `bodyText` (full parsed plain-text body from `.wsnbody` `<body>`)
        - `image` (bool; true when `.wsnbody` contains a non-text resource)
        - `imageSource` (URL string for the first resource thumbnail preview)
        - `displayDate` (formatted localized short-date string from the shared system calendar store)
        - `folders` (`QStringList`)
        - `tags` (`QStringList`)
        - `bookmarked` (bool)
        - `bookmarkColor` (hex string, bookmark icon tint)
- `LibraryNoteListModel` also exposes current note selection state to QML:
    - `currentIndex` (selected visible row)
    - `currentNoteId` (selected note id)
    - `currentBodyText` (selected note full plain-text body)
- Note-list `bodyText` values are newline-normalized but not trimmed, so editor round-trips preserve leading/trailing
  blank lines when serializing `.wsnbody` `<paragraph>` nodes.
- `NoteListItem.qml` resolves note-card visuals from LVRS theme tokens:
    - active background: `LV.Theme.accentBlueMuted`
    - hover background: `LV.Theme.panelBackground06`
    - Figma node `119:3028` frame contract: outer frame `194x102`, with `12px` horizontal padding, `8px` vertical
      padding,
      and an inner `86px` top/middle/bottom content stack separated by `8px` vertical gaps
    - primary text: 2-line `12px` semibold block with fixed `12px` line height
        - optional image mode: the card expands to `194x126`, the top row inserts the Figma `48px` `imageBox` before
          the text block, the title text stays top-left anchored inside its column, and the first resource thumbnail is
          painted over a neutral placeholder fill
        - fixed date row: always reserves the Figma middle slot and falls back to
          `systemCalendarStore.shortDatePlaceholderText` when `displayDate` is empty, or a generic `Date` label if
          the app-level calendar store is unavailable
    - bottom metadata rows: `11px` regular labels with `folder@14x14` and `vcscurrentBranch` icons inside `16px` frames,
      clipped to the card frame so metadata never bleeds past the note item bounds
        - bookmark tint fallback: `LV.Theme.accentYellow`
        - folder/tag icon assets: `folder@14x14`, `vcscurrentBranch`
        - caption text fallback: `LV.Theme.captionColor`

## 4.4 QML View Layer (`src/app/qml/**`)

Primary root:

- `Main.qml` (`LV.ApplicationWindow`)
- `Main.qml` now uses LVRS internal page-stack routing for the shell (`useInternalPageStack`, `initialRoutePath`,
  `pageInitialPath`, `pageRoutes`, `activePageRouter`) instead of a root-level ad-hoc loader deciding which top-level
  surface to mount.
- `main.cpp` must provide the mobile onboarding startup state through `QQmlApplicationEngine::setInitialProperties(...)`
  before `Main.qml` is loaded; WhatSon now injects `onboardingRouteBootstrapController` and
  `desktopOnboardingWindowVisible`, and the coordinator's `startupRoutePath` is mirrored into LVRS
  `initialRoutePath` and `pageInitialPath` before startup routing begins.
- `Main.qml` also opts out of LVRS oversized mobile window height (`mobileOversizedHeightEnabled: false`) so the
  internal page router's active page remains inside the visible viewport instead of being centered inside a synthetic
  `16384px` mobile surface.
- The current shell route is a single workspace page; that page decides between desktop and mobile shell composition
  from LVRS adaptive layout state (`adaptiveMobileLayout`), not from an independent root routing flag.
- `Main.qml` no longer imports `Qt.labs.platform` directly, and it must also avoid a static `WindowView.MacNativeMenuBar`
  type reference. The macOS-native menu bar is lazy-loaded via `Qt.resolvedUrl("window/MacNativeMenuBar.qml")`, so
  the root shell stays loadable on iOS static-QML builds while `File`, `Edit`, `View`, and `Help` keep their
  placeholder-backed top-level titles and `Window` still exposes the native `Onboarding` command through
  `showOnboardingWindow()`.
- The desktop navigation bar edge buttons now toggle the hierarchy sidebar and detail panel through root-window state
  (`hideSidebar`, `hideRightPanel`), and `BodyLayout.qml` converts those booleans into zero-width hidden panels while
  preserving the stored preferred widths so re-expansion restores the previous splitter geometry.
- `Main.qml` owns the root focus-dismiss policy: when LVRS global hit-test metadata reports a left-click on a blank
  background/container surface, the active focus chain is cleared.
- App bootstrap (`main.cpp`) enables `WHATSON_DEBUG_MODE=1` by default when the variable is not explicitly set,
  so `WhatSon::Debug::trace/traceSelf`-based logs are available in development sessions without extra launch flags.
- The iOS app target keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for Xcode-project generation, so
  `src/app/CMakeLists.txt` must explicitly link the static QML plugin targets for the built-in `QtQuick`,
  `QtQuick.Window`, `QtQuick.Layouts`, `QtQuick.Controls`, and `QtQuick.Dialogs` imports used by `Main.qml`,
  `Onboarding.qml`, and `OnboardingContent.qml`; otherwise the mobile onboarding route exits immediately with
  `module "QtQuick" plugin "qtquick2plugin" not found`.
- Render quality policy is enforced at app root for resize stability:
    - Desktop and mobile (`isDesktopPlatform || isMobilePlatform`): resize events (`onWidthChanged` /
      `onHeightChanged`) temporarily suspend dynamic
      resolution, then a debounce end-handler restores max supersample scale and re-enables dynamic resolution.
    - This keeps baseline image quality behavior unchanged while preventing repeated resize-induced quality drift.

Desktop composition:

- `StatusBarLayout`
- `NavigationBarLayout`
- `BodyLayout` (sidebar/list/content/detail panel arrangement with splitter interactions)
- `DetailPanelLayout` +
  `view/panels/detail/{RightPanel,DetailPanel,DetailPanelHeaderToolbar,DetailPanelHeaderToolbarButton,DetailContents}`
  (Figma-driven right detail panel for node `134:4150`)
    - `RightPanel` centers `DetailPanel` horizontally.
    - `DetailPanel` width contract is `min=145` (toolbar total length) and `default=194`, with full-height layout.
    - `DetailContents` consumes remaining vertical space below the 20px toolbar and 10px gap.
    - State logic is backend-driven by `DetailPanelViewModel` (`src/app/viewmodel/detailPanel/DetailPanelViewModel.*`),
      with SRP decomposition into `DetailPanelState.*` (state enum/name/value contract) and
      `DetailPanelToolbarItemsFactory.*` (toolbar item projection).
    - Detail content sections are now backed by dedicated per-state view-model instances
      (`fileInfo`, `fileStat`, `fileFormat`, `fileHistory`, `appearance`, `help`) through
      `DetailContentSectionViewModel`, mapped from the state enum by `DetailPanelViewModel`.
        - `DetailPanelHeaderToolbar` is a lightweight repeater container and delegates per-button rendering/click logic
          to
          `DetailPanelHeaderToolbarButton`.
    - Toolbar frame follows Figma node `142:4198`: `145x20`, `5px` inter-button spacing, and `20x20` icon-button cells
      with `16px` glyphs.
    - Toolbar icon order follows Figma node `142:4198`: `generalprojectStructure`, `chartBar`, `generaladd`,
      `toolwindowdependencies`, `toolWindowClock`, `featureAnswer`.
        - `DetailContents` binds to C++-computed `activeStateName`.

Mobile composition:

- `MobileHierarchyPage` via loader branch
- `Main.qml` now routes the mobile workspace shell through `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`.
  The legacy `MobileNormalLayout.qml` remains only as a compatibility wrapper for the existing panel-viewmodel
  registry.
- `src/app/qml/view/mobile/MobilePageScaffold.qml` owns the persistent mobile chrome:
    - `NavigationBarLayout.qml` in `compactMode`
    - `StatusBarLayout.qml` in `compactMode`
    - `LV.PageRouter` for the routed mobile body stack
    - `interactiveTransitionSettleDuration = 0` on that router so left-edge back commits do not appear to traverse a
      duplicated route entry
    - scaffold-level section spacing reduced to `gap2`, so the hierarchy body sits directly under the compact
      navigation bar instead of inheriting the earlier `gap24` page stack gap
- `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml` mounts two routed bodies inside that scaffold:
    - `HierarchySidebarLayout.qml` with the same `SidebarHierarchyViewModel`
    - a note-list body that reuses `ListBarLayout.qml` with `headerVisible: false` and the same
      `SidebarHierarchyViewModel.resolvedNoteListModel`
    - `LV.PageTransitionController` bound to the scaffold router plus left-edge `LV.EventListener` touch hooks for
      interactive back navigation
- `HierarchySidebarLayout.qml` / `SidebarHierarchyView.qml` expose optional `searchFieldVisible` and `footerVisible`
  switches so the mobile shell can reuse the hierarchy toolbar/list view-model pipeline while inserting the Figma
  search header and dropping the desktop footer controls.
- `BodyLayout.qml` opts the desktop hierarchy column into `searchFieldVisible: true`, preserving the shared search
  header on desktop while mobile pages decide that affordance explicitly.
- `SidebarHierarchyView.qml` does not add the custom toolbar height twice when search is enabled: LVRS `Hierarchy`
  keeps its built-in toolbar reserve, while WhatSon only contributes the explicit search-header stack gap. This keeps
  the post-search list gap at the intended token rhythm on both desktop and mobile.
Hierarchy rendering pipeline:

- `HierarchySidebarLayout` forwards toolbar activation into `SidebarHierarchyViewModel` and consumes the resolved
  hierarchy state it exposes
- `SidebarHierarchyView` now mounts the LVRS `Hierarchy` surface directly instead of composing a custom list/adapter
  stack around `HierarchyList`.
- Each domain hierarchy view-model exposes a direct `hierarchyModel` `QVariantList` property with LVRS-default roles
  (`itemId`, `key`, `label`, `depth`, `expanded`, `showChevron`, `draggable`) plus compatible drag-lock roles
  (`dragAllowed`, `movable`, `dragLocked`) and preserves the persisted flat depth ordering, matching the current LVRS
  `HierarchyList` contract where child presence, expand/collapse, and row-level drag affordance are inferred from row
  order plus explicit depth/drag roles without an intermediate WhatSon adapter.
- `SidebarHierarchyViewModel` is the single sidebar hierarchy state manager. `BodyLayout.qml` consumes
  `resolvedActiveHierarchyIndex`, `resolvedHierarchyViewModel`, and `resolvedNoteListModel` directly from that
  backend object, and `HierarchySidebarLayout.qml` forwards the same resolved hierarchy state into
  `SidebarHierarchyView.qml` without re-normalizing indices or re-resolving per-domain view-models in QML.
- `HierarchySidebarLayout.qml` now composes `HierarchyDragDropBridge`, which introspects the resolved hierarchy
  view-model for `applyHierarchyNodes(...)` / note-drop contracts and feeds LVRS `Hierarchy.editable` plus reorder
  persistence back into the active domain object.
- `BodyLayout.qml` keeps width clamp math plus panel arrangement, while `PanelEdgeSplitter.qml` owns the shared
  edge-drag interaction used by sidebar/list/right-panel splitters.
- `Main.qml` keeps shell routing/window composition, while `MainWindowInteractionController.qml` owns focus
  dismissal, resize render-quality policy, and global navigation shortcut policy.
- Chevron fold/unfold is handled directly by LVRS `Hierarchy`/`HierarchyList` through the serialized
  `expanded`/`showChevron` roles emitted by each hierarchy view-model.
- Row-activation policy: LVRS `Hierarchy.listItemActivated(...)` is authoritative. WhatSon mirrors the resulting
  `itemId` back into the active hierarchy view-model through `setSelectedIndex(...)` and replays selection with
  `activateListItemById(...)` when the underlying model changes. The same activation is also re-emitted through
  `SidebarHierarchyView.qml` and `HierarchySidebarLayout.qml` so `MobileHierarchyPage.qml` can transition from the
  hierarchy body into the note-list body without bypassing the shared selection contract.
- The current sidebar contract keeps hierarchy search out of the mounted QML surface, but folder drag-reorder and
  note-to-folder drops are restored through `HierarchyDragDropBridge` on top of the direct LVRS model contract instead
  of replacing it.
- Blank-area deselect is no longer a WhatSon-owned hierarchy contract. The direct LVRS list keeps one active keyed row
  while selection is present.
- Footer composition policy: `SidebarHierarchyView.qml` mounts the Figma `HierarchyFooter` node (`134:3178`) as a
  direct `LV.ListFooter` instance anchored bottom-left, with fixed `78x24` geometry, transparent button backgrounds,
  and explicit `generaladd` / `generaldelete` / `generalsettings` icon names. The hierarchy list reserves that footer
  height explicitly instead of routing footer actions through a local wrapper.
- Inline-rename policy: `SidebarHierarchyView.qml` owns the transient rename session locally, opens an inline
  `LV.InputField` over the active LVRS hierarchy row on `Enter` / `Return` only when the bound domain exposes
  `canRenameItem(...)` and `renameItem(...)`, projects the edited row label to an empty string while the overlay is
  active, commits via the domain view-model, and cancels on `Escape`, toolbar switches, or selection changes so rename
  state never leaks across hierarchy contexts.
- Expansion-state protection policy: user-triggered expand/collapse changes must flow into the bound hierarchy
  view-model through a narrow `setItemExpanded(itemId, expanded)` hook. `createFolder()` and other point mutations must
  not rebuild untouched hierarchy rows from stale expansion defaults, because that collapses unrelated UI state.
- Create-folder visibility policy: `SidebarHierarchyView.qml` must resolve the active LVRS row before calling
  `createFolder()`, and the receiving hierarchy view-model must expand that parent row before syncing the new child so
  footer-created folders appear immediately under the visually active item.
- Folder identity policy: library note-folder payloads should resolve to canonical full paths before they enter
  note-list view data. The UI may display only the terminal folder segment, but hierarchy filtering must use the exact
  canonical full path so duplicate leaf labels, including repeated labels along one ancestor chain, never resolve to
  the same logical folder.
- Toolbar spacing policy: `SidebarHierarchyView.qml` keeps the higher-level LVRS `Hierarchy` for the list surface, but
  mounts a dedicated LVRS icon-button `Row` over the header strip with a fixed `40 / 7` inter-icon gap so the eight
  `20px` toolbar icons remain left-anchored on the Figma `200px` `HierarchyHeaderToolbar` track.
- Create-folder focus policy: footer-triggered folder creation re-activates the inserted hierarchy row, and newly
  inserted folders use the placeholder label `Untitled`.
- Drag-reorder policy: `SidebarHierarchyView.qml` now enables LVRS `Hierarchy.editable` only when the active domain
  exposes `applyHierarchyNodes(...)` through `HierarchyDragDropBridge`, and successful LVRS `listItemMoved(...)`
  events are persisted immediately back into the domain view-model. The bound hierarchy payload is first normalized
  into a JS array because LVRS editable drag support rejects raw C++ `QVariantList` payloads.
- Mobile hierarchy policy: `MobileHierarchyPage.qml` repositions the shared hierarchy shell as a routed page, but the
  mobile shell does not fork `Hierarchy.editable`. LVRS editability still follows
  `HierarchyDragDropBridge::reorderContractAvailable`, while the first-screen mobile page hides the shared hierarchy
  footer and keeps the compact bottom status/add-note bar as the only bottom chrome. When the user activates a
  hierarchy row, the same mobile page swaps its body to a note-list body backed by
  `SidebarHierarchyViewModel::resolvedNoteListModel`, hides the desktop list header, suppresses the compact leading
  action, and suppresses the hierarchy add-folder action. That mobile page now routes through an
  internal `LV.PageRouter` stack and uses `LV.PageTransitionController` plus left-edge `LV.EventListener` touch hooks,
  so mobile-only page undo is committed by the patched LVRS routing runtime instead of replaying a private copied
  route-memory stack.
- Library note-drop policy: `SidebarHierarchyView.qml` now mounts a narrow `DropArea` over the LVRS hierarchy surface,
  resolves the underlying LVRS `HierarchyItem` from the drop position, validates the target through
  `HierarchyDragDropBridge::canAcceptNoteDrop(...)`, and persists accepted note drops through
  `HierarchyDragDropBridge::assignNoteToFolder(...)`, which appends the dropped hierarchy path into the note header
  `<folders>` payload while preserving existing folder assignments.
- Library folder move persistence: `LibraryHierarchyViewModel::applyHierarchyNodes(...)` still owns canonical
  `Folders.wsfolders` rewrites plus note-header `<folders>` normalization when LVRS drag-reorder commits a new depth
  ordering.
- Projects folder move persistence: `ProjectsHierarchyViewModel::applyHierarchyNodes(...)` still owns
  `Folders.wsfolders` rewrites when LVRS drag-reorder commits a new depth ordering.
- Rename commit policy for hub-loaded hierarchies: the view-model updates staged in-memory data, applies it to the
  domain store, then calls store-driven file sync (`writeToFile`) for `*.wsfolders`, `*.wsresources`, `*.wsevent`,
  `*.wspreset`, `*.wstags`. Model commit occurs only after successful store sync.
- Bookmarks domain behavior is color-folder driven:
    - Uses fixed 9 bookmark color folders from `WhatSonBookmarkColorPalette`.
    - Folder CRUD and view-options footer actions are disabled for the bookmarks hierarchy.
    - Selecting a bookmark color folder filters right-panel note cards by that bookmark color.
- Progress domain behavior is fixed-state driven:
    - Progress hierarchy labels are immutable constants at runtime.
    - Rename/create/delete actions are disabled in the progress hierarchy.
- Panel rendering contract:
    - Every QML file under `src/app/qml/view/panels/**` binds a panel-local `panelViewModel` property from
      `panelViewModelRegistry.panelViewModel("<panel-key>")`.
    - Panel keys are path-scoped for nested groups (`navigation.*`, `sidebar.*`) and file-name scoped for root panel
      files (for example `BodyLayout`, `NavigationBarLayout`).
    - Panel debug hook contract is standardized:
        - QML-side `requestViewHook(reason)` must forward to `panelViewModel.requestViewModelHook(reason)` and then emit
          `viewHookRequested`.
        - C++-side `PanelViewModel::requestViewModelHook(reason)` emits a unified debug trace through
          `WhatSon::Debug::traceSelf` with scope `panel.viewmodel` and action `hook.request`.
        - Detail panel state transitions must be traced through `DetailPanelViewModel` /
          `DetailContentSectionViewModel` using the same `WhatSon::Debug::traceSelf` path.
        - New in-app debug output must not introduce ad-hoc `console.log`/`qWarning` formats when an existing
          `WhatSon::Debug` trace contract already exists.

---

## 5. Data and Package Topology

Detected package conventions:

- Hub root: `*.wshub` directory
- Contents roots:
    - `.wscontents` (fixed internal path)
    - or dynamic `<HubName>.wscontents`
- Core hub domains:
    - `Library.wslibrary`
        - Note storage domain (`*.wsnote`, `*.wsnhead`, `*.wsnbody`, `*.wsndiff`, `*.wsnpaint`)
        - Global note index (`index.wsnindex`)
    - `<HubName>.wsresources`
        - Binary/resource storage domain for note attachments (image/video/audio/other payloads)
    - `<HubName>Stat.wsstat`
        - Hub-level aggregate statistics and metadata domain (note/resource counts, character totals, created/modified
          timestamps,
          participant list, and related summary attributes)
- Hierarchy/auxiliary files (typically under `*.wscontents`):
    - `Folders.wsfolders` (hierarchical folder tree JSON for user folders; runtime prepends immutable library system
      roots `All Library`, `Draft`, and `Today` ahead of this tree)
    - `Tags.wstags` (tag tree or flat list)
    - `Bookmarks.wsbookmarks` (bookmark hierarchy source)
    - `Progress.wsprogress` (progress state domain)
    - `ProjectLists.wsproj` (project hierarchy domain)
    - `Preset.wspreset` (preset hierarchy domain)
- Placement manifest:
    - `.whatson/hub.json`

### 5.1 Sync Design Intent (Filesystem-First)

Hub synchronization is intentionally network-agnostic:

- The authoritative sync unit is the entire `.wshub` directory.
- Cross-device sync is delegated to filesystem/cloud providers (for example iCloud Drive, Google Drive, Dropbox), not to
  an app-managed
  online API.
- WhatSon is expected to operate correctly even without direct network connectivity by reading/writing the shared hub
  filesystem package.

### 5.2 Conflict Minimization and Merge Strategy

Design policy for conflict reduction:

- Domain creator/parser pairs own write/read normalization for each file type.
- WhatSon no longer enforces a single-writer lease for `.wshub` mounts. Desktop, iOS, and Android sessions may mount
  the same hub concurrently, and conflict handling is deferred to the normal file-level merge/reconciliation policy.
- `WhatSonHubWriteLease` is now a legacy cleanup shim for stale `.whatson/write-lease.json` artifacts left by older
  builds. Runtime load and mutation paths keep calling the helper, but it no longer blocks concurrent sessions.
- Records should include profile-aware last-modified timestamps so conflict merge can be resolved with explicit edit
  provenance rather than opaque
  file-level overwrite behavior.
- Merge UX goal is deterministic and human-readable conflict resolution using timestamp + profile identity context.

Parser resilience is intentionally high. The code accepts schema variants such as:

- `depth`, `dpeth`, or `indentLevel`
- `label`, `name`, `title`, or `text`
- tags as root array/object fields (`tags`, `children`, `items`)

This tolerance reduces runtime breakage when file producers are inconsistent.

---

## 6. Eventing and Signal/Slot Contracts

## 6.1 Runtime Eventing Contract

Current runtime eventing follows pure Qt MVVM wiring:

- `main.cpp` does not instantiate a view-model bridge bus.
- Domain state flows from file/runtime stores to domain-specific view-models.
- QML observes state by binding directly to per-domain view-model context properties.
- Cross-layer communication is constrained to signals/slots and invokable methods on each domain view-model.

## 6.2 C++ Model/ViewModel Signal/Slot Coverage

Audit result:

- Tracked C++ model/view-model classes were re-audited after bridge removal.
- No runtime command/event bus class remains under `src/app/viewmodel`.
- Panel registry classes (`PanelViewModel`, `PanelViewModelRegistry`) follow the same explicit
  signal/slot contract as hierarchy view-models.

Conclusion:

- Current C++ architecture already satisfies the requirement that model/view-model classes expose signal and slot
  sections.

## 6.3 QML View Signal Coverage Snapshot

Audit result:

- Total QML files: 52
- Files declaring explicit `signal`: 43
- Files without explicit `signal`: 9

Files without explicit signal declarations:

- `src/app/qml/DesignTokens.qml`
- `src/app/qml/MainWindowInteractionController.qml`
- `src/app/qml/view/content/editor/ContentsGutterLayer.qml`
- `src/app/qml/view/content/editor/ContentsMinimapLayer.qml`
- `src/app/qml/window/DebugConsole.qml`
- `src/app/qml/window/MacNativeMenuBar.qml`
- `src/app/qml/window/Preference.qml`
- `src/app/qml/window/ProfileControl.qml`
- `src/app/qml/window/QuickNote.qml`

Implication:

- If the rule is interpreted strictly as "every view must define explicit signal and slot-like handlers," these files
  are the current enforcement gap.
- The previously retained LVRS override shims (`HierarchyListCompat.qml`, `SidebarHierarchyInteractionController.qml`,
  `NavigationIconButton.qml`) were removed, so the sidebar/navigation layer now relies on stock LVRS surfaces only.

---

## 7. Permission and IO Subsystems

## 7.1 Permission Pipeline

`PermissionBootstrapper` in `main.cpp` sequentially requests:

- Full disk access (macOS)
- Photo library (Apple)
- Microphone (Qt permission API)
- Accessibility (macOS)
- Calendar (Qt permission API)
- Reminders (Apple)
- Local network (Apple)
- Location (Qt permission API)

Decisions are persisted in `QSettings` under `permissions/*`.

`ApplePermissionBridge.mm`:

- Real native permission paths for macOS/iOS
- Stub implementation on non-Apple platforms grants permission immediately
- Both implementations are free-function based; debug tracing must use `WhatSon::Debug::trace(...)`, not
  `traceSelf(this, ...)`

## 7.2 IO Runtime Controller

`WhatSonIoRuntimeController` processes queued LVRS-origin IO events:

- `io.ensureDir`
- `io.writeUtf8`
- `io.appendUtf8`
- `io.readUtf8`
- `io.removeFile`

Each processed action emits structured result metadata (`ok`, `action`, `message`, `timestamp`, and extra fields).

Runtime performance notes:

- `WhatSonIoEventListener` uses a head-index queue strategy to avoid `removeFirst()` shifting cost on every event pop.
- `WhatSonIoRuntimeController::processAll(...)` consumes events directly and dispatches without recursive
  `processNext(...)` calls.
- `WhatSonSystemIoGateway` caches already-ensured directory paths to avoid repeated `mkpath` overhead on write/append
  hot paths.

---

## 8. Test-Defined Architecture Contracts

From `tests/app/**`, architecture is guarded by explicit tests:

- `test_hierarchy_viewmodels.cpp`:
    - CRUD contracts, strict-vs-non-strict correction behavior, bookmark color normalization
- `test_whatson_hub_runtime_store.cpp`:
    - runtime placement/tags loading and header-tag fallback
- `test_qml_binding_syntax_guard.cpp`:
    - mandatory text patterns for sidebar and `Main.qml` wiring
    - splitter clamp invariant in `BodyLayout.qml` (`totalSplitterWidth` must sanitize non-finite width)
        - binding syntax guard against invalid standalone literals in `Binding` blocks
- `test_solid_architecture_contracts.cpp`:
    - SRP guard for shell/sidebar/editor decomposition (`MainWindowInteractionController`, `PanelEdgeSplitter`,
      `LV.Hierarchy`, `ContentsEditorSelectionBridge`, `ContentsLogicalTextBridge`,
      `ContentsGutterMarkerBridge`, `ContentsEditorSession.qml`)
    - DIP/LSP guard for sidebar state: `SidebarHierarchyViewModel` must continue to work through
      `ISidebarSelectionStore` and `IHierarchyViewModelProvider` substitutions
    - ISP guard for editor helpers: selection, logical-text, and gutter-marker adapters must keep distinct
      meta-object contracts and the removed `ContentsEditorBridge.*` must stay absent

Status update (2026-03-01):

- The prior `Main.qml` contract drift around shell routing has been resolved.
- `Main.qml` now uses:
    - `useInternalPageStack: true`
    - `initialRoutePath: startupRoutePath`
    - `pageInitialPath: startupRoutePath`
    - `pageRoutes: [onboardingRoute, workspaceShellRoute]`
    - routed workspace layout selection from `adaptiveMobileLayout`
- This removes dependence on a root-level ad-hoc loader path and keeps shell state aligned with LVRS
  `ApplicationWindow` / `PageRouter` contracts.
- `SidebarHierarchyViewModel` now owns the resolved hierarchy index, hierarchy view-model, and note-list model
  contracts that `BodyLayout.qml` and `HierarchySidebarLayout.qml` consume directly.
- This prevents toolbar routing state from fragmenting across multiple QML assembly layers.
- Hierarchy model `ShowChevronRole` is now derived dynamically from depth adjacency at data-read time.
- Chevron visibility contract is parent-only: leaf items must not render chevrons regardless of serialized/static
  `showChevron` values.

---

## 9. Architecture Risks and Technical Debt

1. String-literal contract fragility in QML tests
    - String-based QML assertions can fail on refactors that preserve behavior but alter literal source text.

2. Sidebar state duplication regression risk
    - `SidebarHierarchyViewModel` is now the single source of truth for active hierarchy index, hierarchy view-model,
      and note-list model.
    - Any future reintroduction of local QML resolution logic in `BodyLayout.qml` or `HierarchySidebarLayout.qml` will
      re-fragment that state and increase routing drift risk.

3. Splitter clamp fragility if helper return is removed
    - `BodyLayout.qml` drag-resize math depends on finite occupied width.
    - `totalSplitterWidth()` must always return numeric width; removing return can silently disable edge drag resizing.

4. Build script complexity concentration
    - LVRS host/iOS fallback logic in app CMake is substantial and platform-conditional.
    - This area is high-impact for onboarding and CI reproducibility.

5. Developer quality tooling bootstrap state
    - Root CMake now provides `whatson_qmllint`, `whatson_qmlformat_check`, `whatson_qmlformat_fix`,
      `whatson_clang_tidy`, and `whatson_dev_checks`.
    - `clang-tidy` remains environment-dependent because the executable is not bundled with Qt/LVRS; the target is a
      contract surface, not a guarantee that the host already has LLVM tooling installed.

6. File format tolerance vs strictness
    - Parser permissiveness improves compatibility but may mask malformed upstream data unless strict validation mode is
      activated consistently.

---

## 10. Key Source Index

Build:

- `CMakeLists.txt`
- `src/app/CMakeLists.txt`
- `src/daemon/CMakeLists.txt`
- `tests/CMakeLists.txt`

Entrypoints:

- `src/app/main.cpp`
- `src/daemon/main.cpp`

Runtime stores:

- `src/app/file/hub/WhatSonHubRuntimeStore.*`
- `src/app/file/hub/WhatSonHubPlacementStore.*`
- `src/app/file/hierarchy/tags/WhatSonHubTagsStateStore.*`
- `src/app/file/hierarchy/tags/WhatSonHubTagsDepthProvider.*`

View-models and models:

- `src/app/viewmodel/hierarchy/**`
- `src/app/viewmodel/panel/**`

QML shell:

- `src/app/qml/Main.qml`
- `src/app/qml/view/panels/BodyLayout.qml`
- `src/app/qml/view/panels/HierarchySidebarLayout.qml`
- `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

Permissions and IO:

- `src/app/permissions/ApplePermissionBridge.*`
- `src/app/file/IO/WhatSonIoRuntimeController.*`

Test contracts:

- `tests/app/test_qml_binding_syntax_guard.cpp`
- `tests/app/test_hierarchy_viewmodels.cpp`
- `tests/app/test_whatson_hub_runtime_store.cpp`
- `tests/app/test_panel_viewmodel_registry.cpp`

---

## 11. Practical Conclusion

The current architecture is a layered LVRS-first Qt application with clear boundaries between domain file logic,
view-model orchestration, and QML composition. Runtime loading, signal/slot-based observability, and parser resilience
are strong. The most immediate governance needs are:

- Keeping QML string-literal contract tests synchronized with intended behavior-level contracts
- Making index normalization logic explicit in sidebar routing
- Deciding whether "every view must have signal/slot" is a strict enforcement rule for all QML files or only for
  interactive container views

This document is intended as the baseline reference for subsequent refactoring, rule enforcement, and CI-level
architecture checks.
