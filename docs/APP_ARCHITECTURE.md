# WhatSon Application Architecture Analysis

Last Updated: 2026-03-03  
Repository: `WhatSon`  
Scope: Full application architecture (build, runtime, data, UI, eventing, and test contracts)

---

## 1. Executive Summary

WhatSon is a cross-platform Qt 6.5+ application built with QML and LVRS 1.0. The codebase is split into:

- An LVRS-based GUI app (`WhatSon`)
- A minimal background daemon (`WhatSon_daemon`)
- A Rust CLI launcher (optional target)
- A test suite validating file-format parsing, runtime stores, hierarchy view-model behavior, and QML wiring contracts

At runtime, the app auto-discovers the first `blueprint/*.wshub` package, parses hierarchy domains in worker threads,
applies snapshots to dedicated view-models on the main thread, and exposes those view-models to QML through context
properties.

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
    - mobile/export helpers for iOS/Android

Dependency discovery baseline:

- Qt 6.5 (`Core` at root, `Quick`/`QuickControls2` in app target)
- LVRS via `find_package(LVRS CONFIG REQUIRED)`
- LVRS default prefix fallback: `~/.local/LVRS` (if `LVRS_PREFIX` is not set)

## 2.2 App Target (`src/app/CMakeLists.txt`)

Key behaviors:

- Recursive source/header/QML collection
- `qt_add_qml_module` for all QML under `src/app/qml/**`
- `lvrs_configure_project_defaults(...)` + `lvrs_configure_qml_app(WhatSon)`
- Apple-specific framework linkage (EventKit, Photos, ApplicationServices on macOS host)
- Host/iOS LVRS module fallback/overlay logic for runtime QML import stability

This confirms LVRS is not a style add-on; it is a first-class runtime dependency in build and QML module configuration.

## 2.3 Daemon Target (`src/daemon/CMakeLists.txt`)

- Single executable target with Qt Core linkage
- Healthcheck mode prints `status=ok`
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
- `LibraryHierarchyViewModel` composes All/Draft/Today buckets and note list model synchronization
- `LibraryAll` body parser extracts `bodyPlainText` and `bodyFirstLine` from `.wsnbody` `<body>` content, strips inline
  tags
  (including custom tags such as `<Bold>`), and decodes XML entities before view-model consumption.
- Note primary text is derived from the top slice of `bodyPlainText`; when body text is empty it falls back to
  `noteId -> note directory stem`.
- `LibraryNoteListModel` now exposes note-card roles as a stable view contract:
    - `id` (string)
    - `primaryText` (string)
        - `folders` (`QStringList`)
        - `bookmarked` (bool)
        - `bookmarkColor` (hex string, bookmark icon tint)

## 4.4 QML View Layer (`src/app/qml/**`)

Primary root:

- `Main.qml` (`LV.ApplicationWindow`)
- App bootstrap (`main.cpp`) enables `WHATSON_DEBUG_MODE=1` by default when the variable is not explicitly set,
  so `WhatSon::Debug::trace/traceSelf`-based logs are available in development sessions without extra launch flags.
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
    - Toolbar icon order follows Figma node `134:4152`: `generalprojectStructure`, `chartBar`, `dataFile`,
      `generalhistory`, `cwmPermissionView`, `featureAnswer`.
        - `DetailContents` binds to C++-computed `activeStateName`.

Mobile composition:

- `MobileNormalLayout` via loader branch

Hierarchy rendering pipeline:

- `HierarchySidebarLayout` maps toolbar index to domain view-model
- `SidebarHierarchyView` handles search, selection, inline rename, create/delete folder, and toolbar event propagation
    - Hierarchy list data source is strictly `activeDomainViewModel.itemModel` (store-backed model path only).
      UI-side external depth/model injection is intentionally disabled to prevent arbitrary model substitution.
    - Chevron fold/unfold is handled by LVRS `HierarchyItem` (`expanded` state +
      `HierarchyList.notifyExpansionChanged`),
      and delegate row visibility/height reflects `HierarchyItem.rowVisible`.
        - Rename trigger policy: open text input overlay from selected item with `Enter/Return` key (double-click rename
          disabled)
        - Rename gating policy: QML checks per-item `canRenameItem(index)` from the active hierarchy view-model before
          opening the overlay. Global `renameEnabled` is treated as a fallback only.
        - Rename commit policy for hub-loaded hierarchies: view-model updates staged in-memory data, applies it to
          domain
          store, then calls store-driven file sync (`writeToFile`) for `*.wsfolders`, `*.wsresources`, `*.wsevent`,
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
    - `Folders.wsfolders` (hierarchical folder tree JSON; runtime computes depth from tree position)
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
- Runtime timer jobs are expected to perform periodic read/write cycles for frequently changing hub files (`.wslibrary`,
  `.wsresources`, `.wsstat`,
  and hierarchy sidecar files).
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

- Total QML files: 30
- Files declaring explicit `signal`: 20
- Files without explicit `signal`: 10

Files without explicit signal declarations:

- `src/app/qml/DesignTokens.qml`
- `src/app/qml/view/panels/DetailPanelLayout.qml`
- `src/app/qml/view/panels/ListBarLayout.qml`
- `src/app/qml/view/panels/ListItemsPlaceholder.qml`
- `src/app/qml/view/panels/MobileNormalLayout.qml`
- `src/app/qml/view/panels/NavigationBarLayout.qml`
- `src/app/qml/view/panels/NoteListItem.qml`
- `src/app/qml/view/panels/navigation/NavigationApplicationContentsBar.qml`
- `src/app/qml/view/panels/navigation/NavigationIconButton.qml`
- `src/app/qml/view/panels/navigation/NavigationInformationBar.qml`

Implication:

- If the rule is interpreted strictly as "every view must define explicit signal and slot-like handlers," these files
  are the current enforcement gap.

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

Status update (2026-03-01):

- The prior `Main.qml` contract drift around mobile/desktop branching has been resolved.
- `Main.qml` now uses:
    - `readonly property string activeMainLayout`
    - loader source routing based on `activeMainLayout`
- This removes dependence on legacy helper symbols and keeps desktop/mobile branching in one binding axis.
- `HierarchySidebarLayout.qml` now returns a normalized valid index explicitly in `normalizeHierarchyIndex(...)`.
- This prevents toolbar routing fallback to a single default hierarchy view-model.
- Hierarchy model `ShowChevronRole` is now derived dynamically from depth adjacency at data-read time.
- Chevron visibility contract is parent-only: leaf items must not render chevrons regardless of serialized/static
  `showChevron` values.

---

## 9. Architecture Risks and Technical Debt

1. String-literal contract fragility in QML tests
    - String-based QML assertions can fail on refactors that preserve behavior but alter literal source text.

2. Toolbar index normalization regression risk
    - `HierarchySidebarLayout.qml` now returns an explicit normalized index for in-range values.
    - Any future removal of that explicit return can collapse routing back to a default hierarchy view-model.

3. Splitter clamp fragility if helper return is removed
    - `BodyLayout.qml` drag-resize math depends on finite occupied width.
    - `totalSplitterWidth()` must always return numeric width; removing return can silently disable edge drag resizing.

4. Build script complexity concentration
    - LVRS host/iOS fallback logic in app CMake is substantial and platform-conditional.
    - This area is high-impact for onboarding and CI reproducibility.

5. File format tolerance vs strictness
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
