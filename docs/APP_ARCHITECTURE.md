# WhatSon Application Architecture Analysis

Last Updated: 2026-03-14  
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
    - `whatson_export_binaries`
    - `whatson_package`
    - mobile/export helpers for iOS/Android

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
- Platform icon wiring from `resources/`:
    - macOS bundle embeds `AppIcon.icns`
    - iOS generates an Xcode app icon catalog from the shipped PNG variants, with PNG bundle fallback when needed
    - Android package staging overlays density-specific launcher icons from `resources/<density>/AppIcon.png`
    - Windows executable embeds `AppIcon.ico`
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
          one of `NavigationApplicationViewBar.qml`, `NavigationApplicationEditBar.qml`,
          or `NavigationApplicationControlBar.qml`.
        - The non-control application bars currently provide the shared baseline `NavigationPreferenceBar.qml`, matching
          the default preference controls shown in the control-mode layout.
        - `Main.qml` derives the desktop sidebar initial width from the effective hierarchy-toolbar width.
            - `Main.qml` owns the global `Tab` shortcut and cycles navigation mode only when the focused object is not a
              text
              input/editor (`text`, `cursorPosition`, `selectedText` focus contract).
        - `NavigationModeBar.qml` and `NavigationEditorViewBar.qml` render the current `Control` / `Plain` design
          case
          through those active state objects, but click selection is menu-driven through `LV.ComboBox` +
          `LV.ContextMenu`, not direct next-state cycling.
        - `NavigationAddNewBar.qml`, `NavigationApplicationControlBar.qml`, and `MobileNormalLayout.qml` all forward
          the same `create-note` view hook into `LibraryHierarchyViewModel::createEmptyNote()`.
        - `NavigationModeBar.qml`, `NavigationEditorViewBar.qml`, `ListBarLayout.qml`, and `MobileNormalLayout.qml`
          use LVRS label styles/theme tokens for local typography and avoid panel-local hardcoded font families or
          text-color literals.
            - `ListBarLayout.qml` now composes the dedicated `ListBarHeader.qml` frame (Figma node `134:3180`), where
              the
              search field keeps LVRS defaults and only applies the minimum inline-variant overrides through local
              header
              properties (`transparent` background, `18px` height, `7px/3px` insets, `12px` text line box) while using
              `LV.InputField.searchMode` to keep the built-in leading search icon. The trailing buttons use LVRS icon
              assets `cwmPermissionView` and `sortByType` without extra color-token overrides. The active query is
              pushed into the active domain note-list model (`LibraryNoteListModel` or `BookmarksNoteListModel`), so
              visible note cards filter against runtime-parsed note body text without reparsing `.wsnote` content on
              each edit. The inline search header also recenters the underlying `LV.InputField.inputItem` from its live
              `contentHeight`, which prevents `Space`/`Backspace` transitions from snapping the text/caret between two
              fixed vertical positions. `ListBarLayout.qml` now owns the note-list `ListView` directly, including the
              bidirectional selection bridge between the visible `ListView` and the active domain note-list model, so
              note taps and runtime selection changes stay aligned even when the active hierarchy domain swaps the
              underlying note-list model instance. The same layout also composes `FocusedNoteDeletionBridge`, which
              captures the visually focused note id on press and uses it for focused-list `Backspace`/`Delete`
              keystrokes before falling back to the bound note-list model. Destructive note deletion still stays
              delegated through `LibraryHierarchyViewModel` and executed in the file layer by
              `WhatSonHubNoteDeletionService`, while bookmark-specific state only mirrors the result. Note-card
              delegates bind `roleModel.<role>` directly from the runtime
              role object instead of routing every field through a dynamic extractor helper, which keeps the card data
              path
              simple and avoids silent blank bindings when helper functions disappear. Note drags are also app-wired
              rather than LVRS-blocked: delegates
              advertise `whatson.library.note` plus copy semantics and force `DragHandler` pointer takeover, while the
              note-card tap handler stays on `TapHandler.DragThreshold`, but press now only marks a transient visual
              candidate row and selection is committed on tap release, then reasserted once through a short-lived
              pending-selection replay so drag startup is not canceled by note-model refresh. The surrounding
              `ListView` also yields drag-scrolling while a note row is pressed, so the delegate `DragHandler` can
              start the `whatson.library.note` drag before the viewport steals the pointer. `SidebarHierarchyView.qml`
              now binds the higher-level LVRS `Hierarchy` surface directly to each domain view-model's standard
              `hierarchyModel` property, and the standard-contract pass leaves note drops plus folder reordering out of
              the mounted sidebar surface until those behaviors are reintroduced on top of the same direct model
              contract.
            - `ContentViewLayout.qml` is now only the panel wrapper for the center editor slot, while
              `view/content/editor/ContentsDisplayView.qml` implements the actual Figma `ContentsDisplayView` editing
              surface by composing dedicated SRP modules:
              `view/content/editor/ContentsGutterLayer.qml`,
              `view/content/editor/ContentsMinimapLayer.qml`, and
              `view/content/editor/ContentsDrawerSplitter.qml`. The editor keeps `LV.TextEditor` in the parent surface
              and exposes shared calculations for those child modules through explicit resolver bindings. The gutter
              computes visible line
              numbers from the same `editorText` source, cursor line, and editor render metrics. `wrapMode:
              TextEdit.Wrap` is enabled, but gutter numbering still stays on logical document lines by mapping each
              logical line start through `editorItem.positionToRectangle(...)`; wrapped visual rows therefore do not
              change the line-number sequence. The editor surface itself keeps Fill height even when the current body
              is empty, and the editable body block is top-left aligned with `48px` top inset plus `16px` horizontal /
              bottom inset, overriding the LVRS internal centered multi-line placement. Figma node `155:5345` is
              treated as the source of truth for
              the gutter token contract: `panelBackground04` surface, `#4E5157` inactive caption numbers, `#9DA0A8`
              active line number, `2px` horizontal frame inset, `x=14` line-number column origin, right-aligned number
              text mirrored against the editor's `16px` left inset, and the fixed `18px` icon-rail anchor at `x=40`.
              Figma node `155:5352` is treated as the source of truth for body typography (`LV.Theme.fontBody`, `12px`,
              medium, zero letter spacing) while the text selection highlight intentionally reuses the standard LVRS
              input accent (`LV.Theme.accent`) instead of a panel-local darker blue. `editorText` is projected from the
              selected note's `.wsnbody` `<body>`
              plain-text payload through the active note-list model's `currentBodyText`, and persisted back through the
              active hierarchy view-model's `saveBodyTextForNote(...)` after a short debounce so typing does not force
              a full `.wsnbody` rewrite and note-list refresh on every keystroke.
              If no concrete note is selected, the editor surface must not fabricate a fresh unsaved draft or a helper
              prompt. Instead, the whole center surface stays blank until selection resolves to a real note.
              The gutter depends on the logical-line offset array maintained by `ContentsLogicalTextBridge`; if that
              backend
              offset model breaks, the editor can still paint body text while the line-number column goes empty.
              The same rule now applies to every helper that feeds child view state into the gutter/minimap/editor
              shell: those helpers are treated as total functions and must return an explicit fallback value instead of
              leaking `undefined`.
              The MVVM side of the editor now delegates note-selection/count/persistence contracts to
              `ContentsEditorSelectionBridge`, logical-line parsing to `ContentsLogicalTextBridge`, external
              gutter-marker
              normalization to `ContentsGutterMarkerBridge`, and debounce/select-to-editor synchronization to
              `ContentsEditorSession.qml`. `ContentViewLayout.qml` still resolves the incoming content view-model and
              note-list model first, but `ContentsDisplayView.qml` now consumes those backend-derived capability flags
              and
              text metrics instead of recomputing them inline in QML.
              Cursor-line lookup and visible-line enumeration must use that offset table plus viewport-derived search
              instead of reparsing the entire body or rescanning line `1..N` for every edit.
              The first visible gutter row is derived by mapping the current viewport `contentY` through
              `logicalLineNumberForDocumentY(...)`, which keeps the lookup simpler while still matching the actual top
              visible logical text row.
              Because `LV.TextEditor` can settle its internal layout after note rebinding and surface re-entry, the
              gutter must also expose an explicit refresh revision with a short multi-pass timer; note switches,
              resurfacing, viewport changes, and late `contentHeight` updates should all request another sampling pass
              so the gutter rebinds to the editor's final geometry instead of staying stretched from an older note.
              The current-line gutter marker must follow the cursor's active visual row and occupied text height rather
              than the whole trailing fill area of the editor, otherwise the blue marker can incorrectly extend from
              the last logical line to the bottom of the panel.
              A right-side Xcode-style minimap is also attached to the same editor document. It resolves the internal
              `TextEditor` flickable, draws a compressed body-text silhouette on `Canvas`, and maps each bar through the
              editor's actual content height plus text-start offset instead of distributing rows evenly across the rail.
              That keeps short notes top-aligned and visually tied to the text body rather than the gutter marker lane.
              The minimap paint path now rebuilds actual wrapped visual-row segments from
              `TextEditor.positionToRectangle(...)`
              instead of stretching one logical-line rectangle across the whole wrapped block; each row is painted as a
              thinner centered stroke so the silhouette reads like text rather than a carved slab. Those rows are now
              packed with a fixed `1px` gap between bars so the minimap remains visually denser than the main text
              column instead of spreading rows apart across the whole rail.
              The minimap stays borderless and inline: the visible viewport is only a translucent fill without an
              outline, the current cursor line is reduced to the active text-line silhouette width, and click/drag plus
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
        - optional image mode: top row inserts the Figma `24px` `imageBox` before the text block and paints the first
          resource thumbnail over a neutral placeholder fill
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
- `Main.qml` now uses LVRS internal page-stack routing for the shell (`useInternalPageStack`, `pageInitialPath`,
  `pageRoutes`, `activePageRouter`) instead of a root-level ad-hoc loader deciding which top-level surface to mount.
- The current shell route is a single workspace page; that page decides between desktop and mobile shell composition
  from LVRS adaptive layout state (`adaptiveMobileLayout`), not from an independent root routing flag.
- `Main.qml` owns the application menu bar and exposes empty `File`, `Edit`, `View`, `Window`, and `Help` menus
  through a macOS-native `Qt.labs.platform.MenuBar`; each native menu keeps a disabled placeholder item so macOS does
  not collapse otherwise-empty top-level entries.
- `Main.qml` owns the root focus-dismiss policy: when LVRS global hit-test metadata reports a left-click on a blank
  background/container surface, the active focus chain is cleared.
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
    - Toolbar frame follows Figma node `142:4198`: `145x20`, `5px` inter-button spacing, and `20x20` icon-button cells
      with `16px` glyphs.
    - Toolbar icon order follows Figma node `142:4198`: `generalprojectStructure`, `chartBar`, `generaladd`,
      `toolwindowdependencies`, `toolWindowClock`, `featureAnswer`.
        - `DetailContents` binds to C++-computed `activeStateName`.

Mobile composition:

- `MobileNormalLayout` via loader branch

Hierarchy rendering pipeline:

- `HierarchySidebarLayout` forwards toolbar activation into `SidebarHierarchyViewModel` and consumes the resolved
  hierarchy state it exposes
- `SidebarHierarchyView` now mounts the LVRS `Hierarchy` surface directly instead of composing a custom list/adapter
  stack around `HierarchyList`.
- Each domain hierarchy view-model exposes a direct `hierarchyModel` `QVariantList` property with LVRS-default roles
  (`itemId`, `key`, `label`, `depth`, `expanded`, `showChevron`), so the QML sidebar binds a standard LVRS tree model
  without an intermediate WhatSon bridge object.
- `SidebarHierarchyViewModel` is the single sidebar hierarchy state manager. `BodyLayout.qml` consumes
  `resolvedActiveHierarchyIndex`, `resolvedHierarchyViewModel`, and `resolvedNoteListModel` directly from that
  backend object, and `HierarchySidebarLayout.qml` forwards the same resolved hierarchy state into
  `SidebarHierarchyView.qml` without re-normalizing indices or re-resolving per-domain view-models in QML.
- `BodyLayout.qml` keeps width clamp math plus panel arrangement, while `PanelEdgeSplitter.qml` owns the shared
  edge-drag interaction used by sidebar/list/right-panel splitters.
- `Main.qml` keeps shell routing/window composition, while `MainWindowInteractionController.qml` owns focus
  dismissal, resize render-quality policy, and global navigation shortcut policy.
- Chevron fold/unfold is handled directly by LVRS `Hierarchy`/`HierarchyList` through the serialized
  `expanded`/`showChevron` roles emitted by each hierarchy view-model.
- Row-activation policy: LVRS `Hierarchy.listItemActivated(...)` is authoritative. WhatSon mirrors the resulting
  `itemId` back into the active hierarchy view-model through `setSelectedIndex(...)` and replays selection with
  `activateListItemById(...)` when the underlying model changes.
- The current standard-contract pass intentionally leaves custom sidebar search, note-drop handling, and editable
  hierarchy persistence out of the mounted QML surface. Those behaviors are deferred until they can be reintroduced on
  top of the direct LVRS model contract instead of replacing it.
- Blank-area deselect is no longer a WhatSon-owned hierarchy contract. The direct LVRS list keeps one active keyed row
  while selection is present.
- Create-folder focus policy: footer-triggered folder creation re-activates the inserted hierarchy row, and newly
  inserted folders use the placeholder label `Untitled`.
- Drag-reorder policy: the standard-contract sidebar pass does not mount editable hierarchy mutation in QML. Legacy
  `applyHierarchyNodes(...)` entrypoints remain in the domain view-models for later custom-domain reintegration.
- Library note-drop policy: the standard-contract sidebar pass does not mount note-drop overlays in QML. Legacy
  `LibraryHierarchyViewModel::assignNoteToFolder(...)` remains available for later reintegration.
- Library folder move persistence: `LibraryHierarchyViewModel::applyHierarchyNodes(...)` still owns canonical
  `Folders.wsfolders` rewrites plus note-header `<folders>` normalization when custom hierarchy mutation is restored.
- Projects folder move persistence: `ProjectsHierarchyViewModel::applyHierarchyNodes(...)` still owns
  `Folders.wsfolders` rewrites when custom hierarchy mutation is restored.
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

- Total QML files: 46
- Files declaring explicit `signal`: 39
- Files without explicit `signal`: 7

Files without explicit signal declarations:

- `src/app/qml/DesignTokens.qml`
- `src/app/qml/view/panels/navigation/NavigationIconButton.qml`
- `src/app/qml/window/DebugConsole.qml`
- `src/app/qml/window/Preference.qml`
- `src/app/qml/window/ProfileControl.qml`
- `src/app/qml/window/QuickNote.qml`

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
    - `pageInitialPath: workspaceRoutePath`
    - `pageRoutes: [workspaceShellRoute]`
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
