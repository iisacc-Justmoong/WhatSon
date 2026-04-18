# `src/app/qml/view/panels/ListBarLayout.qml`

## Responsibility

`ListBarLayout.qml` is the shared list surface for desktop and mobile routes. It owns interaction flow
(tap/drag/context-menu/viewport) and now branches visual delegates between `NoteListItem` and
`ResourceListItem` by model contract, while note-list probing is delegated to C++ bridge objects.

## Composition Model

The root view now delegates note multi-selection state and modifier interpretation to
`ListBarSelectionController.qml`.

`ListBarLayout.qml` keeps wrapper functions such as `requestNoteSelection(...)` and
`syncSelectionFromCommittedState()` so delegates and peer panels continue to use a stable host API while the
selection state machine lives in a sibling controller file.

## Source Metadata
- Source path: `src/app/qml/view/panels/ListBarLayout.qml`
- Source kind: QML view/component
- File name: `ListBarLayout.qml`
- Approximate line count: 762

## QML Surface Snapshot
- Root type: `Rectangle`

### Object IDs
- `listBarLayout`
- `noteSelectionState`
- `noteSelectionController`
- `noteDragPreviewState`
- `noteDeletionBridge`
- `noteListContractBridge`
- `noteContextMenu`
- `noteDragPreview`
- `noteDragPreviewCard`
- `resourceDragPreviewCard`
- `topToolbar`
- `noteListView`
- `noteItemDelegate`
- `noteCard`
- `resourceCard`
- `noteDragHandler`
- `mobileLongPressDragArea`

### Required Properties
- `bookmarkColor`
- `bookmarked`
- `displayDate`
- `folders`
- `image`
- `imageSource`
- `index`
- `noteId`
- `primaryText`
- `tags`

### Signals
- `noteActivated`
- `viewHookRequested`

## Viewport Contract

- The visible `ListView` now renders `displayedNoteListEntries`, a view-owned snapshot array populated through
  `NoteListModelContractBridge.readAllRows()`, instead of binding delegates directly to the resetting domain model.
- On note-list model swaps, the first visible snapshot is now read against the explicit incoming model object rather
  than waiting for the bridge's own rebinding cycle, so hierarchy transitions do not depend on one event-turn of
  adapter lag before showing rows.
- `syncDisplayedNoteListEntries()` compares the new row snapshot to the previous one and only replaces the visible
  array when the actual row content changed. Periodic model resets with identical content therefore no longer tear down
  and rebuild every visible row.
- Visible row snapshots now omit `bodyText`, so editor-side same-note body rewrites no longer change the list snapshot
  signature unless another rendered field actually changed.
- Snapshot refresh requests from `itemsChanged()`, row insert/remove/move, `dataChanged`, and `layoutChanged` now
  funnel through one queued helper per event-loop turn, so one logical list refresh no longer rereads and reapplies the
  same row snapshot multiple times in immediate succession.
- When the bound `noteListModel` instance itself changes, the view now clears `displayedNoteListEntries`
  immediately, primes the snapshot from the explicit incoming model, and then performs one deferred transition sync.
  This prevents both one-frame stale-row reuse from the previous hierarchy and post-switch blank states while
  `NoteListModelContractBridge` / `Connections.target` are still rebinding to the new domain model.
- `modelReset` is now treated as viewport-lifecycle only inside this view. The actual visible-row resync is driven by
  the model's post-refresh `itemsChanged()` signal, eliminating one more duplicate snapshot-consumption path from the
  reset turn.
- `itemCountChanged()` now also queues the same snapshot refresh path as `itemsChanged()`, so a hierarchy-specific
  model that updates its row population in stages cannot strand the visible list on the cleared transition snapshot.
- The note/resource list models now emit one `itemsChanged()` per filtered-content refresh instead of emitting a second
  redundant `itemsChanged()` after `setItems()`, so `ListBarLayout.qml` no longer receives duplicate "list changed"
  notifications for the same model-reset turn.
- The note list now splits viewport motion by target form factor:
  - desktop keeps `Flickable.StopAtBounds` and still cancels inertial carry on `onFlickStarted`, so wheel/drag motion
    stays quantized to the existing narrow-step panel contract.
  - mobile switches to `Flickable.DragAndOvershootBounds` plus `Flickable.FollowBoundsBehavior`, so note rows and
    resource rows inherit kinetic overscroll instead of dead-stop dragging.
- `noteListKineticViewportEnabled` now gates the viewport contract from real mobile runtime detection
  (`LV.Theme.mobileTarget` or `Window.window.isMobilePlatform`), so adaptive/mobile routes keep kinetic scrolling even
  when the visual route and the raw theme profile would otherwise drift.
- `flickDeceleration` and `maximumFlickVelocity` are now mobile-aware tokens. Mobile uses scaled kinetic values
  (`LV.Theme.scaleMetric(2800/12000)`), while desktop keeps the previous low-velocity non-kinetic behavior through
  `noteListScrollTick`.
- `noteListScrollTick` is fixed to `LV.Theme.gap2`, but quantized viewport snapping is now desktop-only.
  Mobile still uses `clampNoteListContentY(value)` for restore/bounds repair, while live touch motion is left to the
  native `ListView` kinetic engine.
- `synchronousDrag` is also now desktop-only. Mobile stops forcing synchronous viewport drag so Qt's built-in touch
  flick pipeline can keep velocity after release.
- The list caches `contentY` before note-model reset cycles and restores it after the reset settles, so periodic
  note-list refreshes do not yank the viewport back to the first row.
- `LV.WheelScrollGuard` is mounted over the list viewport so wheel input follows the same small-step contract as drag
  scrolling.
- The list viewport inset and row spacing now come from `LV.Theme.gap2`, and the top toolbar height comes from
  `LV.Theme.gap24` instead of fixed `2px/24px` literals.

## Interaction Contract

- The panel enters note-list mode whenever an actual `noteListModel` is injected. It must not hard-code
  hierarchy toolbar indexes such as `Library` or `Bookmarks`, because domains like `Projects` also own
  note-list projections.
- The panel can now also receive only `hierarchyViewModel`; `NoteListModelContractBridge` resolves the effective
  note-list model from that hierarchy object's `hierarchyNoteListModel` / `noteListModel` contract.
  A hierarchy-domain switch therefore updates the list surface from one hierarchy input instead of depending on a
  second, separately-timed `activeNoteListModel` binding.
- `resourceListMode` is detected from `resolvedNoteListModel.currentResourceEntry`, then delegates switch to
  `ResourceListItem` so resources rows no longer inherit note-card visuals.
- `activateNoteIndex(index, noteId)` is the only immediate note-activation path. It updates `currentIndex`, pushes the
  authoritative selection back into the bound note-list model, captures `focusedNoteId` for keyboard deletion, and
  reasserts the pending user choice after the current event turn.
- Pointer selection now routes through `requestNoteSelection(index, noteId, modifiers)`:
  - plain click/tap: single selection
  - `Shift` + click/tap: range selection anchored by `noteSelectionAnchorIndex`
  - `Cmd/Ctrl` + click/tap: additive toggle selection
  - `Cmd/Ctrl + Shift` + click/tap: additive range selection (existing set + anchor-target range union)
- Modifier normalization now merges per-event modifiers with `Qt.application.keyboardModifiers`, so platform event
  payloads that drop one side of the modifier state no longer collapse multi-selection into single-row activation.
- The desktop left-click `TapHandler` explicitly accepts all modifier states (`acceptedModifiers: Qt.KeyboardModifierMask`),
  captures modifier keys on press, and keeps a short-lived cache for tap resolution.
- `resolveSelectionModifiers(...)` now restores press-time modifier intent when tap events arrive without modifier flags
  (for example, occasional `Cmd`/`Shift` loss on macOS pointer release dispatch).
- Multi-selection state is held in `selectedNoteIndices`; model-authoritative `currentIndex/currentNoteId` remains the
  primary note for downstream domain routing.
- The list now resolves `selectedNoteIndices` back into stable note ids through
  `NoteListModelContractBridge.readNoteIdAt(...)`,
  so batch note-list actions do not depend on delegate visibility.
- `syncCurrentIndexFromModel()` prevents unsolicited `ListView.currentIndex` resets from leaking back into app state.
- `NoteListModelContractBridge` centralizes search/current-index/current-note reflection and write calls so QML does
  not need to own all dynamic contract detection.
- The bridge now also exposes `readAllRows()` plus `readAllRowsForModel(QObject*)`, allowing the QML view to keep a
  stable render snapshot even while the underlying `QAbstractItemModel` goes through
  `beginResetModel()/endResetModel()` refresh cycles and while hierarchy transitions are rebinding the bridge itself.
- `resolvedNoteListModel` is now derived from the bridge, not from the raw injected `noteListModel` property.
  This is the key regression guard for toolbar/domain switches: the list resets and re-primes itself when the bridge's
  resolved model changes, even if the caller only swapped the active hierarchy viewmodel.
- Committed active-row state now reads `noteListContractBridge.currentIndex/currentNoteId` **property contracts**
  (not invokable snapshots), so QML binding reactivity tracks selection changes and avoids the first-row-fixed
  active-card regression.
- Delegate active styling uses `isDelegateActive(index, noteId)`:
  - first contract: `selectedNoteIndices` membership (multi-selection highlight)
  - primary contract: committed `currentIndex`
  - fallback contract: committed `currentNoteId` equality
  so the selected row stays visually active even when the model momentarily reorders or defers index stabilization.
- `FocusedNoteDeletionBridge` still tracks the focused single note as a fallback, but keyboard delete now prefers the
  resolved multi-selection note-id set when one exists.
- Focused-note sync still reads `resolvedNoteListModel.currentNoteId` first, so single-note delete fallback follows the
  model-authoritative current note contract.

## Drag And Context Menu

- Desktop uses immediate internal drag for note-to-folder assignment; mobile defers drag pickup to a `1000ms`
  long-press surface.
- The drag preview is reparented into the overlay layer so the carried note card can cross panel boundaries.
- Dragging a row that is already part of a multi-selection now exports the full selected note-id set, not just the
  delegate under the pointer.
- The drag preview adds a count badge when more than one selected note is being carried.
- That badge now derives its inset from `LV.Theme.gap8` and its size from `LV.Theme.scaleMetric(20/10)`, so it scales
  with LVRS mobile UI density instead of staying desktop-sized.
- Note-card context menus are centralized at the root through `contextMenuNoteId`, `contextMenuNoteIds`, and
  `openNoteContextMenu(...)`, which keeps delegates free of per-row popup wiring while preserving group actions.
- Right-clicking or long-pressing a row that already belongs to the current multi-selection preserves that selection as
  the context-menu target set.
- Keyboard delete, context-menu delete, and `Clear all folders` now replay the action across the selected note ids via
  `LibraryNoteMutationViewModel` batch helpers.
- Pointer-drag hot-spot fallback now uses `noteItemDelegate.width/height` explicitly, so drag-start
  math remains bound to the delegate's own touch target.
- While a note drag is active, visible note-row snapshot refreshes are now deferred through
  `requestDisplayedNoteListEntriesSync(...)` instead of immediately replacing `displayedNoteListEntries`.
  This prevents `itemsChanged()` / `dataChanged()` churn from tearing down the source delegate mid-drag.
- That deferred drag path now composes with the queued snapshot scheduler above, so repeated model notifications during
  the same turn still collapse into one post-drag visible snapshot apply.
- `reuseItems` is now disabled during active drag, so `ListView` does not recycle the grabbed delegate while the drag
  handler still owns pointer state.
- The note-id helper functions used by drag payload construction now return concrete arrays/booleans again, removing
  silent `undefined` paths that could abort multi-selection drag flows in QML JavaScript.

## Tests

- The maintained C++ regression suite now covers the bridge contract that this file depends on for hierarchy-driven
  note-list rebinding.
- Modifier-selection regression checklist for this file:
  - `Shift + click` selects contiguous ranges from `noteSelectionAnchorIndex`.
  - `Cmd/Ctrl + click` toggles row membership without collapsing to single-row selection.
  - `Cmd/Ctrl + Shift + click` unions the anchor range with existing selection.
  - Releasing modifier keys near pointer-up must still respect press-time intent.
  - Pressing `Delete` with multiple selected notes must delete every selected note, not only `currentNoteId`.
  - The note context menu must apply `Delete` and `Clear all folders` to the selected group when the invoked row is
    already part of that group.
  - Dragging one note from a selected group into the hierarchy sidebar must carry every selected note id in the drag
    payload.
  - Periodic note-list refreshes with unchanged row content must not visibly blink, because `displayedNoteListEntries`
    remains stable across equivalent model resets.
  - One logical note-list rebuild must not cause two immediate `displayedNoteListEntries` resync passes through
    `modelReset` plus a second post-reset `itemsChanged()` consumption path.
  - Switching from one hierarchy domain to another must not show the previous hierarchy's row snapshot for one frame
    while the new note-list model is being rebound.
  - Switching hierarchy domains by changing only the mounted hierarchy viewmodel must still replace the effective
    note-list model immediately.
  - Switching into the resources hierarchy must not leave the shared list blank before the user explicitly picks a
    resource taxonomy row.
  - A transition-time model population that only updates `itemCountChanged()` before later data notifications must
    still repopulate `displayedNoteListEntries`.
  - Periodic note-list refreshes that arrive during an active drag must be applied only after the drag ends, so the
    source delegate is not recycled out from under `noteDragHandler`.
  - On mobile, the shared note/resource list must keep inertial scrolling after touch release instead of stopping on
    the release frame.
  - On mobile, `onContentYChanged` and `onMovementEnded` must not re-quantize live touch motion back onto
    `noteListScrollTick` steps.
  - On mobile, dragging a row must still require a long press; ordinary vertical swipes must be stealable by the list
    viewport and continue scrolling with kinetic carry.
