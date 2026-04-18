# WhatSon

WhatSon is an LVRS-based Qt Quick application.

## Structure

- `src/app`: LVRS-based UI application
- `src/daemon`: background daemon skeleton

## Verification Policy

- WhatSon maintains in-repo build and runtime regression gates under the root CMake targets and `test/`.
- `whatson_build_regression` is the maintained incremental build gate for product binaries plus the regression test
  executable inside `build/`.
- `whatson_regression` is the default combined verification target; `whatson_cpp_regression` remains the runtime-only
  Qt Test C++ regression entrypoint.
- Python test scripts under `scripts/test_*.py` remain removed; automated regression coverage lives under CTest and
  the C++ regression suite in `test/`.
- `scripts/build_*.py` and `scripts/runtime_smoke_matrix.py` remain optional developer automation utilities, not
  substitutes for the maintained regression gates.

## Adaptive Layout

- `src/app/qml/Main.qml` now mounts the root shell through LVRS `ApplicationWindow` page-stack APIs:
  `useInternalPageStack: true`, explicit routed onboarding/workspace entries in `pageRoutes`, and an app-owned
  `startupRoutePath` that now comes from `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.*` and is
  mirrored into both `initialRoutePath` and `pageInitialPath` before the first frame.
- The embedded mobile onboarding bootstrap state machine now lives in
  `src/app/viewmodel/onboarding/OnboardingRouteBootstrapController.*` instead of being open-coded across `Main.qml`.
  `Main.qml` only applies requested LVRS route changes and keeps the desktop onboarding subwindow separate from the
  mobile embedded route flow.
- Mobile onboarding success now defers the `/onboarding -> /` route flip with `Qt.callLater(...)` so iOS/Android native
  picker teardown can finish before the workspace shell replaces the onboarding surface.
- Android onboarding now routes `content://` SAF hub selections and destination directories through
  `src/app/platform/Android/WhatSonAndroidStorageBackend.*`. The backend keeps the OS-provided document URI as the
  source of truth, materializes the selected `.wshub` package into a deterministic app-local mount directory under app
  data, refreshes that mounted copy again from the stored source URI before startup runtime bootstrap, and now mirrors
  successful note-package writes from that mounted working copy back into the original SAF document tree.
- iOS onboarding now follows the same direct-filesystem vault principle used by Obsidian mobile inside the active app
  session: once Files grants access to a `.wshub`, `src/app/platform/Apple/AppleSecurityScopedResourceAccess.*`
  opens a security-scoped resource session and the runtime reads and writes the real `.wshub` directory in place
  instead of going through a copied mount. WhatSon also persists an implicit iOS bookmark for that Files-backed hub
  scope and restores it during startup preflight before deciding whether onboarding is needed.
- The onboarding controller now preserves the raw fully-encoded Android picker URI before any desktop path
  normalization runs, so opaque SAF providers still reach the Android document backend instead of being rejected as
  non-mountable local paths.
- `src/app/qml/Main.qml` also disables LVRS `mobileOversizedHeightEnabled` for this app root. The default oversized
  mobile window strategy paints large top/bottom fill regions with `windowColor`, which pushed the routed onboarding
  page host outside the visible first-frame viewport and produced an apparently blank screen on iOS/Android.
- `src/app/qml/Main.qml` opts mobile back into LVRS full-window content coverage by disabling
  `delegateMobileInsetsToSystem` and forcing `forceFullWindowAreaOnMobile`. WhatSon therefore owns the visible top and
  bottom safe-area bands as part of the routed app surface instead of painting a background-only fallback outside the
  live content root.
- `src/app/qml/MainWindowInteractionController.qml` now keeps the resize-time dynamic-resolution suspend/resume guard
  desktop-only. iOS startup no longer flips `LV.RenderQuality.dynamicResolutionEnabled` during the first geometry churn,
  which reduces swapchain recreate pressure while the Metal command queue still owns the previous multisample targets.
- `src/app/qml/Main.qml` now also watches the embedded mobile onboarding/workspace route host. If LVRS exposes the
  expected route path but the current page item never materializes, the root forces a route rebuild and paints a
  temporary fallback surface instead of leaving the device on a black blank frame.
- `main.cpp` now re-acquires iOS Files access during startup mount preflight before deciding whether onboarding is
  needed. The stored `.wshub` path is checked through `AppleSecurityScopedResourceAccess::ensureAccessForPath(...)`
  first, and its parent directory is retried as a fallback, so reopening the app no longer falls back to onboarding
  merely because the new iOS session starts with an empty in-memory access registry.
- `src/app/qml/Main.qml` now keeps LVRS shell render quality in auto-detect mode
  (`forcedDeviceTierPreset: -1`). Recent LVRS bootstrap profiles already dropped the old `UltraTier` shell default, so
  WhatSon must not keep the stale mobile `HighTier` override.
- The routed workspace page still composes WhatSon's custom desktop/mobile shells, but layout selection now hangs off
  LVRS adaptive shell state (`adaptiveMobileLayout`) instead of a root-level ad-hoc loader branch.
- `Main.qml` also registers the internal router as the global navigator so later shell-level route changes flow
  through LVRS `Navigator` / `PageRouter` semantics instead of bespoke root state.
- `Main.qml` now keeps `desktopPanelSurfaceColor` as the single desktop panel-surface token and binds it directly into
  desktop child panels instead of repeating one proxy color property per shell section.
- `Main.qml` now registers the mutable app-facing view-model objects with LVRS `ViewModels` and resolves its root
  aliases back through `LV.ViewModels.get(...)` before passing them into child objects. Shortcut writers also bind
  dedicated writable view ids (`windowInteractions.libraryNoteMutation`, `windowInteractions.navigationMode`,
  `windowInteractions.sidebarHierarchy`) so shortcut mutations no longer bypass the LVRS ownership registry.
- The desktop `BodyLayout.qml` now also paints a thin top border from the shared splitter token so the transparent
  content HStack still reads as a separate surface below `NavigationBarLayout.qml`.
- `src/app/qml/Main.qml` now lazy-loads the macOS-native menu bar via `Qt.resolvedUrl("window/MacNativeMenuBar.qml")`
  instead of holding a static `WindowView.MacNativeMenuBar` type reference in the root shell; this keeps the iOS
  static QML bundle from parsing the macOS-only `Qt.labs.platform` module while preserving the native `Onboarding`
  command under the `Window` menu.
- The iOS app target keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for Xcode export generation, so
  `src/app/CMakeLists.txt` explicitly links the static QML plugin targets for `QtQuick`, `QtQuick.Window`,
  `QtQuick.Layouts`, `QtQuick.Controls`, and `QtQuick.Dialogs`. Without those links, the standalone mobile
  onboarding path exits immediately with `module "QtQuick" plugin "qtquick2plugin" not found`.
- The desktop navigation bar's left/right edge buttons now toggle the hierarchy sidebar and detail panel directly.
  Collapse only zeros the live panel width; the last preferred widths are preserved so expanding restores the previous
  splitter geometry instead of resetting panel sizes.
- Navigation calendar actions now open a daily calendar view, weekly calendar view, monthly calendar view,
  and yearly calendar view in the editor's inline content slot, reusing shared calendar view-model backends.
- Calendar content surfaces now share a `CalendarBoardStore` backend so events and tasks can be created with explicit
  `date + time` arguments and projected back into day-cell counters across day/week/month/year views.
- The same calendar board now also projects library notes into a single calendar item per note. It anchors that item
  to whichever of `createdAt` or `lastModifiedAt` is more recent, so a note modified on `2026-04-08` appears in the
  April 8 calendar views alongside manual calendar event items instead of replacing them.
- Calendar note projection now refreshes from the live library runtime snapshot on startup and library-side note
  mutations, which prevents the calendar views from staying empty while still preserving the disk reindex fallback for
  bookmark/progress-originated note changes.
- `CalendarBoardStore` now maintains date-keyed entry/count indexes for both manual and projected items, so
  day/week/month/year viewmodels stop paying a full-board scan cost on every date query.
- The monthly calendar projection now carries per-day `entries` payloads into the day-cell model itself, so note
  lifecycle chips render visually inside the month grid instead of relying on a late re-query path.
- Calendar queries now also fall back to the live library note provider when the projected-entry cache is empty, so the
  visible calendar surfaces can still render note chips for dates like `2026-04-08` instead of showing only the day
  number grid.
- Projected note chips now label themselves with the same top-line body preview text used by `NoteListItem`, instead of
  prefixing the chip with lifecycle strings such as `Created note` or `Modified note`.
- Projected note chips in Agenda/day/week/month views can now reopen the backing library note in the editor on
  click/touch. Week view keeps that affordance only for slot chips that represent exactly one note entry, because the
  current `title +N` summary chip does not expose a unique note target.
- The monthly pager and month-grid delegates now resolve live projections/day-cells by index instead of holding stale
  JS `modelData` snapshots, which keeps notes on their real dates during initial month-view bootstrap instead of
  leaking them into adjacent disabled cells.
- Library runtime note snapshots now push directly into `CalendarBoardStore` through a dedicated
  `LibraryHierarchyViewModel` signal, so month-view note chips do not depend on a later UI-triggered rebuild such as
  pressing `Today`.
- Library note saves, metadata reloads, note creation, note deletion, and folder clears now prefer single-note
  indexed-state upsert/remove paths instead of replacing the entire runtime note snapshot, which reduces rebuild fanout
  into the note list and calendar projection surfaces.
- Navigation-driven calendar opens now reset Agenda/day/week/month/year overlays back to today's date context, while
  year-view drill-down into month view still preserves the explicitly tapped month/date.
- Day/Agenda/Week/Year calendar `request*View()` hooks are now log/hook-only and no longer rebuild their models after
  QML has already changed the displayed cursor, which removes the duplicate recalculation path when opening or moving
  those calendar routes.
- Week calendar no longer keeps a second QML-side date model/cache above `WeekCalendarViewModel`; the viewmodel now
  owns the lazy timeline date window and the page is limited to viewport synchronization and interaction.
- The desktop workspace shell now keeps the broad panel wrappers (`StatusBarLayout`, `NavigationBarLayout`,
  `HierarchySidebarLayout`, `ListBarLayout`, `ContentViewLayout`, `DetailPanelLayout`) transparent, so the root
  `LV.ApplicationWindow` `panelBackground01` canvas remains the only large desktop background surface.
- The LVRS derived surface aliases now stay on the lower-luminance Figma scale (`windowAlt -> panelBackground01`,
  `subSurface -> panelBackground02`, `surfaceSolid -> panelBackground03`, `surfaceAlt -> panelBackground04`) so
  editor, toolbar, and card defaults do not drift brighter than the `panelBackground01` desktop canvas.
- desktop status and sidebar search shells now stay transparent, and inactive hierarchy rows no longer repaint
  `panelBackground12`, so the sidebar/list/content split keeps reading as one shared `ApplicationWindow` canvas
  instead of several stacked panel fills.

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

## Detail Panel Surface

- The detail panel `fileStat` tab is now backed by `src/app/viewmodel/detailPanel/DetailFileStatViewModel.*` instead of
  a generic placeholder section object, so QML can bind directly to the persisted `.wsnhead <fileStat>` counters.
- `src/app/qml/view/panels/detail/DetailFileStatForm.qml` now follows the Figma `235:7734` plain-text layout instead of
  the earlier card grid, using `description` typography rows for project/folder/tag/date metadata and the tracked file
  statistics.
- The file-stat summary now uses the same cleared-project fallback as the project selector, so empty project metadata is
  shown as `No project` instead of `Untitled`.
- `src/app/qml/view/panels/detail/DetailPanel.qml` now passes that statistics object into
  `DetailContents.qml` through an explicit `fileStatViewModel` property instead of relying on a generic active-state
  object for the statistics tab.

## Hierarchy Interaction

- `SidebarHierarchyViewModel` is the single sidebar hierarchy state manager. `BodyLayout.qml` and
  `HierarchySidebarLayout.qml` no longer normalize or resolve hierarchy state locally; they consume
  `resolvedActiveHierarchyIndex`, `resolvedHierarchyViewModel`, and `resolvedNoteListModel` directly from that shared
  backend object before passing child bindings down into the sidebar, note list, and editor.
- The sidebar hierarchy seam is now interface-first: every domain hierarchy view-model implements
  `IHierarchyViewModel`, `HierarchyViewModelProvider` only accepts that interface, and note-list resolution no longer
  relies on raw `QObject` property introspection.
- Sidebar rendering now mounts LVRS `Hierarchy` directly. Each domain hierarchy view-model exposes a standard
  `hierarchyModel` property with LVRS-default roles
  (`itemId`, `key`, `label`, `depth`, `expanded`, `showChevron`, `draggable`), and keeps that payload as a flat
  depth-array because current LVRS `Hierarchy` / `HierarchyList` infer child presence, visibility, expand/collapse,
  and row-level drag gating from row order plus explicit `depth` / `draggable`. `SidebarHierarchyView.qml` binds that
  model without an intermediate adapter and now normalizes the incoming C++ `QVariantList` into a real JS array before
  it reaches LVRS editable drag logic.
- `IHierarchyViewModel` is now a read-oriented shared contract. Rename/create/delete/expand/reorder/note-drop moved
  behind dedicated capability interfaces, and QML consumes those write paths through `HierarchyInteractionBridge` plus
  `HierarchyDragDropBridge` instead of depending on one fat interface for every hierarchy screen.
- `SidebarHierarchyView.qml` no longer keeps rename, bookmark-palette, and note-drop controller logic in one root
  object. The mounted LVRS surface now composes `SidebarHierarchyRenameController.qml`,
  `SidebarHierarchyNoteDropController.qml`, and `SidebarHierarchyBookmarkPaletteController.qml` as sibling helpers
  while the root view stays focused on layout, shell state, and LVRS event wiring.
- Sidebar drag-reorder is now reintroduced through a dedicated system-level `HierarchyDragDropBridge` wired into
  LVRS `Hierarchy.editable` plus `listItemMoved(...)`, so folder tree mutation stays on the direct LVRS event path
  instead of reviving the old local interaction controller. The same mounted sidebar now also accepts
  `whatson.library.note` drops directly over hierarchy rows and routes accepted note-to-folder assignments back through
  the bridge, appending the dropped target hierarchy path into the note's `.wsnote` header `<folders>` list while
  preserving every existing folder assignment already known in the runtime note record or persisted header.
- `HierarchySidebarLayout.qml` and `SidebarHierarchyView.qml` no longer carry unused `frameName` / `frameNodeId`
  passthrough metadata, so the shared hierarchy surfaces expose only state that participates in runtime rendering or
  routing.
- Note-card selection still commits only on release, but pointer ownership now splits by platform: desktop keeps the
  `TapHandler.DragThreshold` plus immediate drag path, while mobile defers drag pickup to a dedicated `1000ms`
  long-press area so vertical list scrolling can win before the row starts a note drag.
- The note-card visual state now keeps a dedicated persistent `active` contract separate from transient `pressed`, so
  the committed current note remains visibly highlighted after the pointer is released and while the editor is showing
  that note.
- The note list now suspends viewport dragging only once a real note drag is active. A mere press keeps `ListView`
  scrollable, and on mobile the row must survive a `1000ms` hold before it is promoted into the shared
  `whatson.library.note` drag path.
- The same note delegates now use `Drag.Internal`, keep publishing `application/x-whatson-note-id` mime data, and
  mount an overlay-parented `NoteListItem` preview in the active visual state so the grabbed card itself follows the
  pointer instead of falling back to the platform note-id tooltip.
- While a note card is dragged across hierarchy folders, `SidebarHierarchyView.qml` now previews the hovered drop row
  with a pulsing active-style overlay before release, and desktop internal drags drive that same preview through the
  shared position-based note-drop helper instead of relying only on native `DropArea` traffic.
- The grabbed note card itself now drops to `25%` opacity, both for the moving overlay preview and the source row,
  so the hovered-folder highlight stays readable instead of being visually buried under the dragged note surface.
- Note-list snapshot refreshes are now deferred while a drag is active, so background `itemsChanged()` /
  `dataChanged()` churn does not tear down the grabbed delegate and abruptly cancel note-to-folder drags.
- `ListBarLayout.qml` now groups transient note-selection replay state and drag-preview state into dedicated local
  `QtObject` blocks (`noteSelectionState`, `noteDragPreviewState`) so the shared note-list surface keeps a smaller
  root property contract without changing the selection or drag behavior.
- `ListBarLayout.qml` now treats the bound note-list model as the authoritative selection source and snaps
  `ListView.currentIndex` back to the model value when reset-time view defaults try to synthesize row `0`, which keeps
  the initial workspace note list visually unselected until the user or a higher-level workflow picks a note.
- Note-card active styling is now bound only to that committed model selection, so touch press can still turn into a
  list scroll gesture on mobile without painting the row active before release.
- Mobile note-card drags now begin only after a `1000ms` hold, while desktop note drags stay immediate; `ListView`
  therefore remains scrollable on mobile until the long-press actually promotes the row into a drag.
- The note-card context menu now follows the same platform split: desktop still opens it immediately on right-click,
  while mobile only arms it after the same `1000ms` hold and opens it on release if that hold never turns into a drag.
- Library note-to-folder drop now treats a same-folder re-drop as an explicit no-op after reading the local
  `.wsnhead`, so debug traces can distinguish duplicate assignments from genuine routing failures without rewriting the
  header file unnecessarily.
- Newly created folders now start with the placeholder label `Untitled` instead of sequence-based labels such as
  `Folder1`.

## Content Editor Surface

- `src/app/qml/view/panels/ContentViewLayout.qml` is now a panel-level wrapper only; the actual Figma
  `ContentsDisplayView` editor implementation lives in
  `src/app/qml/view/content/editor/ContentsDisplayView.qml`.
- The navigation daily/weekly/monthly/yearly calendar actions now mount day/week/month/year calendar content directly
  in the `ContentDisplayView` slot of the editor surface, and the calendar pages are backed by shared context objects
  from `main.cpp`
  (`dayCalendarViewModel`, `weekCalendarViewModel`, `monthCalendarViewModel`, `yearCalendarViewModel`).
- `ContentViewLayout.qml` now also owns the shared "open note from calendar" bridge. It activates the requested note
  through `LibraryHierarchyViewModel::activateNoteById(...)`, switches the active hierarchy back to Library, and then
  dismisses the visible calendar overlay so the editor shows the selected note.
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
- The desktop editor panel and gutter fill now stay transparent so the same root
  `LV.ApplicationWindow` `panelBackground01` canvas shows through, while the gutter still keeps `#4E5157` inactive
  caption line numbers and `#9DA0A8` active line numbers from the Figma frame.
- The line-number text is right-aligned against the same `16px` inset used by the editor body, so the gutter numbers
  terminate on the same vertical edge that the editable text begins from.
- The gutter/editor stack also preserves the Figma internal geometry: `2px` horizontal frame inset, line-number column
  anchored from `x=14`, and the fixed `18px` icon-rail anchor at `x=40`.
- The editor surface keeps Figma-style Fill height even when the body text is empty, and the editable text block is
  top-left aligned with a shared `16px` top / horizontal / bottom inset instead of vertical centering.
- That shared top inset is now materialized through the custom `ContentsInlineFormatEditor.qml` wrapper around
  `QtQuick.TextEdit` while the internal `topPadding` stays forced to `0`, so the `16px` separation from the navigation
  bar remains visible even when the editor viewport recalculates under Fill sizing on both desktop and mobile.
- The wrapper disables rendered preview output and forced mode defaults where required while keeping
  `wrapMode: TextEdit.Wrap`; the gutter still tracks logical `.wsnbody` lines through `positionToRectangle(...)`, so
  wrapped visual rows do not renumber the document.
- The same `QtQuick.TextEdit`-backed wrapper binds the body token explicitly for this surface:
  `LV.Theme.fontBody`, `12px` host-driven weight/size policy, zero letter spacing, explicit `LV.Theme.bodyColor` text
  color, and the standard LVRS selection highlight (`LV.Theme.accent`), matching the Figma `Body` token and the rest
  of the app's input controls.
- `LibraryNoteListModel` and `BookmarksNoteListModel` now carry each note's full `bodyText` plus current selection
  state (`currentIndex`, `currentNoteId`, `currentBodyText`) so the list pane and editor pane stay synchronized
  without cross-domain model reuse.
- `SystemCalendarStore` now reads the active system locale, UI languages, territory, time zone, first weekday, and
  localized short/long date patterns from `src/app/calendar/SystemCalendarStore.*`. `Library` and `Bookmarks`
  note-list dates are formatted through that shared store instead of a hardcoded `yyyy-MM-dd` contract, and
  `NoteListItem.qml` falls back to the store's short-date placeholder text when a card has no display date, with a
  generic `Date` label only as the last-resort UI fallback when that store is unavailable.
- `NoteListItem.qml` now also accepts `image` + `imageSource` roles. When a `.wsnbody` `<resource ...>` entry exists,
  the note card expands the image variant to the Figma `126px` frame, keeps a `48px` `imageBox` at the left edge of
  the top row, anchors the two-line title block to the top-left of the text column, and uses the first resource's
  resolved thumbnail URL as the preview source.
- `ListBarLayout.qml` now owns the note-list `ListView` directly, including the bidirectional selection bridge between
  `ListView.currentIndex` and the active domain note-list model; note-card taps must update the model selection, and
  model changes must resync the visible current row immediately when the active hierarchy domain changes.
- `ListBarLayout.qml` also keeps a short-lived pending note-selection intent and replays it once via `Qt.callLater`,
  so rapid note changes still win even if the previously focused editor is flushing a debounced body save.
- The same `ListBarLayout.qml` now composes a narrow `FocusedNoteDeletionBridge`, so `Backspace` / `Delete` resolve
  the visually focused note id directly from the list view before falling back to the active note-list model. The
  bridge still forwards deletion into the injected `LibraryNoteMutationViewModel::deleteNoteById(...)` contract, while
  the note-mutation view-model stays separate from the hierarchy read model and delegates destructive writes into
  `WhatSonHubNoteDeletionService`. File-system integrity repair now lives
  under `src/app/file/validator/`: `WhatSonHubStructureValidator` resolves hub/library/stat paths,
  `WhatSonNoteStorageValidator` resolves materialized `.wsnote` / `.wsnhead` storage, and
  `WhatSonLibraryIndexIntegrityValidator` owns orphan pruning plus `index.wsnindex` rewrites. When a stale
  `index.wsnindex` entry no longer has a materialized `.wsnote`, load-time indexing now prunes the orphan from both
  the visible note set and the rewritten index, and the delete service also treats such entries as index-only cleanup
  instead of failing for a missing directory.
  `BookmarksHierarchyViewModel` only mirrors the deletion into its bookmarked subset.
- `ListBarLayout.qml` now also mounts an LVRS note-card context menu for desktop right-click. It targets the hovered
  note id without forcing a selection change first, and currently exposes `Delete note` plus `Clear all folders`.
  The latter forwards into `LibraryNoteMutationViewModel::clearNoteFoldersById(...)`, which delegates the
  non-destructive header rewrite to `WhatSonHubNoteFolderClearService` and refreshes the visible note metadata
  without deleting the note itself.
- The note-card delegate now uses explicit required note roles (`noteId`, `primaryText`, `image`, `imageSource`,
  `displayDate`, `folders`, `tags`, `bookmarked`, `bookmarkColor`) instead of depending on a nullable runtime
  `model` object, which removes delegate startup `TypeError` churn during note-list refresh and keeps note drags
  stable while the bound list model changes.
- `SidebarHierarchyView.qml` no longer composes a local hierarchy interaction engine or an LVRS adapter bridge. It
  mounts `LV.Hierarchy` directly and consumes the active `IHierarchyViewModel` contract instead of runtime
  `QObject` duck-typing.
- `DetailPanel.qml` now resolves its `detailPanelViewModel` through LVRS `ViewModels` instead of reading the mutable
  view-model directly from the root QML context.
- `ContentsDisplayView.qml` now composes four narrow editor helpers instead of one god-object bridge:
  `ContentsEditorSelectionBridge` for note selection/count/persistence contracts,
  `ContentsLogicalTextBridge` for logical-line parsing, `ContentsGutterMarkerBridge` for gutter-marker normalization,
  and `ContentsEditorSession.qml` for idle-sync session state plus selection-to-editor text synchronization. The visual surface keeps
  only editor-geometry sampling and render placement.
- The live editor surface now treats markdown as part of the canonical `.wsnbody` source format instead of a separate
  preview grammar. `ContentsTextFormatRenderer` splits the cheap source-editing HTML surface from the optional
  markdown-aware preview HTML, and hidden preview paths no longer regenerate on every edit.
- `ContentsEditorSelectionBridge` now forwards editor persistence into
  `src/app/file/sync/ContentsEditorIdleSyncController.*`.
  QML/editor code now requests immediate `{noteId, bodyText}` flushes for live user mutations, and the sync
  controller keeps the buffered `1000ms` fetch clock only as the retry/drain path for already accepted dirty note
  snapshots. The background note-management lane re-reads the current `.wsnote`, writes `.wsnbody` / `.wsnhead`
  through `WhatSonLocalNoteFileStore`, mirrors normalized body state back into the active editable hierarchy
  viewmodel, mirrors mounted-Android note writes back into the original source package before reporting success, and
  defers hub-wide `.wsnbody` backlink/open-count scans to a later
  `requestTrackedStatisticsRefreshForNote(...)` pass owned by that viewmodel.
- Note selection transitions no longer pay that hub-wide `.wsnbody` rescan. The selection bridge now resolves
  `{noteId, noteDirectoryPath}` from `.wsnhead`-backed metadata and increments `openCount` through a header-only
  rewrite path, so selecting another note stays decoupled from backlink recalculation.
- Editor body persistence now treats live editing as write-through by default: ordinary typing, formatting, and
  programmatic source rewrites request immediate `.wsnbody` flushes first, and QML editor sessions use the buffered
  fetch clock only to drain or retry note snapshots that were already accepted into the persistence lane.
- Desktop/mobile editor hosts now also accept native file drags directly over the note surface without a MIME-key
  prefilter, route those drops through `ResourcesImportViewModel`, create flat `.wsresource` packages under the
  active hub `*.wsresources` root, append the package paths into `Resources.wsresources`, inject canonical
  `<resource type="..." format="..." path="...">` calls into the live note source, and rebuild the in-editor
  resource-card overlay from the current presentation snapshot so the dropped asset appears immediately instead of
  waiting for a later filesystem reread.
- Immediate editor flush requests now fail fast when the persistence lane rejects the current snapshot, instead of
  silently reporting acceptance while the note remains dirty only in memory.
- The same save path now short-circuits when the normalized plain-text body is unchanged, so a no-op save no longer
  rewrites `.wsnbody` or strips existing empty/custom body tags that the editor did not modify.
- Legacy `.wsnbody` semantic tags now resolve through one shared registry across persistence and editor HTML read paths,
  so tags such as `<next/>`, `<title>`, `<subTitle>`, and `<eventTitle>` no longer render as literal XML in one path
  while being interpreted semantically in another.
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
  editor surface, opens a different note, or the underlying `QtQuick.TextEdit` finishes a delayed relayout, the visible
  line numbers
  and current-line markers are resampled from the settled editor geometry instead of stretching stale positions from a
  previous note/session.
- Minimap snapshotting no longer walks the whole note text through `positionToRectangle(...)` on ordinary edits.
  Desktop and mobile now share an incremental line-range diff helper, cache logical-line minimap groups, and only
  resample the changed text window unless a layout reset or note switch forces a full rebuild.
- The blue current-line gutter marker is bound to the cursor's active visual row, so the marker no longer stretches
  through the whole remaining editor height when the cursor sits on the last logical line.
- `ContentsDisplayView.qml` no longer reintroduces a second desktop panel fill inside the editor stack: the editor
  surface and gutter fill stay transparent and let the root `LV.ApplicationWindow` canvas read through
  the whole desktop content column.
- The editor surface now also exposes a right-side Xcode-style minimap, but it is rendered as a borderless inline text
  silhouette instead of a framed rail. Its bar positions come from the editor's real content height and text-start
  offset, so short notes stay top-aligned and the minimap reflects the text body rather than gutter markers.
- That silhouette is now painted from actual wrapped visual-row segments taken from the `QtQuick.TextEdit` layout rather than
  from one height-scaled logical-line block, so wrapped paragraphs appear as separate thin text strokes instead of
  carved slabs.
- Minimap rows are packed with a fixed `1px` gap between bars, which keeps the overview denser than the main editor and
  prevents sparse empty rails from making the minimap appear longer than the body it represents.
- The current cursor line on that minimap is reduced to the active line's own silhouette width, and the visible
  viewport is shown as a subtle translucent fill without an outline border. Click/drag plus wheel-routed scrolling are
  still supported.
- Cursor-only and viewport-only minimap updates now reuse cached row geometry instead of rebuilding the whole minimap
  snapshot on every cursor/scroll signal. When a route disables the minimap or mobile input runs through
  `preferNativeInputHandling`, the editor skips that minimap sampling path entirely so OS cursor gestures such as the
  iOS spacebar-drag session stay native.
- Full-document RichText projection is no longer rebound directly from `editorText` on every edit. The desktop/mobile
  editor views now keep the source-to-logical bridge hot for typing diffs, but defer the expensive
  `ContentsTextFormatRenderer` render plus editor-surface/minimap snapshot commit to a short idle timer or an explicit
  focus-loss / note-switch flush.
- The shared `ContentsInlineFormatEditor.qml` wrapper now also forwards cursor/selection/geometry changes to
  `Qt.inputMethod.update(...)` using the same `Qt::ImQueryInput` / cursor-rectangle queries documented by Qt, so iOS
  keyboard trackpad-style cursor and selection gestures can keep following the live `TextEdit` state.
- Programmatic selection restoration now uses `TextEdit.moveCursorSelection(...)` instead of plain `select(start, end)`
  when an active edge must be preserved. This keeps iOS keyboard-driven range expansion from collapsing to only the
  most recently traversed fragment after the app re-syncs the surface or reapplies a selection.
- The shared editor wrapper now also supplements touch-only multi-tap selection on native-input mobile paths:
  double-tap reselects the touched word and triple-tap expands to the surrounding paragraph so iOS selection gestures
  are not lost when the live editor is hosted inside a `Flickable`.
- The left marker rail is state-driven: the current cursor line is blue (`LV.Theme.primary`), lines changed in the
  current session are yellow (`#FFF567`), and externally supplied sync-conflict ranges are red (`LV.Theme.danger`).
- Conflict detection and sync integration are not implemented yet, but `ContentsDisplayView.qml` already accepts
  external gutter marker ranges via `gutterMarkers` using `{ type: "changed" | "conflict", startLine, lineSpan }` or
  `{ type, startLine, endLine }`.
- The QML safety guard scans for two recurring corruption patterns: standalone string literals inside `Binding {}`
  blocks and standalone dotted expressions such as `noteListItem.imageSource` that should have been property
  assignments. Critical `ContentsDisplayView.qml` helper bodies are also expected to keep explicit `return`
  statements.
- The same guard path also protects centralized MVVM boundaries for data-driven views: sidebar state stays anchored in
  `SidebarHierarchyViewModel`, hierarchy rendering keeps using the LVRS `Hierarchy` surface plus each domain
  view-model's direct `hierarchyModel` property, and editor-side selection/persistence/text/gutter contracts stay
  split across dedicated editor adapters.
- The obsolete LVRS override layers have been removed from the tree: no local hierarchy compat list, no sidebar
  interaction-controller hierarchy engine, and no project-specific `NavigationIconButton` wrapper remain. Navigation
  panels now bind stock LVRS primitives directly.

## Theme Token Usage

- `NoteListItem.qml`, `ListBarLayout.qml`, `MobilePageScaffold.qml`, `MobileHierarchyPage.qml`, `NavigationModeBar.qml`, and
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

Ordinary host builds stay incremental and no longer trigger the dedicated `build-trial`
desktop packaging tree automatically.
If you explicitly need the nested trial packaging pass from the host tree, opt in during configure
and invoke the mirror target manually:

```bash
cmake -S . -B build -DWHATSON_ENABLE_TRIAL_BUILD_MIRROR=ON
cmake --build build --target whatson_sync_trial_build
```

On macOS the nested trial tree pins `CMAKE_OSX_ARCHITECTURES` from the host/LVRS setup so an
auxiliary x86_64 CMake process does not accidentally configure `build-trial` against an arm64-only
LVRS install.
The nested configure step forwards list-style cache values such as `CMAKE_PREFIX_PATH` and
`CMAKE_OSX_ARCHITECTURES` through a generated preload cache script, so the forwarded Qt/LVRS
prefixes keep their full path list instead of splitting into stray extra shell commands.

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
    - `whatson_build_regression`: builds maintained app/daemon/CLI targets plus the C++ regression test executable.
    - `whatson_cpp_regression`: runs the Qt Test based C++ build/runtime regression suite under `test/cpp`.
    - `whatson_regression`: default aggregate regression gate (`whatson_build_regression` + `whatson_cpp_regression`).
    - `whatson_dev_checks`: default aggregate target (`qmllint` + `qmlformat_check`, plus `clang-tidy` when installed).
- `qmllint` is intentionally configured to fail on syntax/import errors while tolerating the repository's current
  warning
  baseline, so it is immediately useful for catching broken QML without forcing a full warning cleanup first.
- `whatson_clang_tidy` reads repository policy from [.clang-tidy](/Volumes/Storage/static/Product/WhatSon/.clang-tidy).
- If a tool is missing at configure time, the corresponding target still exists but fails with an explicit installation
  hint when invoked.

## Regression Tests

The automated regression suite lives under `test/`, with the maintained build entrypoints exposed from the root
`CMakeLists.txt`.

- `whatson_build_regression` performs the maintained incremental build gate for app/daemon/CLI targets and
  `whatson_cpp_regression_tests`.
- `whatson_cpp_regression` runs Qt Test based C++ build/runtime regressions for hub/sidebar stores, sidebar hierarchy
  resolution, navigation state/viewmodels, onboarding route bootstrap, scheduler behavior, note resource/folder
  helpers, and structured document mutation/collection policies.
- `whatson_regression` combines the build gate and the runtime C++ regression suite for the standard repository
  verification pass.

```bash
cmake -S . -B build
cmake --build build --target whatson_build_regression -j
cmake --build build --target whatson_regression -j
ctest --test-dir build --output-on-failure -L cpp_regression
```

The suite intentionally stays below the full app runtime: it verifies regression-sensitive behavior, ownership
boundaries, and persistence contracts without starting the interactive workspace shell.

## Unified Root CMake Targets

Platform build, launch, export, and package targets are orchestrated from the root `CMakeLists.txt` and grouped into
`cmake/root/build`, `cmake/root/dev`, `cmake/root/runtime`, and `cmake/root/distribution`.

```bash
cmake --build build --target whatson_build_all
cmake --build build --target whatson_build_regression
cmake --build build --target whatson_regression
cmake --build build --target whatson_run_app
cmake --build build --target whatson_healthcheck_daemon
cmake --build build --target whatson_export_binaries
cmake --build build --target whatson_package
```

The optional Rust CLI launcher mirrors the same desktop app entrypoint and now supports reopening the onboarding window
without bootstrapping the workspace shell:

```bash
cmake --build build --target whatson_cli
build/cargo/release/whatson
build/cargo/release/whatson onboard
```

`whatson onboard` forwards an internal `--onboarding-only` app flag and loads `Onboarding.qml` directly.
The onboarding surface now uses native Qt Quick dialogs to either create a new `.wshub` package at a user-selected
path through `WhatSonHubCreator` or load an existing package immediately into the workspace shell. Existing hub selection
accepts the `.wshub` package root, a parent directory that contains exactly one `.wshub`, or any nested path inside an
existing `.wshub` bundle so mobile document pickers can promote package-internal selections back to the hub root.
On iOS the native Files picker still provides security-scoped document URLs, so onboarding now starts a security-scoped
resource session before creating or loading a `.wshub` and retains access to the resolved hub root for the rest of the
app session. The selected Files scope is also persisted as an implicit bookmark so startup can restore that access and
reopen the same external `.wshub` without replaying onboarding when the bookmark remains valid. Mobile onboarding also
switches hub creation to a directory-picker flow and synthesizes a unique
`Untitled*.wshub` target path inside the chosen folder, because iOS native file dialogs only implement the open path
and native mobile pickers diverge from desktop save-dialog semantics. Existing hub selection now keeps the iOS folder
picker plus the in-session candidate fallback, but Android switches to a native file picker that can target a
`.wshub` package document directly. Android onboarding then resolves external-storage SAF document URLs back to shared
filesystem paths whenever the picker points at a local `.wshub`, so the existing directory-based hub creator and
runtime loader can mount the package tree instead of treating the document URI itself as a virtual directory.
Mobile startup now follows the LVRS page-stack contract instead of booting a separate standalone onboarding window when
no hub is restored. `Main.qml` registers `/onboarding` beside the workspace route and lets the same
`LV.ApplicationWindow` transition from onboarding into the workspace shell, so iOS/Android no longer cross a
multi-window handoff that can terminate the app session after a successful hub load. The dedicated `Onboarding.qml`
window wrapper remains available for the explicit `whatson onboard` entrypoint and the desktop window-menu command,
but regular mobile bootstraps route inside `Main.qml` through the shared `OnboardingContent.qml` surface.
Mobile onboarding no longer treats `hubLoaded` as an immediate page-complete event. `OnboardingHubController`
now tracks a session-state ladder (`idle -> resolvingSelection -> loadingHub -> hubLoaded -> routingWorkspace -> ready`)
and `Main.qml` only commits `/onboarding -> /` after the LVRS router confirms the workspace navigation. If that route
commit fails, the controller falls back to `failed`, keeps the app inside onboarding, and surfaces the error instead of
tearing down the mobile session.
Before the runtime loader starts, `OnboardingHubController` also performs a local `.wshub` mount preflight:
non-local document-provider URLs that cannot be resolved into real package directories now fail inside onboarding with a
targeted error, instead of cascading into a full-domain runtime bootstrap failure.
The desktop onboarding window's right-hand status panel stays aligned with the Figma onboarding design and shows either `No WhatSon Hub Selected`
or the currently selected `.wshub` package name.
The last selected `.wshub` is persisted through the app session store as a startup candidate, so the next launch tries
that selection before falling back to `blueprint/*.wshub`.
Startup onboarding is now gated by hub mountability rather than by full runtime-domain success. WhatSon first resolves
the persisted hub selection into a mountable startup hub path (local `.wshub`, Android source URI -> mounted local
copy, or restored Android mounted-hub source URI). If that persisted candidate can no longer be mounted, startup
retries the bundled blueprint hub before handing control to onboarding.
When a startup hub is available, the first workspace frame now prioritizes the critical library-facing runtime
domains and defers low-priority hierarchy domains (`Event`, `Preset`) until post-show idle turns or the first sidebar
activation that needs them.
Concurrent hub access is now allowed across desktop and mobile sessions. `WhatSonHubWriteLease` remains only as a
legacy cleanup shim for old `.whatson/write-lease.json` artifacts, so onboarding/runtime loading and filesystem
mutation paths no longer reject a `.wshub` just because another WhatSon session already touched the same package.
Runtime hub refresh now flows through `src/app/file/sync/WhatSonHubSyncController.*`. The controller keeps a periodic
filesystem watcher/timer policy, recursive `QFileSystemWatcher` coverage over the mounted `.wshub`, and a debounced
reload path that reacts to observed hub-signature changes without UI interaction hints. Signature hashing and watcher
path collection now come from a single recursive observation pass, so one sync hint no longer scans the hub twice.
App-owned
`.whatson/write-lease.json` heartbeat updates are excluded from that observed signature/watch set so local lease
maintenance never bounces the runtime through a self-reload.
App-owned note and folder mutations do not bounce the runtime through a full reload immediately. Instead,
`LibraryHierarchyViewModel` and `BookmarksHierarchyViewModel` emit a local mutation acknowledgment so
`WhatSonHubSyncController` can refresh its baseline after self-writes and keep the current editing session stable.
The runtime loader must derive bookmarks from the shared library snapshot instead of reparsing the same `.wshub` tree
a second time. `WhatSonLibraryIndexedState` centralizes the library/draft/today projection backend used by both the
runtime loader and hierarchy viewmodels.
Within `ContentsDisplayView.qml`, a selected note's live editor buffer becomes the source of truth after the user edits
it once; divergent same-note storage payloads are rejected and re-persisted so runtime reloads cannot steal the caret
or overwrite the active mobile/desktop editing session.
The desktop/mobile editor surfaces now keep `documentPresentationSourceText` as a separate whole-document
presentation snapshot. Ordinary typing updates raw `editorText` immediately, while `ContentsLogicalTextBridge` and
`ContentsTextFormatRenderer` only rebuild from that snapshot after editor idle, blur, or note switch; the typing
controller carries an incremental logical-to-source offset cache between those commits so single-character edits no
longer force full-note plain-text diff/remap/rerender work.
That same typing controller now also maintains incremental logical line-start offsets and pushes the full live state
into `ContentsLogicalTextBridge.adoptIncrementalState(...)`, while desktop/mobile line-geometry helpers reuse cached
`minimapLineGroups` even when the minimap is hidden. Inline `<resource ... />` blocks now also share the same
single-logical-line contract across the bridge, structured block parser, and fallback RichText projection, so gutter,
minimap, and source-offset helpers no longer disagree about resource block height. As a result, ordinary typing no
longer needs a whole-note bridge rebuild or a second whole-document line-rectangle sweep just to keep gutter/minimap
helpers aligned.
Editor/QML code now stages save intent through `src/app/file/sync/ContentsEditorIdleSyncController.*`, which owns the
worker-thread idle detector and note-exit flush promotion.
Non-editing note-management work still sits behind `src/app/file/note/ContentsNoteManagementCoordinator.*`; direct
`.wsnote` persistence, header open-count maintenance, tracked-stat refresh, and post-persist metadata resync are
serialized there outside the editor hot path.
Local note-file CRUD is now centralized under `src/app/file/note/WhatSonLocalNoteFileStore.*` and
`src/app/file/note/WhatSonLocalNoteDocument.hpp`. That IO layer owns `.wsnhead` / `.wsnbody` create-read-update-delete
operations plus per-note `.wsnhistory` append-only diff logging and `.wsnversion` manifest initialization for library
note creation, body persistence, folder-drop header rewrites, folder hierarchy remaps, and note deletion, so the local
filesystem remains the first writer of record before runtime projections or external sync react.
- `src/app/file/note/WhatSonLocalNoteVersionStore.*` adds git-like note version primitives on top of that local file
  boundary: it captures full working-tree snapshots into `.wsnversion`, tracks `currentSnapshotId` vs
  `headSnapshotId`, computes compact header/body diffs between any two snapshots, supports detached checkout of an
  existing snapshot, and appends a fresh rollback snapshot when the user restores an older state.
App-owned filesystem mutations are also serialized through `src/app/file/IO/WhatSonSystemIoGateway.*`. UTF-8 file
overwrites now commit through an atomic save path, and library index / note companion writes no longer bypass that
gateway with ad-hoc `QFile` truncation helpers.

On native desktop host builds, `whatson_export_binaries` now stages a self-contained install tree under `build/dist`
via `cmake --install`. The same deployment path is used by:

```bash
cmake --install build --prefix /absolute/output/prefix
```

Desktop install/export layout:

- macOS: `build/dist/WhatSon.app` at the install prefix root, with deployed Qt/LVRS runtime content inside the bundle
- Windows: `build/dist/bin/WhatSon.exe` plus deployed Qt/LVRS runtime DLLs, `qml/`, `plugins/`, and `qt.conf`
- Linux: deployed Qt/LVRS runtime libraries, QML imports, the desktop entry, and the app icon metadata

Platform icon packaging is resolved from `resources/icons/app/` during configure time:

- macOS bundle icon: `resources/icons/app/desktop/AppIcon.icns`
- iOS app icon set: generated Xcode asset catalog from the iPhone/iPad PNG variants under `resources/icons/app/ios/`,
  attached to the iOS bundle `Resources` phase so Xcode compiles `Assets.car`
- Android launcher icon: density-specific `resources/icons/app/android/<density>/AppIcon.png` files overlaid into the
  Android package
- Windows executable icon: `resources/icons/app/desktop/AppIcon.ico`
- Linux desktop/package icon: `resources/icons/app/desktop/AppIcon.png`

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

Host desktop app builds now emit the runnable app artifact at the build-directory root:

- macOS: `build/WhatSon.app`
- Windows: `build/WhatSon.exe`
- Linux and other non-bundle desktop targets: `build/WhatSon`

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
./build/WhatSon 2>&1 | rg "\\[whatson:debug\\]|\\[wsnhead:index\\]"
# macOS bundle path equivalent:
./build/WhatSon.app/Contents/MacOS/WhatSon 2>&1 | rg "\\[whatson:debug\\]|\\[wsnhead:index\\]"
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
- The desktop navigation add surfaces share one `create-note` panel hook path:
  `NavigationAddNewBar.qml` and `NavigationApplicationControlBar.qml`
  route `create-note` into `LibraryNoteMutationViewModel::createEmptyNote()`, while the mobile bottom add button
  delegates through `windowInteractions.createNoteFromShortcut()` so generic mobile panel hooks do not create notes.
- The application-wide `StandardKey.New` shortcut is desktop-only. Mobile note creation must stay on explicit UI
  surfaces instead of a global application shortcut.
- Mobile workspace views now live under `src/app/qml/view/mobile/**`. `src/app/qml/view/mobile/MobilePageScaffold.qml`
  keeps the top navigation bar and the bottom status/add-note bar persistent across mobile workspace pages, while
  `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml` now mounts the node `174:5026` hierarchy body and the
  node `174:5169` note-list body plus the node `174:5687` editor body through an internal `LV.PageRouter` inside that
  scaffold.
- `MobilePageScaffold.qml` composes the shared `NavigationBarLayout.qml` (`compactMode: true`) and
  `StatusBarLayout.qml`, so mobile pages reuse the same LVRS navigation/sidebar contracts as the desktop shell instead
  of redrawing private chrome.
- The compact search field inside `StatusBarLayout.qml` now lets `LV.InputField` render its own
  `shapeCylinder` frame directly and binds `compactToolbarText` through `placeholderText`, so the mobile bottom bar
  keeps the pill-shaped search affordance from the shared LVRS component set instead of a wrapped local rectangle.
- `MobilePageScaffold.qml` now keeps the token-only page paddings (`16 / 48 / 16`) but reduces the scaffold-level
  spacing between the compact navigation bar, the routed page body, and the compact
  status/add-note bar.
- `MobileHierarchyPage.qml` disables LVRS `usePlatformSafeMargin`, keeps the hierarchy column on the same `panelBackground01` canvas as the mobile root, follows the Figma `174:5026` hierarchy-local `gap2` spacing for toolbar/search/list, applies token-only body paddings (`16 / 48 / 16` via LVRS gap composition), shows the compact top-right add-folder affordance, overrides the shared hierarchy shell geometry to `0 / 0 / 2 / 2` for toolbar/search/list spacing, and hides the shared hierarchy footer on mobile.
- The same `MobileHierarchyPage.qml` now mounts the mobile note-list body through the shared `ListBarLayout.qml`
  contract instead of duplicating note delegates locally: it binds `SidebarHierarchyViewModel.resolvedNoteListModel`,
  hides the desktop list header with `headerVisible: false`, keeps the note-list body on the same
  `panelBackground01` canvas, routes hierarchy row taps into that note-list body, and lets the shared list delegate
  emit `noteActivated(index, noteId)` so mobile routing can follow note selection without a second list-specific
  delegate stack.
- That shared `ListBarLayout.qml` note/resource viewport now branches its flick contract by platform:
  desktop keeps the existing quantized non-kinetic scroll, while mobile uses kinetic flick deceleration/velocity plus
  overshoot bounds so note and resource lists behave like native iPhone inertial lists.
- `MobileHierarchyPage.qml` now mounts the mobile editor body through the shared `ContentViewLayout.qml` contract and
  binds the same `SidebarHierarchyViewModel.resolvedHierarchyViewModel` plus `resolvedNoteListModel` pair used on
  desktop, so note-card taps open the real editor for the selected note instead of a mobile-only text surface.
- The mobile editor route only overrides layout knobs that differ from desktop Figma: it keeps the shared LVRS editor
  session/persistence wiring, hides the minimap, keeps the same `16px` top inset as desktop, removes the
  frame side inset, clears the gutter fill back to transparent, and narrows the gutter to `40px` with a `22px`
  line-number column.
- The shared `SidebarHierarchyView.qml` now lets row taps approve flick takeover on mobile, so the embedded
  `LV.Hierarchy` scroller keeps inertial vertical scrolling instead of dead-stop drag behavior on touch devices.
- `MobileHierarchyPage.qml` now suppresses the compact leading action on the mobile note-list view, so the routed list
  and editor views match the Figma top bar and leave page undo to swipe navigation instead of a visible back button.
- The same mobile note-list route now also swaps the shared `NavigationBarLayout.qml` compact trailing controls to the
  Figma node `174:5171` order `sortByType -> cwmPermissionView -> toolwindowtodo`, and hides the compact settings
  button so the note-list top bar no longer reuses the hierarchy-route chrome.
- The shared compact `settings` button is now exclusive to the `/mobile/hierarchy` route. Both the routed note-list
  view and the routed editor view suppress it, so the mobile hierarchy chrome no longer bleeds into the other routed
  bodies.
- `MobilePageScaffold.qml` now owns the mobile routed body through `LV.PageRouter`, and `MobileHierarchyPage.qml`
  drives that stack with explicit `/mobile/hierarchy` and `/mobile/note-list` routes instead of a private page-state
  enum plus route-memory copies.
- `MobileHierarchyPage.qml` now drives left-edge page undo through `LV.PageTransitionController` and a left-edge touch
  `DragHandler`, so mobile back navigation stays local to the edge hit zone instead of subscribing the whole window to
  global LVRS gesture runtime events. The same touch session is now consumed after commit or cancel so an editor
  back-swipe cannot immediately start a second pop that skips the intermediate note-list route.
- The same mobile route shell now snapshots the active hierarchy `selectedIndex` before opening or repairing the
  note-list/editor stack, and it suppresses the temporary hierarchy-route deselection pass while canonical
  `setRoot(hierarchy) -> push(note-list)` rebuilds run. This keeps folder-specific note lists such as `Untitled`
  from falling back to `All Library` during editor back-swipe repair, which also removes the extra post-pop entrance
  animation that came from rebuilding the wrong list context.
- When the mobile route returns to `/mobile/hierarchy`, `MobileHierarchyPage.qml` now clears the active hierarchy
  selection through the shared domain view-model, so tapping the same folder after backing out of the note-list or
  editor reopens the routed note-list body instead of being swallowed by a stale LVRS active-row state.
- `StatusBarLayout.qml` keeps the compact mobile search field on the LVRS cylinder style, while
  `SidebarHierarchyView.qml` overrides the shared `ListBarHeader.qml` search input back to LVRS `shapeRoundRect` so the
  hierarchy search field stays rounded instead of pill-shaped.
- The same `MobileHierarchyPage.qml` now pins the hierarchy header stack to the exact Figma `174:5026` rhythm:
  `toolbar 20 / gap 2 / search 18 / gap 2 / hierarchy list`, instead of inheriting the shared desktop
  `24px` search-header frame minimum.
- The desktop `BodyLayout.qml` explicitly keeps `HierarchySidebarLayout.searchFieldVisible: true`, so the shared
  hierarchy search header remains visible on the desktop workspace while mobile still controls that affordance per
  routed page.
- `SidebarHierarchyView.qml` now lets the embedded LVRS `Hierarchy` consume its own internal `20px` toolbar reserve and
  only adds the explicit `searchHeaderTopGap + searchHeaderMinHeight + searchListGap` when the shared search header is
  visible, removing the duplicated toolbar-height gap that previously appeared under the search bar on both desktop and
  mobile.
- The hierarchy search field no longer keeps a second horizontal wrapper inset; its frame alignment now comes directly
  from the same `horizontalInset` that anchors the list body, so the search bar and hierarchy rows share one padding
  source.
- `NavigationApplicationControlBar.qml` compact mode is now the collapsed mobile control surface: it exposes only the
  existing context-menu button, while `NavigationBarLayout.qml` keeps the navigation mode combo on the shared
  `NavigationModeViewModel` without replacing the shared hierarchy footer actions. The compact control menu anchors
  from the trigger's bottom-right corner, the compact navigation right group resolves in the Figma order `new folder ->
  menu`, the hierarchy-route compact control `LV.IconMenuButton` now uses the `toolwindowtodo` glyph with the built-in
  LVRS chevron indicator plus the Figma `2 / 4 / 2 / 2` padding contract, and action-only control entries suppress the LVRS shortcut column so
  labels keep their full width budget on mobile.
- When the routed mobile note-list page is active, `NavigationApplicationControlBar.qml` now switches to a dedicated
  compact variant that renders `sortByType`, `cwmPermissionView`, and a `toolwindowtodo` `LV.IconMenuButton` in the
  same order as the Figma note-list bar instead of reusing the hierarchy/control compact menu button.
- Navigation mode state is centralized in `src/app/viewmodel/navigationbar/NavigationModeViewModel.*`:
  `main.cpp` injects `navigationModeViewModel`, and the navigation bar mode combo binds to the dedicated enum-backed
  `View/Edit/Control/Presentation` state plus its per-mode QObject viewmodels.
- Editor view mode state is centralized in `src/app/viewmodel/navigationbar/EditorViewModeViewModel.*`:
  `main.cpp` injects `editorViewModeViewModel`, and the navigation bar editor-view combo binds to the dedicated
  enum-backed `Plain/Page/Print/Web/Presentation` state plus its per-view QObject viewmodels.
- The sidebar initial width now follows the effective rendered width of the hierarchy toolbar: Figma's `200px`
  toolbar track plus `2px` side insets resolves to a `204px` default sidebar width.
- Figma navigation frames are split into dedicated QML files under `src/app/qml/view/panels/navigation/`:
  `NavigationPropertiesBar.qml`, `NavigationInformationBar.qml`, `NavigationModeBar.qml`,
  and `NavigationEditorViewBar.qml`.
- The right-side application area is split by navigation mode into dedicated QML files under mode-specific
  subdirectories: `navigation/view/NavigationApplicationViewBar.qml`,
  `navigation/edit/NavigationApplicationEditBar.qml`, and
  `navigation/control/NavigationApplicationControlBar.qml`.
- Control-only child bars now live with the control-mode shell under `navigation/control/`, while shared bars such as
  `NavigationAddNewBar.qml` and `NavigationPreferenceBar.qml` remain at the navigation root.
- `navigation/control/NavigationApplicationControlBar.qml` follows the Figma child order `Calendar -> AppControl -> Export -> AddNew ->
  Preference`, keeping the create control on the right side of the control-mode application cluster.
- `navigation/view/NavigationApplicationViewBar.qml` follows Figma node `149:4000` and now composes
  `NavigationApplicationViewOptionBar -> NavigationApplicationViewModeBar -> NavigationApplicationViewCalendarBar ->
  NavigationAddNewBar -> NavigationPreferenceBar` in desktop/full mode.
- `navigation/edit/NavigationApplicationEditBar.qml` follows Figma node `149:4102` and now composes
  `NavigationCalendarBar -> NavigationAddNewBar -> NavigationPreferenceBar` in desktop/full mode.
- Shared application wrappers that still span multiple modes remain under `navigation/`:
  `NavigationApplicationAddNewBar.qml` and `NavigationApplicationPreferenceBar.qml`.
- Edit mode also keeps the shared `NavigationApplicationCalendarBar.qml`, while view mode now owns its
  Figma-diverged calendar/options/mode slices locally under `navigation/view/`.
- Both view/edit bars use the same compact-mode shell pattern as control mode (`Loader` + `LV.IconMenuButton` +
  `LV.ContextMenu`) so mobile/tight layouts still expose the mode tools through a single overflow trigger.
- View compact menus now mirror the full Figma metadata order by exposing view options, view modes, calendar scope
  actions, `New File`, and `Preferences`, while preserving `keyVisible: false` action rows.
- `Main.qml` binds a global `Tab` shortcut that cycles `View/Edit/Control/Presentation` only when no text input or
  text editor currently owns focus.
- `Main.qml` also binds a global platform-native New shortcut (`Cmd+N` on macOS, `Ctrl+N` elsewhere), but that
  desktop-only ownership-aware New shortcut is now exposed only on desktop platforms. Mobile note creation must stay
  on explicit UI surfaces instead of a global application shortcut.
- New note creation now emits a dedicated runtime signal from `LibraryHierarchyViewModel`, and `ContentsDisplayView.qml`
  consumes it to move keyboard focus into the editor so body text entry can begin immediately.
- If the active note search would hide the freshly created note, the library list clears that search first so the new
  note can still become the active editor target.
- `Main.qml` also listens to LVRS global press hit-tests and clears the current focus chain when a left-click lands on
  an empty background/container surface instead of an interactive control.
- `NavigationModeBar.qml` and `NavigationEditorViewBar.qml` use `LV.ComboBox` as the trigger surface and open
  `LV.ContextMenu` enumerations on click instead of cycling state directly.
- The `NavigationModeBar.qml` context menu now follows the Figma `191:5519` icon contract with
  `generalshow -> renameColumn -> abstractClass`, and it keeps the trailing `Key` column hidden by leaving each
  entry `keyVisible: false`.
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
- Footer CRUD scope policy: only `LibraryHierarchyViewModel`, `ProjectsHierarchyViewModel`, and
  `TagsHierarchyViewModel` expose active hierarchy `+/-` footer behavior. Bookmarks, resources, progress, event,
  and preset keep those actions disabled because their rows are runtime/view-model projections.
- Expansion-state protection policy: `SidebarHierarchyView.qml` must only push user-touched expansion changes back into
  a hierarchy view-model through a targeted `setItemExpanded(...)` hook. Bulk hierarchy rewrites are not allowed for
  create-folder flows because they mutate untouched rows and destabilize the rendered LVRS tree.
- Create-folder targeting policy: library and tags insert the new row as a child of the active hierarchy item and
  expand that parent so the inserted child becomes visible immediately. Projects are flat, so project creation always
  inserts a new root-level sibling row and never creates child items.
- Folder-identity policy: library note-folder assignments are stored and propagated as canonical full paths whenever
  hierarchy context exists (`Parent/Child/Leaf`). UI rendering may collapse those paths to the leaf label for display,
  but selection/filtering must use the exact canonical path and must never fan a note out across different folders that
  merely share the same leaf name or ancestor-chain label.
- `SidebarHierarchyView.qml` maps that CRUD/view-options surface into a direct bottom `LV.ListFooter` instance
  matching the Figma `HierarchyFooter` node (`134:3178`), with explicit `78x24` sizing, transparent button
  backgrounds, and concrete icon names (`generaladd`, `generaldelete`, `generalsettings`) instead of relying on a
  local custom footer wrapper.
- Rename interaction policy: when the selected hierarchy row is renameable, `SidebarHierarchyView.qml` enters an inline
  `LV.InputField` overlay on `Enter` / `Return`, anchors that overlay to the edited row geometry instead of the current
  LVRS active-item pointer, seeds the editor with the leaf folder name only, suppresses the edited row label and path
  fallback while the overlay is active, commits through the bound view-model `renameItem(...)` contract, and cancels
  cleanly on `Escape` or selection/toolbar changes.
- The hierarchy toolbar keeps LVRS `Hierarchy` for the list surface, but `SidebarHierarchyView.qml` mounts a dedicated
  fixed `Row` of LVRS icon buttons over the header strip so the eight `20px` icons stay left-anchored on the Figma
  `200px` track with a stable `40 / 7` gap.
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
- Chevron click now toggles fold/unfold through LVRS `HierarchyItem.expanded` without re-activating the row, and
  sidebar delegates follow `HierarchyItem.rowVisible` for effective height/visibility so collapsed descendants do not
  reserve row space.
- `library`: `WhatSonLibraryHierarchy{Store,Parser,Creator}` (`Library.wslibrary/index.wsnindex`)
- `projects`: `WhatSonProjectsHierarchy{Store,Parser,Creator}` (`ProjectLists.wsproj`)
- Projects hierarchy rows keep the runtime Figma `45:2750` contract without extending the persisted project schema:
  every row emits `iconName: "customFolder"`, while the visible label is still sourced directly from the bound
  project-name/view-model string.
- `bookmarks`: `WhatSonBookmarksHierarchy{Store,Parser,Creator}` (`Bookmarks.wsbookmarks`)
- `tags`: `WhatSonTagsHierarchy{Store,Parser,Creator}` (`Tags.wstags`, tree/flat JSON with preserved hierarchy depth)
- `resources`: `WhatSonResourcesHierarchy{Store,Parser,Creator}` (`Resources.wsresources`)
- Resources hierarchy now renders a runtime-only read-only taxonomy tree for the Figma `45:3244` sidebar contract:
  every row emits `iconName: "virtualFolder"`, top-level parents are `Image`, `Video`, `Document`, `3D Model`,
  `Web page`, `Music`, `Audio`, `ZIP`, and `Other`. All categories except `Other` expand into concrete extension
  children (for example `Image -> .png/.jpeg/.jpg/.tiff/.webp`); `Other` is a leaf catch-all bucket for unclassified
  files. The persisted `Resources.wsresources` flat path list is preserved for the future file-support layer and is
  not rewritten by the current sidebar taxonomy view.
- `progress`: `WhatSonProgressHierarchy{Store,Parser,Creator}` (`Progress.wsprogress`)
- Progress hierarchy now renders a runtime-only read-only Figma `45:3409` taxonomy instead of exposing the persisted
  `Progress.wsprogress` state array directly. The sidebar rows are fixed to `First draft`, `Modified draft`,
  `In Progress`, `Pending`, `Reviewing`, `Waiting for approval`, `Done`, `Lagacy`, `Archived`, and
  `Delete review`, with exact LVRS icon names `inlineinlineEdit`, `rendererKit`, `progressresume`, `pending`,
  `showLogs`, `toolWindowTimer`, `validator`, `nodesexcludeRoot`, `projectModels`, and
  `gutterCheckBoxIndeterminate@14x14`. The first four rows keep visible chevrons per Figma metadata even though the
  current runtime layer does not yet materialize child nodes behind them. The persisted `Progress.wsprogress` file is
  preserved unchanged for the future progress-detail layer.
- `event`: `WhatSonEventHierarchy{Store,Parser,Creator}` (`Event.wsevent`)
- `preset`: `WhatSonPresetHierarchy{Store,Parser,Creator}` (`Preset.wspreset`)

Folders hierarchy file behavior (Library sidebar):

- `Folders.wsfolders` supports tree-style JSON with `schema: "whatson.folders.tree"` and
  `folders: [{id, label, children: [...] }]`.
- `depth` is not a persisted file field; it is computed at parse/view-model mapping time for sidebar rendering.
- Library sidebar filtering uses the persisted folder `id`/path as the canonical scope key, not the display `label`,
  so duplicate leaf labels under different parents stay disambiguated.
- Runtime-injected `All Library`, `Draft`, and `Today` are explicit system buckets; user-created folders such as
  `All` remain regular editable folders, but any concrete folder segment named `Today` is treated as the reserved
  smart-folder token and is dropped from library folder trees / note header assignments.
- Library sidebar rows now follow the Figma `32:500` icon contract: `All Library -> database`, `Draft ->
  generaledit`, `Today -> generalhistory`, regular folders -> `objectGroup`, and accent non-system smart folders ->
  `controllerFolder`.
- Library system buckets emit LVRS-compatible row drag roles (`draggable`, `dragAllowed`, `movable`, `dragLocked`)
  and stay pinned as the immutable `All Library -> Draft -> Today` depth-0 prefix even when LVRS editable reorder
  payloads are committed back into the library hierarchy view-model.
- Legacy list/object formats are still accepted and normalized into runtime depth entries.

Library runtime classification behavior:

- `All`: indexes `.wsnindex` entries and enriches them with `.wsnhead` metadata (`id`, created/modified
  timestamps, and related fields)
- `All`: reads each note's `.wsnbody`, extracts text inside `<body>...</body>`, and uses only that body text as
  note-list summary text; blank bodies stay visually blank instead of falling back to internal IDs or filesystem stems
- Notes whose resolved folder metadata is empty are still rendered with a user-facing `Draft` folder label in the note
  card, while the immutable `Draft` hierarchy bucket now uses the stricter raw `.wsnhead <folders>` contract described
  below.
- `All`: detects the first non-text `<resource ...>` entry in `.wsnbody`, resolves its thumbnail path against the note
  directory / hub root, and exposes that preview to the note-list card
- `All`: scans both fixed `Library.wslibrary` and dynamic `*.wslibrary` roots under each `*.wscontents`
- `All`: `LibraryNoteMutationViewModel::createEmptyNote()` routes note creation through
  `WhatSonHubNoteCreationService`, which creates a blank note directly under the active `.wslibrary` with a
  mixed-case alphanumeric `16-16` ID, persists the header/body, an empty `.wsnhistory`, an empty `.wsnversion`, a
  single `links.wsnlink`, and attachment scaffold through the existing note creators, updates `index.wsnindex` plus
  hub stat metadata, and keeps the new note selected in the current scope
- `Draft`: filters notes only when the raw `.wsnhead` `<folders>...</folders>` block is present and contains no
  concrete folder text or `<folder>` entries, so stale `.wsnindex` folder values and literal `Draft` text do not
  qualify as draft membership
- `Today`: filters notes where `<created>` or `<lastModified>` matches the current date; literal `<folder>Today</folder>`
  values are ignored rather than treated as concrete folder membership

Runtime IO components (`src/app/file/IO`):

- `WhatSonIoEventListener`: accepts LVRS/runtime event names with prefix filtering and queues IO events.
- `WhatSonSystemIoGateway`: owns filesystem UTF-8 read/write/append/remove and directory utility operations.
- `WhatSonIoRuntimeController`: bridges queued IO events to system IO operations (`io.ensureDir`,
  `io.writeUtf8`, `io.appendUtf8`, `io.readUtf8`, `io.removeFile`) and stores structured last-result output.

Bookmarks runtime behavior:

- `.wsnhead` `<bookmarks state="...">` is parsed into bookmark state (`bool`) and bookmark colors (`string list`)
- `.wsnhead` now also persists a numeric `<fileStat>` block for detail statistics:
  `totalFolders`, `totalTags`, `letterCount`, `wordCount`, `sentenceCount`, `paragraphCount`,
  `spaceCount`, `indentCount`, `lineCount`, `openCount`, `modifiedCount`, `backlinkToCount`,
  `backlinkByCount`, and `includedResourceCount`
- Note creation/update rewrites that `<fileStat>` block from the current header/body state, while
  editor note-selection increments `openCount` and refreshes incoming backlink counts for the
  opened note
- Bookmark colors support name tokens and hex tokens; both are normalized to hex for note-list rendering
- Bookmarks hierarchy list is derived from runtime note records and includes only notes where `bookmarked == true`
- `WhatSonBookmarksHierarchyStore` maintains a canonical 10-color hex criteria set that matches `.wsnhead` bookmark color
  tokens:
  `#EF4444`, `#F97316`, `#F59E0B`, `#EAB308`, `#22C55E`, `#14B8A6`, `#3B82F6`, `#B589EC`, `#8B5CF6`, `#EC4899`
- The full bookmarks hierarchy now renders title-cased color labels (`Red` ... `Pink`) and inserts `Indigo`
  between `Blue` and `Purple`; every hierarchy row uses the bookmark glyph, and both the glyph and label are tinted
  from LVRS theme tokens instead of the default hierarchy placeholder/icon colors. Bookmark rows explicitly disable
  the LVRS fallback placeholder through `HierarchyItem.iconPlaceholderVisible = false`, so the bookmark glyph is the
  only visible icon layer.

## Unified Build And Launch Automation

`scripts/build_all.py` now orchestrates platform-split scripts:

- `scripts/build_host.py`: build + launch on the current development machine
- `scripts/build_android.py`: build + install + launch on Android, then export Android Studio artifact
- `scripts/build_ios.py`: generate/refresh the iOS Xcode project for manual Xcode device testing

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
python3 scripts/build_ios.py
python3 scripts/build_host.py --jobs 4
```

For iOS Xcode-project generation, `scripts/build_ios.py` resolves
`WHATSON_IOS_DEVELOPMENT_TEAM` from local `Apple Development`
code-signing identities and writes that signing metadata into the generated
`.xcodeproj`. When multiple Apple Development teams are present, set the team
explicitly before running the script:

```bash
export WHATSON_IOS_DEVELOPMENT_TEAM=GRWGSK8RDF
python3 scripts/build_ios.py
```

Then open the generated project in Xcode, select the connected iPhone/iPad as
the run destination, and launch the `WhatSon` scheme from Xcode.

Task selection through orchestrator:

```bash
python3 scripts/build_all.py --tasks host,android,ios
python3 scripts/build_all.py --tasks host --no-host-run
python3 scripts/build_all.py --tasks ios
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

Logs are written to `automation-logs/*.log` by default.
Default build and artifact directories are:

- Standard host build directory: `build`
- Trial packaging build directory: `build-trial`
- iOS Xcode project: `build/ios-xcode-artifact/WhatSon.xcodeproj`
- Android Studio project: `build/android-studio-artifact`
- Linux staged install tree: `build/dist`

The generated iOS configure path disables optional `Qt6GrpcQuick` / `Qt6ProtobufQuick`
package discovery because WhatSon does not use those modules and cross-compiling may
otherwise emit host `protoc` warnings.
The generated iOS Xcode project keeps automatic signing metadata (`DEVELOPMENT_TEAM`,
bundle identifier, optional code-sign identity) so the final physical-device build can
be completed from Xcode against the selected run destination.
The app-side iOS startup path now also prefers UIKit-managed windowing/insets together
with the LVRS `LowTier` render preset during physical-device testing. That reduces
first-frame Metal swapchain churn compared with the framework's default full-window
coverage path on iOS.
The host `WhatSon` target no longer depends on iOS Xcode project export, so macOS app
builds are not blocked by stale cross-compile metadata under `build/ios-xcode-artifact`.
The explicit `whatson_export_xcodeproj` target now clears only the nested iOS CMake
cache/state files before reconfiguring, which keeps the export reproducible without
deleting the entire artifact directory.
When the host task runs, the automation still configures both `build` and `build-trial`.
`build` is used for the normal runnable host build, while `build-trial` is reserved for the
trial packaging pass and builds the `whatson_package` target there.
Manual root CMake host builds no longer mirror that behavior automatically, so CLion and plain
`cmake --build build` stay focused on the incremental runnable host tree unless
`WHATSON_ENABLE_TRIAL_BUILD_MIRROR=ON` is set explicitly.

You can override artifact locations:

```bash
python3 scripts/build_all.py \
  --host-build-dir build \
  --trial-build-dir build-trial \
  --ios-project-dir build/ios-xcode-artifact \
  --android-studio-dir build/android-studio-artifact
```

## Runtime Smoke Matrix

`scripts/runtime_smoke_matrix.py` provides an optional execution-focused verification utility on top of `build_all.py`.
It is not a maintained project test suite or a default completion gate.
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

- Logs: `runtime-matrix-logs/*.log`
- Artifacts: `runtime-matrix-artifacts/`
