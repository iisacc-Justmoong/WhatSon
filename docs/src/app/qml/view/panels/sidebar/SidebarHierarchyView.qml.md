# `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

## Role
This is the main visual host for the hierarchy sidebar.

It renders the hierarchy tree, search/header/footer affordances, toolbar switching, rename overlay placement, note-drop hover feedback, and bookmark-specific palette visuals. It is the most reusable sidebar surface in the QML tree.

## Composition Model
The file now delegates several responsibilities to sibling helper controllers.
- `SidebarHierarchySelectionController`: hierarchy multi-selection state, modifier recovery, and primary activation
  routing.
- `SidebarHierarchyRenameController`: rename label normalization and rename transaction handling.
- `SidebarHierarchyNoteDropController`: hierarchy hit testing, drag payload decoding, note-drop preview, and drop commit.
- `SidebarHierarchyBookmarkPaletteController`: bookmark color token lookup and canvas glyph drawing.

Bookmark rows intentionally keep a single `bookmarksbookmark` icon token, but the bookmark domain
may also provide a color-specific `iconSource` override from C++.
The QML palette controller is therefore a visual helper, not the source of truth for bookmark icon
color identity.

The root file still exposes wrapper functions for these helpers so callers can use a stable interface.
The same wrapper rule now applies to hierarchy selection, so pointer-selection behavior moved out of the root file
without changing delegate call sites.

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
- `onHierarchyNodesChanged` no longer calls `requestViewModelHook()` directly.
  Instead, it only resynchronizes LVRS selection/focus presentation.
- `requestHierarchyViewModelReload(reason)` now explicitly ignores `reason == "hierarchy.nodes.changed"` to prevent recursive reload loops when domain hooks emit `hierarchyModelChanged` during projection refresh.

## Selected Row Activation Contract

- `syncSelectedHierarchyItem(...)` now resolves a stable selected-row key via
  `selectedHierarchyItemActivationKey()`.
- Programmatic activation now prefers `LV.Hierarchy.activateListItemByKey(...)` and only falls back to
  `activateListItemById(...)` when no usable key is available.
- This prevents post-insert index drift from activating the wrong row when a new hierarchy item is created and
  surrounding rows are reindexed by LVRS model refresh timing.

## Multi Selection Contract

- Hierarchy row activation now routes through `requestHierarchySelection(item, resolvedIndex, modifiers)` after the
  existing expansion-suppression guard.
- Modifier behavior:
  - plain click: single selection
  - `Shift` + click: contiguous range selection anchored by `hierarchySelectionAnchorIndex`
  - `Cmd/Ctrl` + click: additive toggle selection
- The hierarchy host now captures press-time pointer modifiers via a dedicated left-button `TapHandler`
  (`acceptedModifiers: Qt.KeyboardModifierMask`) mounted on `LV.Hierarchy`.
- `resolveHierarchySelectionModifiers(...)` uses a short-lived press cache when activation callbacks arrive without
  modifier bits, so `Cmd/Ctrl` and `Shift` gestures remain stable across platform callback timing differences.
- `selectedHierarchyIndices` stores the visual multi-selection set, while `hierarchyViewModel.setHierarchySelectedIndex(...)`
  still tracks the primary routed folder.
- Because LVRS hierarchy rows expose only one native active item, additional selections are rendered by
  `hierarchySelectionOverlayLayer` through `selectedHierarchyOverlayRects`.

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

- Chevron-driven expansion now records a resolved hierarchy index from stable model ids first
  (`item.itemId`, then `item.resolvedItemId`, then callback `itemId`), and only falls back to
  visual row indexes (`flatIndex`, active-row id, callback `index`) when model ids are unavailable.
  It then starts a short activation-block timer.
- `onListItemActivated` is deferred by one turn (`Qt.callLater`) and re-checked through
  `shouldSuppressHierarchyActivation(item, itemId, index)` before it can select the folder or emit
  `hierarchyItemActivated(...)`.
- This keeps the suppression stable even when LVRS emits activation and expansion callbacks in
  different order on mobile touch input.
- Activation and expansion bridge calls both use the same resolved hierarchy index, so deep rows
  such as `Resources > Other` map to the correct viewmodel item even when LVRS emits visual index
  values that differ from source-model ids.
- Integer parsing now avoids `Number(value) || -1` in this surface, so valid zero-based ids/indexes
  (notably first rows and progress `0`) no longer collapse into `-1`.
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
- Added modifier-based hierarchy multi-selection (`Cmd/Ctrl` toggle, `Shift` range) with an explicit
  overlay highlight path for non-primary selected rows.
- Added press-time modifier capture and cached activation-modifier resolution so modifier-selection does not regress when
  LVRS activation callbacks are delivered after pointer-up.

## Tests

- Automated test files are not currently present in this repository.
- Modifier-selection regression checklist for this file:
  - `Shift + click` creates contiguous hierarchy ranges from `hierarchySelectionAnchorIndex`.
  - `Cmd/Ctrl + click` toggles hierarchy rows without collapsing to single selection.
  - Activation callbacks that omit modifier bits must still honor the press-time modifier intent.
