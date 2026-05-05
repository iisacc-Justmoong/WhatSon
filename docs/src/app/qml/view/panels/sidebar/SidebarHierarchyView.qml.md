# `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

## LVRS Token Notes
- Compact footer button backgrounds and footer spacing use `LV.Theme.accentTransparent` and `LV.Theme.gapNone`.
- LVRS does not currently expose named scroll-physics velocity/deceleration tokens, so hierarchy kinetic scrolling
  keeps the existing scaled physics constants while visual colors, spacing, and fixed extents use named `LV.Theme`
  tokens.

## Role
This is the main visual host for the hierarchy sidebar.

It renders the hierarchy tree, search/header/footer affordances, toolbar switching, rename overlay placement, note-drop hover feedback, and bookmark-specific palette visuals. It is the most reusable sidebar surface in the QML tree.

## Composition Model
The file delegates several responsibilities to inline helper `QtObject`s inside the mounted view.
- `hierarchySelectionController`: hierarchy multi-selection state, modifier recovery, and primary activation routing.
- `renameController`: rename label normalization and rename transaction handling.
- `noteDropController`: hierarchy hit testing, drag payload decoding, note-drop preview, and drop commit.
- `bookmarkPaletteController`: bookmark color token lookup and canvas glyph drawing.

These helpers must bind their required dependencies explicitly at construction time. The workspace shell can otherwise
instantiate the LVRS route but fail while creating `SidebarHierarchyView.qml`, leaving the app with a visible blank
window even though startup runtime domains loaded successfully.

Bookmark rows intentionally keep a single `bookmarksbookmark` icon token, but the bookmark domain
may also provide a color-specific `iconSource` override from C++.
The QML palette controller is therefore a visual helper, not the source of truth for bookmark icon
color identity.

The root file still exposes wrapper functions for these helpers so callers can use a stable interface.
The same wrapper rule now applies to hierarchy selection, so pointer-selection behavior moved out of the root file
without changing delegate call sites.

## Important Inputs
- `hierarchyController`: the active hierarchy state provider.
- `hierarchyInteractionBridge`: rename, create, delete, single-row expansion, and bulk expansion bridge.
- `hierarchyDragDropBridge`: reorder and note-drop bridge.
- appearance controls such as panel color, insets, toolbar icon names, and search configuration.

## Surface Contract
- `panelColor` is transparent on desktop so the sidebar inherits the shared `ApplicationWindow` canvas.
- `searchFieldBackgroundColor` is also transparent on desktop; only the inline text affordance remains, without a
  second search-panel slab.
- The LVRS row primitive used underneath this view keeps inactive hierarchy rows transparent, so only active, hover,
  pressed, and drag states paint explicit fills.
- The visible hierarchy model is now held in `displayedHierarchyModel`, a view-owned snapshot. The view compares the
  projected hierarchy payload against the previous snapshot and only replaces the rendered model when the actual row
  content changed.
- The embedded `LV.Hierarchy` now lets the shared `TapHandler` explicitly approve flick takeover, so vertical swipes on
  mobile routes are not trapped by row taps and the underlying LVRS hierarchy scroller can keep its inertial carry.
- The host now also drives `LV.Hierarchy.listOvershootEnabled`, `listFlickDeceleration`,
  `listMaximumFlickVelocity`, and `listReboundDuration` from an explicit mobile kinetic profile, so hierarchy scrolling
  keeps touch momentum even when the active mobile route needs a stronger guarantee than the default theme profile.

## Important Outputs
- `searchSubmitted(...)`
- `searchTextEdited(...)`
- `hierarchyItemActivated(...)`
- `toolbarIndexChangeRequested(...)`
- `viewHookRequested`

These signals make the file a reusable visual surface instead of a hard-coded one-off sidebar.

## Entry and Event Reload Hooks

- When `hierarchyController` changes (domain entry/switch), the view now calls
  `hierarchyController.requestControllerHook()` if the active domain provides it.
- `onHierarchyNodesChanged` no longer calls `requestControllerHook()` directly.
  Instead, it only resynchronizes LVRS selection/focus presentation.
- `onHierarchyNodesChanged` also refreshes `displayedHierarchyModel`, but only when the serialized row payload actually
  differs. Periodic hierarchy refresh signals with unchanged content therefore no longer force a visible tree rebuild.
- `requestHierarchyControllerReload(reason)` now explicitly ignores `reason == "hierarchy.nodes.changed"` to prevent recursive reload loops when domain hooks emit `hierarchyModelChanged` during projection refresh.

## Selected Row Activation Contract

- `syncSelectedHierarchyItem(...)` now resolves a stable selected-row key via
  `selectedHierarchyItemActivationKey()`.
- Programmatic activation now prefers `LV.Hierarchy.activateListItemByKey(...)` and only falls back to
  `activateListItemById(...)` when no usable key is available.
- This prevents post-insert index drift from activating the wrong row when a new hierarchy item is created and
  surrounding rows are reindexed by LVRS model refresh timing.
- Folder creation now immediately re-syncs the primary hierarchy selection back from the controller, promotes the created
  row to the active LVRS item, and starts inline rename when the active domain supports renaming. This keeps newly
  created hierarchy items ready for direct user editing instead of leaving focus parked on the parent row.

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
- `selectedHierarchyIndices` stores the visual multi-selection set, while `hierarchyController.setHierarchySelectedIndex(...)`
  still tracks the primary routed folder.
- Because LVRS hierarchy rows expose only one native active item, additional selections are rendered by
  `hierarchySelectionOverlayLayer` through `selectedHierarchyOverlayRects`.
- `selectedHierarchyIndices` is an alias into `SidebarHierarchySelectionController.selectedIndices`, so startup binding
  churn can briefly expose it as `undefined` before the controller republishes its default `[]` value.
- Direct `.length` checks in change handlers must therefore route through `safeSelectedHierarchyIndices`, which
  normalizes the alias back to an array and prevents `undefined.length` runtime exceptions during startup or domain
  swaps.
- `collectSelectedHierarchyOverlayRects()` must always return an array. If that helper falls through
  without `return overlayRects;`, the overlay layer will read `.length` from `undefined` and the
  sidebar loses multi-selection presentation at runtime.

## Footer View Options

- The right-most `LV.ListFooter` menu button now opens a dedicated `LV.ContextMenu` anchored from the footer edge.
- The `LV.ListFooter` buttons are routed through `onButtonClicked` and `handleHierarchyFooterButtonClicked(...)`.
  Footer button configs carry data-only `eventName` values instead of owning the action callbacks themselves. This keeps
  the hierarchy action dispatch on the sidebar view side and avoids losing create/delete/options behavior when LVRS
  normalizes or rebinds the footer slot objects.
- The compact footer/menu metrics now route through `LV.Theme.gap2`, `LV.Theme.gap4`, and named token compositions
  (`144`, `78`, `24`) instead of fixed sidebar-local pixel literals, so mobile/desktop LVRS scale stays consistent.
- That menu exposes `Expand All` and `Collapse All` actions in English, which matches the repository rule that
  project-facing strings stay in English.
- Bulk expansion is enabled only when the projected hierarchy model currently contains at least one row with
  `showChevron: true`.
- Both actions route through `HierarchyInteractionBridge.setAllItemsExpanded(...)` so the active domain controller, not
  the view, owns the persisted `expanded` state.
- The menu entries now carry explicit `eventName` values (`hierarchy.expandAll` / `hierarchy.collapseAll`) and route
  through a shared trigger handler that accepts both `onItemTriggered(index)` and
  `onItemEventTriggered(eventName, ...)`, preventing callback-type differences in LVRS context-menu dispatch from
  dropping bulk expansion actions.
- The same menu entries also provide direct `onTriggered` callbacks. `LV.ContextMenu` dispatches event specs, item
  callbacks, and generic item triggers in one click path, so `hierarchyViewOptionsTriggerQueuedAction` coalesces that
  turn and guarantees that one user click performs exactly one bulk expand/collapse request.

## Folder Context Menu

- The same `LV.ContextMenu` instance is now reused for library-folder right-click actions instead of creating a second
  popup owner.
- Folder context-menu pointer invocation is centralized through `openHierarchyFolderContextMenuFromPointer(...)`, so
  desktop right-click and mobile long-press share the same hit testing, selection promotion, and protected-folder
  filtering.
- Right-click hit testing routes through `SidebarHierarchyNoteDropController.noteDropTargetAtPosition(...)`, so the menu
  opens only when the pointer is over a concrete visible hierarchy row.
- The folder menu is intentionally limited to library folder nodes (`folder:*` / `uuid`-backed entries). Protected
  system buckets such as `All`, `Draft`, and `Today` do not open this menu.
- Before the menu opens, the clicked folder is promoted to the primary hierarchy selection through the existing
  selection-routing path. This keeps context-menu actions aligned with the same selected-folder state used elsewhere in
  the sidebar.
- The menu exposes only two actions for now:
  - `New Folder`, which forwards to the existing `HierarchyInteractionBridge.createFolder()` path after selecting the
    clicked folder, so the library controller creates the new folder as that folder's child.
    After creation, the sidebar promotes the new row to the active selection and begins inline rename.
  - `Delete Folder`, which forwards to the existing `HierarchyInteractionBridge.deleteSelectedFolder()` path after
    selecting the clicked folder.
- No new backend/service object is introduced for this behavior; the sidebar only reuses the existing selection bridge,
  CRUD bridge, and menu popup.

## Expansion Routing Guard

- Expansion state is now treated as user-owned UI state. `hierarchyExpansionStateByKey` captures row expansion by
  stable hierarchy key, and `syncDisplayedHierarchyModel(...)` overlays that state onto refreshed hierarchy payloads.
  Count-only changes, folder-structure refreshes, and note-to-folder assignment refreshes must therefore not reset
  existing expanded/collapsed rows.
- Expansion keys are scoped by the active hierarchy index, so identical row ids in different hierarchy domains do not
  leak expansion state into each other when the toolbar switches domain.
- Programmatic LVRS expansion changes are not persisted back into the Controller. The sidebar arms an expansion request
  only when the pointer press lands on a row's chevron slot, and `onListItemExpanded` reverts any unarmed expansion
  change back to the preserved state.
- `syncSelectedHierarchyItem(...)` refuses to activate a selected row that is hidden behind a collapsed ancestor. This
  prevents LVRS active-item normalization from auto-expanding ancestors during model rebuilds.
- Chevron-driven expansion now records a resolved hierarchy index from stable model ids first
  (`item.itemId`, then `item.resolvedItemId`, then callback `itemId`), and only falls back to
  visual row indexes (`flatIndex`, then callback `index`) when model ids are unavailable.
  It then starts a short activation-block timer.
- `onListItemActivated` is deferred by one turn (`Qt.callLater`) and re-checked through
  `shouldSuppressHierarchyActivation(item, itemId, index)` before it can select the folder or emit
  `hierarchyItemActivated(...)`.
- Expansion suppression is now absolute for that short window: any activation is ignored while the
  expansion block timer is active, including callbacks that resolve to the currently selected row.
- This prevents LVRS internal active-row normalization from leaking into the application selection
  state when a chevron expand/collapse triggers a stray activation on a lower visible row.
- This keeps the suppression stable even when LVRS emits activation and expansion callbacks in
  different order on mobile touch input.
- `armHierarchyExpansionActivationSuppression(...)` now also increments
  `hierarchyActivationPendingSerial`, so already queued activation callbacks from the same pointer
  transaction are invalidated immediately when expansion is detected.
- Activation and expansion bridge calls both use the same resolved hierarchy index, so deep rows
  such as `Resources > Other` map to the correct controller item even when LVRS emits visual index
  values that differ from source-model ids.
- Integer parsing now avoids `Number(value) || -1` in this surface, so valid zero-based ids/indexes
  (notably first rows and progress `0`) no longer collapse into `-1`.
- After a successful expand/collapse bridge call, the view resynchronizes `selectedFolderIndex`
  back into `LV.Hierarchy` active presentation on the next turn so LVRS visual active state cannot
  remain parked on an internally normalized row.
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
- Starting, committing, and cancelling inline rename now force a `displayedHierarchyModel` refresh so the temporary
  blank-label projection is entered and exited immediately with the transaction state.
- That rename projection now hides only the visible `label`. Stable row identity fields stay intact so LVRS activation
  continues to target the same hierarchy item instead of surfacing internal fallback identifiers.
- `resolveVisibleHierarchyItem(...)` prefers the active LVRS row for the selected id and falls back to the shared
  hierarchy-item locator. This keeps rename placement tied to the selected folder, not whichever generated row happens
  to be first in the rebuilt tree.
- Note-drop preview state is represented by `noteDropHoverIndex`.
- The `DropArea` at the bottom of the file now routes pointer payloads into `noteIdsFromDragPayload(...)`, so a drag
  that originated from a multi-selected note-list group can assign every selected note to the hovered folder in one
  drop.
- The `DropArea` stays enabled whenever a `HierarchyDragDropBridge` is wired. It must not gate itself on
  `noteDropContractAvailable`, because that prevents drag enter/drop events from reaching the controller at all. The
  controller and domain Controller capability remain responsible for accepting or rejecting a concrete target.
- That folder-drop path depends on `SidebarHierarchyNoteDropController.normalizeNoteIds(...)` returning a concrete
  array. If the helper falls through without `return normalized;`, the sidebar will reject every note-list-to-folder
  drop as an empty payload.
- The same `DropArea` now mirrors the drop-commit boolean into `drop.accepted`, so failed folder assignment attempts
  remain visibly rejected by the QML drag/drop contract instead of looking handled.
- The note-drop hover pulse uses explicit `NumberAnimation.from` / `to` pairs on both opacity segments; bare numeric
  tokens are not accepted by qmlcache and must never replace keyed animation properties.

## Architectural Reading
This file should be read as a composed view, not as the place where hierarchy business rules live. If a change requires concrete knowledge about whether a domain can rename, reorder, or accept notes, the answer should come from the bridges and capability interfaces, not from hard-coded QML assumptions.

## Recent Updates
- Added `pragma ComponentBehavior: Bound` at the file root so toolbar `Repeater` delegates can
  reference `sidebarHierarchyView` id members with bound component scope.
- Added modifier-based hierarchy multi-selection (`Cmd/Ctrl` toggle, `Shift` range) with an explicit
  overlay highlight path for non-primary selected rows.
- Bound the inline helper controllers to `sidebarHierarchyView`, `hierarchyTree`, `hierarchyRenameField`,
  `HierarchyInteractionBridge`, `HierarchyDragDropBridge`, and the bookmark overlay canvas at startup so missing
  required-property initialization cannot abort the workspace route.
- Added press-time modifier capture and cached activation-modifier resolution so modifier-selection does not regress when
  LVRS activation callbacks are delivered after pointer-up.
- Restored explicit `NumberAnimation.from` keys on the note-drop hover pulse so Xcode/qmlcache ahead-of-time parsing
  accepts the sidebar animation object again.
- Added mobile flick-takeover approval to the shared `LV.Hierarchy` host so the hierarchy list keeps inertial carry on
  touch routes without changing the existing desktop routing hooks.
- Added an explicit mobile kinetic-scroll configuration layer on top of `LV.Hierarchy`, so the sidebar does not rely on
  LVRS defaults alone to preserve post-release touch momentum.

## Tests

- `test/cpp/suites/contents_display_view_tests.cpp` locks the footer context-menu action contract by checking the
  explicit expand/collapse event names, direct `onTriggered` callbacks, shared action normalizer, and queued-action
  duplicate guard.
- The same test file also locks the hierarchy expansion contract: refreshed models must preserve
  `hierarchyExpansionStateByKey` with active-hierarchy scoping, hidden selected rows must not be activated just to make
  them visible, and unarmed LVRS expansion changes must be reverted instead of persisted.
- Modifier-selection regression checklist for this file:
  - `Shift + click` creates contiguous hierarchy ranges from `hierarchySelectionAnchorIndex`.
  - `Cmd/Ctrl + click` toggles hierarchy rows without collapsing to single selection.
  - Activation callbacks that omit modifier bits must still honor the press-time modifier intent.
  - Chevron expand/collapse must not move the primary active selection to an unrelated sibling row.
  - Chevron expand/collapse must not emit folder activation even when the tapped row is already the
    currently selected folder.
  - Dropping a multi-selected note-list group onto a folder must attempt folder assignment for every dragged note id.
- Folder-drop acceptance must not regress to an always-empty payload because the sidebar note-drop controller omitted its
  normalized-array return path.
- Failed note-drop commits must leave `drop.accepted == false`, so the drag contract continues to report rejection to
  LVRS/Qt.
- The note-drop hover opacity animation must keep explicit `from:` / `to:` keys on both `NumberAnimation` blocks so
  qmlcache parsing does not fail on bare numeric tokens.
- Build-time regression guard: the first pulse segment must keep `from: 0.78` and the second must keep `from: 1.0`;
  replacing either with a bare numeric line breaks qmlcache code generation.
- Startup regression guard: inline helper objects must keep explicit dependency bindings, and the selection helper must
  point `view` at `sidebarHierarchyView`; otherwise QML reports uninitialized required properties during workspace
  route construction.
- Periodic hierarchy refreshes with unchanged nodes must not visibly blink, because `displayedHierarchyModel` must
  remain unchanged across equivalent `hierarchyNodesChanged` emissions.
- Right-clicking a library folder row must open a context menu with `New Folder` and `Delete Folder`.
- Triggering `New Folder` from that menu must reuse the existing folder-creation path and insert the new folder as a
  child of the clicked folder.
- Triggering `Delete Folder` from that menu must reuse the existing delete path and remove the clicked folder instead of
  whichever row was previously selected.
- Right-clicking protected library buckets such as `All`, `Draft`, or `Today` must not open the folder context menu.
- Enter-driven folder rename must not leave the row blank after commit or cancel.
- After a rename, later focus/selection/context-menu interactions must keep the folder label instead of exposing
  `itemKey`, `uuid`, or other internal identifiers.
- On mobile, hierarchy rows must allow the list viewport to take over a vertical swipe and continue scrolling after
  release with kinetic carry.
- On mobile, the host must keep `LV.Hierarchy` overshoot/flick parameters aligned with the touch runtime profile instead
  of leaving the sidebar on the desktop fallback scroll contract.
