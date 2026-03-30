# `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

## Role
This is the main visual host for the hierarchy sidebar.

It renders the hierarchy tree, search/header/footer affordances, toolbar switching, rename overlay placement, note-drop hover feedback, and bookmark-specific palette visuals. It is the most reusable sidebar surface in the QML tree.

## Composition Model
The file now delegates several responsibilities to sibling helper controllers.
- `SidebarHierarchyRenameController`: rename label normalization and rename transaction handling.
- `SidebarHierarchyNoteDropController`: hierarchy hit testing, drag payload decoding, note-drop preview, and drop commit.
- `SidebarHierarchyBookmarkPaletteController`: bookmark color token lookup and canvas glyph drawing.

Bookmark rows intentionally keep a single `bookmarksbookmark` icon token, but the bookmark domain
may also provide a color-specific `iconSource` override from C++.
The QML palette controller is therefore a visual helper, not the source of truth for bookmark icon
color identity.

The root file still exposes wrapper functions for these helpers so callers and tests can use a stable interface.

## Important Inputs
- `hierarchyViewModel`: the active hierarchy state provider.
- `hierarchyInteractionBridge`: rename, create, delete, single-row expansion, and bulk expansion bridge.
- `hierarchyDragDropBridge`: reorder and note-drop bridge.
- appearance controls such as panel color, insets, toolbar icon names, and search configuration.

## Surface Contract
- `panelColor` is transparent on desktop so the sidebar inherits the shared `ApplicationWindow` canvas.
- `searchFieldBackgroundColor` is also transparent on desktop; only the inline text affordance remains, without a
  second search-panel slab.
- The LVRS row primitive used underneath this view keeps inactive hierarchy rows transparent, so only active, hover,
  pressed, and drag states paint explicit fills.

## Important Outputs
- `searchSubmitted(...)`
- `searchTextEdited(...)`
- `hierarchyItemActivated(...)`
- `toolbarIndexChangeRequested(...)`
- `viewHookRequested`

These signals make the file a reusable visual surface instead of a hard-coded one-off sidebar.

## Entry and Event Reload Hooks

- When `hierarchyViewModel` changes (domain entry/switch), the view now calls
  `hierarchyViewModel.requestViewModelHook()` if the active domain provides it.
- The same hook is also called from `onHierarchyNodesChanged` so model mutations tied to sidebar
  actions can force a domain-level refresh path.
- This keeps model-backed projections (for example projects note membership derived from
  `.wsnhead`) synchronized when users re-enter the sidebar domain view.

## Footer View Options

- The right-most `LV.ListFooter` menu button now opens a dedicated `LV.ContextMenu` anchored from the footer edge.
- That menu exposes `Expand All` and `Collapse All` actions in English, which matches the repository rule that
  project-facing strings stay in English.
- Bulk expansion is enabled only when the projected hierarchy model currently contains at least one row with
  `showChevron: true`.
- Both actions route through `HierarchyInteractionBridge.setAllItemsExpanded(...)` so the active domain viewmodel, not
  the view, owns the persisted `expanded` state.
- The menu entries now carry explicit `eventName` values (`hierarchy.expandAll` / `hierarchy.collapseAll`) and route
  through a shared trigger handler that accepts both `onItemTriggered(index)` and
  `onItemEventTriggered(eventName, ...)`, preventing callback-type differences in LVRS context-menu dispatch from
  dropping bulk expansion actions.

## Expansion Routing Guard

- Chevron-driven expansion now records the expanded item id and starts a short activation-block timer.
- `onListItemActivated` is deferred by one turn (`Qt.callLater`) and re-checked through
  `shouldSuppressHierarchyActivation(itemId)` before it can select the folder or emit
  `hierarchyItemActivated(...)`.
- This keeps the suppression stable even when LVRS emits activation and expansion callbacks in
  different order on mobile touch input.
- The underlying LVRS `HierarchyItem` also reserves a dedicated chevron interaction slot
  (`chevronInteractionWidth`) and blocks row activation while the chevron interaction flag is active.
  This keeps desktop and mobile on the same contract: chevron tap/click only expands or collapses.
- This is specifically required for mobile routing, where `hierarchyItemActivated(...)` is treated as
  "open the note-list page for this folder". A chevron tap must only fold or unfold the hierarchy.

## Drag and Rename Behavior
- Rename state is represented by `editingHierarchyIndex` and `editingHierarchyLabel`.
- Inline rename geometry is no longer derived only from the live LVRS row object. The view now keeps an
  `editingHierarchyPresentation` snapshot so the overlay stays attached to the selected row even if `LV.Hierarchy`
  regenerates items during a rename transaction.
- `resolveVisibleHierarchyItem(...)` prefers the active LVRS row for the selected id and falls back to the shared
  hierarchy-item locator. This keeps rename placement tied to the selected folder, not whichever generated row happens
  to be first in the rebuilt tree.
- Note-drop preview state is represented by `noteDropHoverIndex`.
- The `DropArea` at the bottom of the file routes pointer payloads into `noteIdFromDragPayload(...)`, which now ultimately lives in the note-drop controller helper.

## Architectural Reading
This file should be read as a composed view, not as the place where hierarchy business rules live. If a change requires concrete knowledge about whether a domain can rename, reorder, or accept notes, the answer should come from the bridges and capability interfaces, not from hard-coded QML assumptions.

## Recent Updates
- Added `pragma ComponentBehavior: Bound` at the file root so toolbar `Repeater` delegates can
  reference `sidebarHierarchyView` id members with bound component scope.
