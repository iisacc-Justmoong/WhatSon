# WhatSon

WhatSon is an LVRS-based Qt Quick application.

## Structure

- `src/app`: LVRS-based UI application
- `src/daemon`: background daemon skeleton

## Adaptive Layout

- `src/app/qml/Main.qml` now mounts the root shell through LVRS `ApplicationWindow` page-stack APIs:
  `useInternalPageStack: true`, `pageInitialPath: "/"`, and an explicit routed workspace entry in `pageRoutes`.
- The routed workspace page still composes WhatSon's custom desktop/mobile shells, but layout selection now hangs off
  LVRS adaptive shell state (`adaptiveMobileLayout`) instead of a root-level ad-hoc loader branch.
- `Main.qml` also registers the internal router as the global navigator so later shell-level route changes flow
  through LVRS `Navigator` / `PageRouter` semantics instead of bespoke root state.
- `src/app/qml/Main.qml` also declares the application menu bar at the root window level with empty `File`, `Edit`,
  `View`, `Window`, and `Help` menus; macOS uses `Qt.labs.platform.MenuBar`, and each otherwise-empty native menu
  keeps a disabled placeholder row so the top-level titles remain visible in the global menu bar.

## Search Input Behavior

- Status bar search uses `LV.InputField` in `searchMode` and exposes editable state via QML properties/signals.
- Note-list search uses `LV.InputField` in `searchMode` and forwards the active query into the bound
  domain-specific note-list model (`LibraryNoteListModel` for Library, `BookmarksNoteListModel` for Bookmarks).
  Filtering is performed against runtime-parsed note body text assembled from `.wsnbody` `<body>` content instead of
  reparsing `.wsnote` files on every keystroke.
- The note-list search header also recenters the underlying LVRS `inputItem` from its live `contentHeight`, so
  whitespace-only edits (`Space`, `Backspace`) no longer flip the inline text/caret between two fixed vertical
  positions on macOS.
- Library folder filtering resolves nested note membership against the active hierarchy, so child-folder selection can
  still match notes whose header stores folder ancestry as separate `<folder>` entries or leaf-only values such as
  `/Competitor` instead of the full `Research/Competitor` path.

## Hierarchy Interaction

- `SidebarHierarchyViewModel` is the single sidebar hierarchy state manager. `BodyLayout.qml` and
  `HierarchySidebarLayout.qml` no longer normalize or resolve hierarchy state locally; they consume
  `resolvedActiveHierarchyIndex`, `resolvedHierarchyViewModel`, and `resolvedNoteListModel` directly from that shared
  backend object before passing child bindings down into the sidebar, note list, and editor.
- Sidebar rendering now mounts LVRS `Hierarchy` directly. Each domain hierarchy view-model exposes a standard
  `hierarchyModel` property with LVRS-default roles (`itemId`, `key`, `label`, `depth`, `expanded`, `showChevron`),
  and `SidebarHierarchyView.qml` binds that model without an intermediate adapter.
- Row activation now follows the LVRS `Hierarchy.listItemActivated(...)` contract directly. WhatSon mirrors the
  resulting `itemId` back into the active hierarchy view-model through `setSelectedIndex(...)` and replays the
  selection with `activateListItemById(...)` when the domain model changes.
- The standard-contract pass intentionally removes sidebar-specific hierarchy search, note-drop overlays, and editable
  reorder persistence from the mounted QML surface. Those custom domain behaviors can be reintroduced later on top of
  the LVRS-standard model contract instead of replacing it.
- Note-card selection still uses `TapHandler.DragThreshold` for drag coexistence, but press now only marks a transient
  visual candidate row; the authoritative note selection is committed on tap release and reasserted once on the next
  event turn so drag startup is not canceled by note-model refresh.
- The note list now follows the same rule for pointer ownership: while a note row is pressed, `ListView` temporarily
  stops drag-scrolling so the delegate `DragHandler` can win first and begin the `whatson.library.note` drag cleanly.
- Newly created folders now start with the placeholder label `Untitled` instead of sequence-based labels such as
  `Folder1`.

## Content Editor Surface

- `src/app/qml/view/panels/ContentViewLayout.qml` is now a panel-level wrapper only; the actual Figma
  `ContentsDisplayView` editor implementation lives in
  `src/app/qml/view/content/editor/ContentsDisplayView.qml`.
- The editor text is sourced from the active note-list model's selected note body, which is the parsed plain-text
  payload extracted from `.wsnbody` `<body>` content.
- The left `74px` gutter is driven from the same `editorText` source as the editor itself, so line numbers react to
  the same logical document and current cursor line.
- That gutter depends on the logical-line offset model maintained by `ContentsLogicalTextBridge`; when that backend
  offset model is broken, the editor text can still render while every visible gutter number disappears.
- The editor-view helper contract is intentionally strict now: helper functions that feed gutter/minimap/layout state
  must behave like total functions with an explicit return value on every success path and every fallback path. QML
  silently propagates `undefined`, and that can blank the gutter while leaving the rest of the editor visible.
- The same rule now applies to MVVM-fed bindings: views should not scatter direct `viewModel.foo !== undefined` checks
  across delegates. Normalize the incoming model/view-model once, then bind children only to the resolved contract.
- The gutter now follows the Figma `ContentsDisplayView` token contract directly: `panelBackground04` background,
  `#4E5157` inactive caption line numbers and `#9DA0A8` active line number.
- The line-number text is right-aligned against the same `16px` inset used by the editor body, so the gutter numbers
  terminate on the same vertical edge that the editable text begins from.
- The gutter/editor stack also preserves the Figma internal geometry: `2px` horizontal frame inset, line-number column
  anchored from `x=14`, and the fixed `18px` icon-rail anchor at `x=40`.
- The editor surface keeps Figma-style Fill height even when the body text is empty, and the editable text block is
  top-left aligned with `48px` top padding plus `16px` horizontal / bottom padding instead of vertical centering.
- `LV.TextEditor` disables rendered preview output and forced wrap defaults
  (`showRenderedOutput: false`, `enforceModeDefaults: false`) while enabling `wrapMode: TextEdit.Wrap`; the gutter
  still tracks logical `.wsnbody` lines through `positionToRectangle(...)`, so wrapped visual rows do not renumber the
  document.
- `LV.TextEditor` now binds the body token more explicitly for this surface: `LV.Theme.fontBody`, `12px` medium weight,
  zero letter spacing, and the standard LVRS input selection highlight (`LV.Theme.accent`), matching the Figma `Body`
  token and the rest of the app's input controls.
- `LibraryNoteListModel` and `BookmarksNoteListModel` now carry each note's full `bodyText` plus current selection
  state (`currentIndex`, `currentNoteId`, `currentBodyText`) so the list pane and editor pane stay synchronized
  without cross-domain model reuse.
- `SystemCalendarStore` now reads the active system locale, UI languages, territory, time zone, first weekday, and
  localized short/long date patterns from `src/app/calendar/SystemCalendarStore.*`. `Library` and `Bookmarks`
  note-list dates are formatted through that shared store instead of a hardcoded `yyyy-MM-dd` contract, and
  `NoteListItem.qml` falls back to the store's short-date placeholder text when a card has no display date, with a
  generic `Date` label only as the last-resort UI fallback when that store is unavailable.
- `NoteListItem.qml` now also accepts `image` + `imageSource` roles. When a `.wsnbody` `<resource ...>` entry exists,
  the note card keeps the Figma `24px` `imageBox` at the left edge of the top row and uses the first resource's
  resolved thumbnail URL as the preview source.
- `ListBarLayout.qml` now owns the note-list `ListView` directly, including the bidirectional selection bridge between
  `ListView.currentIndex` and the active domain note-list model; note-card taps must update the model selection, and
  model changes must resync the visible current row immediately when the active hierarchy domain changes.
- `ListBarLayout.qml` also keeps a short-lived pending note-selection intent and replays it once via `Qt.callLater`,
  so rapid note changes still win even if the previously focused editor is flushing a debounced body save.
- The same `ListBarLayout.qml` now composes a narrow `FocusedNoteDeletionBridge`, so `Backspace` / `Delete` resolve
  the visually focused note id directly from the list view before falling back to the active note-list model. The
  bridge still forwards deletion into the injected `LibraryHierarchyViewModel::deleteNoteById(...)` contract, and that
  view-model remains a thin wrapper around `WhatSonHubNoteDeletionService`. File-system integrity repair now lives
  under `src/app/file/validator/`: `WhatSonHubStructureValidator` resolves hub/library/stat paths,
  `WhatSonNoteStorageValidator` resolves materialized `.wsnote` / `.wsnhead` storage, and
  `WhatSonLibraryIndexIntegrityValidator` owns orphan pruning plus `index.wsnindex` rewrites. When a stale
  `index.wsnindex` entry no longer has a materialized `.wsnote`, load-time indexing now prunes the orphan from both
  the visible note set and the rewritten index, and the delete service also treats such entries as index-only cleanup
  instead of failing for a missing directory.
  `BookmarksHierarchyViewModel` only mirrors the deletion into its bookmarked subset.
- The note-card delegate reads `model.<role>` directly from the runtime role object instead of passing every field
  through a dynamic role-extraction helper, which keeps note preview bindings simpler and avoids silent blank-card
  regressions when a helper is removed.
- `SidebarHierarchyView.qml` no longer composes a local hierarchy interaction engine or an LVRS adapter bridge. It
  mounts `LV.Hierarchy` directly and consumes the active view-model's standard `hierarchyModel` property.
- `ContentsDisplayView.qml` now composes four narrow editor helpers instead of one god-object bridge:
  `ContentsEditorSelectionBridge` for note selection/count/persistence contracts,
  `ContentsLogicalTextBridge` for logical-line parsing, `ContentsGutterMarkerBridge` for gutter-marker normalization,
  and `ContentsEditorSession.qml` for debounce plus selection-to-editor text synchronization. The visual surface keeps
  only editor-geometry sampling and render placement.
- Body persistence still flows through the active hierarchy view-model's `saveBodyTextForNote(...)` contract with a
  short idle debounce, so `.wsnbody` rewrites and note-list refreshes no longer happen on every keystroke.
- When no note is selected, `ContentsDisplayView.qml` no longer pretends that an unsaved draft exists and does not
  return a synthetic editor prompt. The center surface simply stays empty until a concrete note selection exists.
- Full `bodyText` is preserved as normalized plain text rather than trimmed display text, so leading/trailing blank
  lines survive editor round-trips into `.wsnbody`.
- Gutter cursor-line lookup now comes from the prebuilt logical-line offset table and visible-range lookup starts from
  the current editor viewport offset instead of rescanning every logical line from the top on each paint.
- The first visible gutter line is derived by mapping the current viewport `contentY` back through
  `logicalLineNumberForDocumentY(...)`, which keeps the line-number model simpler while still matching the top visible
  logical document line.
- The gutter also keeps an explicit refresh-revision pulse with a short multi-pass timer, so when a user re-enters the
  editor surface, opens a different note, or the `TextEditor` finishes a delayed relayout, the visible line numbers
  and current-line markers are resampled from the settled editor geometry instead of stretching stale positions from a
  previous note/session.
- The blue current-line gutter marker is bound to the cursor's active visual row, so the marker no longer stretches
  through the whole remaining editor height when the cursor sits on the last logical line.
- The editor surface now also exposes a right-side Xcode-style minimap, but it is rendered as a borderless inline text
  silhouette instead of a framed rail. Its bar positions come from the editor's real content height and text-start
  offset, so short notes stay top-aligned and the minimap reflects the text body rather than gutter markers.
- That silhouette is now painted from actual wrapped visual-row segments taken from the `TextEditor` layout rather than
  from one height-scaled logical-line block, so wrapped paragraphs appear as separate thin text strokes instead of
  carved slabs.
- Minimap rows are packed with a fixed `1px` gap between bars, which keeps the overview denser than the main editor and
  prevents sparse empty rails from making the minimap appear longer than the body it represents.
- The current cursor line on that minimap is reduced to the active line's own silhouette width, and the visible
  viewport is shown as a subtle translucent fill without an outline border. Click/drag plus wheel-routed scrolling are
  still supported.
- The left marker rail is state-driven: the current cursor line is blue (`LV.Theme.primary`), lines changed in the
  current session are yellow (`#FFF567`), and externally supplied sync-conflict ranges are red (`LV.Theme.danger`).
- Conflict detection and sync integration are not implemented yet, but `ContentsDisplayView.qml` already accepts
  external gutter marker ranges via `gutterMarkers` using `{ type: "changed" | "conflict", startLine, lineSpan }` or
  `{ type, startLine, endLine }`.
- The QML safety guard now scans for two recurring corruption patterns as part of the test suite: standalone string
  literals inside `Binding {}` blocks and standalone dotted expressions such as `noteListItem.imageSource` that should
  have been property assignments. Critical `ContentsDisplayView.qml` helper bodies are also asserted to keep their
  explicit `return` statements.
- The same guard suite now also checks the centralized MVVM contract boundaries for the data-driven views: sidebar
  hierarchy state must stay anchored in `SidebarHierarchyViewModel`, hierarchy rendering must keep using the LVRS
  `Hierarchy` surface plus each domain view-model's direct `hierarchyModel` property instead of falling back to local
  compatibility wrappers or adapter layers, and
  editor-side selection/persistence/text/gutter contracts must stay split across the dedicated editor adapters instead
  of collapsing back into one bridge.
- `tests/app/test_solid_architecture_contracts.cpp` now locks those shell/sidebar/editor boundaries as SOLID-facing
  contracts: sidebar state must stay single-sourced behind the interface-driven `SidebarHierarchyViewModel`, editor
  adapters must stay role-segregated, and the QML assembly must keep dedicated interaction/session/helper objects
  instead of collapsing back into multi-responsibility controllers.

## Theme Token Usage

- `NoteListItem.qml`, `ListBarLayout.qml`, `MobileNormalLayout.qml`, `NavigationModeBar.qml`, and
  `NavigationEditorViewBar.qml` consume LVRS theme tokens for label typography and accent colors.
- Local hardcoded font-family names and ad-hoc RGBA/hex UI colors are not part of the view contract for those panels;
  use `LV.Label`, `LV.Theme.fontBody`, and the matching `LV.Theme` color tokens instead.

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
- Linux: `~/.local/LVRS/platforms/linux`
- iOS: `~/.local/LVRS/platforms/ios`
- Android: `~/.local/LVRS/platforms/android`
- WASM: `~/.local/LVRS/platforms/wasm` (if installed)

## Build

```bash
cmake -S . -B build
cmake --build build -j
```

If Qt is not auto-discovered on a native desktop build, pass the kit prefix explicitly:

```bash
cmake -S . -B build -DQT_ROOT_PATH=/absolute/path/to/Qt/kit
```

## Developer Tooling

- Root CMake now exposes developer quality targets:
    - `whatson_qmllint`: runs `qmllint` over `src/app/qml/**` with LVRS/Qt/build import paths configured.
    - `whatson_qmlformat_check`: verifies `src/app/qml/**` formatting without modifying files.
    - `whatson_qmlformat_fix`: rewrites `src/app/qml/**` in-place with `qmlformat`.
    - `whatson_clang_tidy`: runs `clang-tidy` against configured C++ translation units using
      `build/compile_commands.json`.
    - `whatson_dev_checks`: default aggregate target (`qmllint` + `qmlformat_check`, plus `clang-tidy` when installed).
- `qmllint` is intentionally configured to fail on syntax/import errors while tolerating the repository's current
  warning
  baseline, so it is immediately useful for catching broken QML without forcing a full warning cleanup first.
- `whatson_clang_tidy` reads repository policy from [.clang-tidy](/Volumes/Storage/static/Product/WhatSon/.clang-tidy).
- If a tool is missing at configure time, the corresponding target still exists but fails with an explicit installation
  hint when invoked.

## Unified Root CMake Targets

Platform build, launch, export, and package targets are centralized in the root `CMakeLists.txt`.

```bash
cmake --build build --target whatson_build_all
cmake --build build --target whatson_run_app
cmake --build build --target whatson_healthcheck_daemon
cmake --build build --target whatson_export_binaries
cmake --build build --target whatson_package
```

On native desktop host builds, `whatson_export_binaries` now stages a self-contained install tree under `build/dist`
via `cmake --install`. The same deployment path is used by:

```bash
cmake --install build --prefix /absolute/output/prefix
```

Desktop install/export layout:

- macOS: `build/dist/WhatSon.app` at the install prefix root, with deployed Qt/LVRS runtime content inside the bundle
- Windows: `build/dist/bin/WhatSon.exe` plus deployed Qt/LVRS runtime DLLs, `qml/`, `plugins/`, and `qt.conf`
- Linux: deployed Qt/LVRS runtime libraries, QML imports, the desktop entry, and the app icon metadata

Platform icon packaging is resolved from `resources/` during configure time:

- macOS bundle icon: `resources/AppIcon.icns`
- iOS app icon set: generated Xcode asset catalog from the iPhone/iPad PNG variants under `resources/`
- Android launcher icon: density-specific `resources/<density>/AppIcon.png` files overlaid into the Android package
- Windows executable icon: `resources/AppIcon.ico`
- Linux desktop/package icon: `resources/AppIcon.png`

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
./scripts/bootstrap_whatson.sh host
./scripts/bootstrap_whatson.sh macos
./scripts/bootstrap_whatson.sh linux
./scripts/bootstrap_whatson.sh ios
./scripts/bootstrap_whatson.sh android
```

`host` resolves to the current desktop platform (`macos` or `linux`).
On Linux hosts, `all` builds the Linux bootstrap target first and then attempts Android only when the Android NDK is
installed.

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
- Apple permission bridge source guard against free-function `traceSelf(this, ...)` misuse
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
./build/src/app/bin/WhatSon 2>&1 | rg "\\[whatson:debug\\]|\\[wsnhead:index\\]"
# macOS bundle path equivalent:
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
- The navigation add surfaces share one `create-note` hook path:
  `NavigationAddNewBar.qml`, `NavigationApplicationControlBar.qml`, and `MobileNormalLayout.qml`
  all route into `LibraryHierarchyViewModel::createEmptyNote()`.
- Navigation mode state is centralized in `src/app/viewmodel/navigationbar/NavigationModeViewModel.*`:
  `main.cpp` injects `navigationModeViewModel`, and the navigation bar mode combo binds to the dedicated enum-backed
  `View/Edit/Control/Presentation` state plus its per-mode QObject viewmodels.
- Editor view mode state is centralized in `src/app/viewmodel/navigationbar/EditorViewModeViewModel.*`:
  `main.cpp` injects `editorViewModeViewModel`, and the navigation bar editor-view combo binds to the dedicated
  enum-backed `Plain/Page/Print/Web/Presentation` state plus its per-view QObject viewmodels.
- The sidebar initial width now follows the effective rendered width of the hierarchy toolbar.
- Figma navigation frames are split into dedicated QML files under `src/app/qml/view/panels/navigation/`:
  `NavigationPropertiesBar.qml`, `NavigationInformationBar.qml`, `NavigationModeBar.qml`,
  and `NavigationEditorViewBar.qml`.
- The right-side application area is also split by navigation mode into dedicated QML files:
  `NavigationApplicationViewBar.qml`, `NavigationApplicationEditBar.qml`,
  `NavigationApplicationControlBar.qml`, and `NavigationApplicationPresentationBar.qml`.
- `NavigationApplicationViewBar.qml`, `NavigationApplicationEditBar.qml`, and
  `NavigationApplicationPresentationBar.qml` mount the same baseline `NavigationPreferenceBar.qml` used by the
  control mode until mode-specific tools are added.
- `Main.qml` binds a global `Tab` shortcut that cycles `View/Edit/Control/Presentation` only when no text input or
  text editor currently owns focus.
- `Main.qml` also listens to LVRS global press hit-tests and clears the current focus chain when a left-click lands on
  an empty background/container surface instead of an interactive control.
- `NavigationModeBar.qml` and `NavigationEditorViewBar.qml` use `LV.ComboBox` as the trigger surface and open
  `LV.ContextMenu` enumerations on click instead of cycling state directly.
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
  `LibraryNoteListModel`, `BookmarksNoteListModel`) now expose validation hooks for backend/UI interception:
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
- Library sidebar filtering uses the persisted folder `id`/path as the canonical scope key, not the display `label`,
  so duplicate leaf labels under different parents stay disambiguated.
- Runtime-injected `All Library`, `Draft`, and `Today` are explicit system buckets; user-created folders such as
  `All` remain regular editable folders.
- Legacy list/object formats are still accepted and normalized into runtime depth entries.

Library runtime classification behavior:

- `All`: indexes `.wsnindex` entries and enriches them with `.wsnhead` metadata (`id`, created/modified
  timestamps, and related fields)
- `All`: reads each note's `.wsnbody`, extracts text inside `<body>...</body>`, and uses only that body text as
  note-list summary text; blank bodies stay visually blank instead of falling back to internal IDs or filesystem stems
- Notes whose header resolves to no folder labels are treated as `Draft` for user-facing note-list metadata, so the
  note card renders the folder row as `Draft` and the immutable `Draft` hierarchy bucket stays visually consistent with
  the same classification rule.
- `All`: detects the first non-text `<resource ...>` entry in `.wsnbody`, resolves its thumbnail path against the note
  directory / hub root, and exposes that preview to the note-list card
- `All`: scans both fixed `Library.wslibrary` and dynamic `*.wslibrary` roots under each `*.wscontents`
- `All`: `LibraryHierarchyViewModel::createEmptyNote()` creates a blank note directly under the active `.wslibrary`
  with a mixed-case alphanumeric `16-16` ID, persists the header/body/link/attachment scaffold through the existing
  note creators, updates `index.wsnindex` plus hub stat metadata, and keeps the new note selected in the current scope
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

Native build parallelism is now bounded by default to `min(cpu_count, 8)`.
Use `--jobs <N>` to override the total job budget. `--parallel` splits that budget across active platform tasks, and the
same limit is forwarded to downstream native builders such as `cmake --build`, `xcodebuild`, and Android `gradlew`.
All build scripts now emit structured debug snapshots as `[state] {json}` lines so long-running phases, selected
devices, command start/finish, and task results can be captured directly from stdout.

Run a single platform script directly:

```bash
python3 scripts/build_host.py
python3 scripts/build_android.py
python3 scripts/build_ios.py --ios-device "<UDID-or-Device-Name>"
python3 scripts/build_host.py --jobs 4
```

Task selection through orchestrator:

```bash
python3 scripts/build_all.py --tasks host,android,ios
python3 scripts/build_all.py --tasks host --no-host-run
python3 scripts/build_all.py --tasks ios --ios-device "<UDID-or-Device-Name>"
python3 scripts/build_all.py --tasks host,android,ios --jobs 6
python3 scripts/build_all.py --tasks host,android,ios --parallel
```

Example debug capture:

```bash
python3 scripts/build_all.py --tasks host,android --jobs 4 | rg '^\[state\]'
```

Behavior by OS:

- macOS: runs host + Android + iOS flows.
- Linux/Windows: iOS task is skipped automatically; host and Android flows still run.
- Headless Linux host sessions without `DISPLAY` and `WAYLAND_DISPLAY` automatically configure
  the full desktop app build, run daemon healthcheck, and then skip only the desktop app launch.

Logs are written to `build/automation-logs/*.log` by default.
Default artifacts are generated at:

- iOS Xcode project: `build/ios-xcode-artifact/WhatSon.xcodeproj`
- Android Studio project: `build/android-studio-artifact`
- Linux staged install tree: `build/dist`

The generated iOS configure path disables optional `Qt6GrpcQuick` / `Qt6ProtobufQuick`
package discovery because WhatSon does not use those modules and cross-compiling may
otherwise emit host `protoc` warnings.

You can override artifact locations:

```bash
python3 scripts/build_all.py \
  --ios-project-dir build/ios-xcode-artifact \
  --android-studio-dir build/android-studio-artifact
```

## Runtime Smoke Matrix

`scripts/runtime_smoke_matrix.py` provides an execution-focused verification layer on top of `build_all.py`.
It is intended to prove that the same UI codebase is built and launched across platforms with clean state.
It emits the same `[state] {json}` debug snapshots for each phase and command.

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
python3 scripts/runtime_smoke_matrix.py --jobs 4
python3 scripts/runtime_smoke_matrix.py --tasks host,android --skip-ios-smoke
python3 scripts/runtime_smoke_matrix.py --tasks ios --strict-ios-smoke
```

Outputs:

- Logs: `build/runtime-matrix-logs/*.log`
- Artifacts: `build/runtime-matrix-artifacts/`
