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
- Approximate line count: 756

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

- The note list keeps `boundsBehavior` and `boundsMovement` on `Flickable.StopAtBounds` so the viewport does not
  overshoot and rebound when the user hits the first or last row.
- `flickDeceleration` is forced to `1000000` and `onFlickStarted: noteListView.cancelFlick()` cancels inertial carry,
  so the list behaves like immediate drag scrolling instead of a kinetic panel.
- `noteListScrollTick` is fixed to `LV.Theme.gap2`, and all viewport motion is quantized through
  `noteListMaxContentY()`, `quantizedNoteListContentY(value)`, `applyNoteListViewportStep(contentY)`, and
  `settleNoteListViewport()`.
- The list caches `contentY` before note-model reset cycles and restores it after the reset settles, so periodic
  note-list refreshes do not yank the viewport back to the first row.
- `LV.WheelScrollGuard` is mounted over the list viewport so wheel input follows the same small-step contract as drag
  scrolling.

## Interaction Contract

- The panel enters note-list mode whenever an actual `noteListModel` is injected. It must not hard-code
  hierarchy toolbar indexes such as `Library` or `Bookmarks`, because domains like `Projects` also own
  note-list projections.
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
- `syncCurrentIndexFromModel()` prevents unsolicited `ListView.currentIndex` resets from leaking back into app state.
- `NoteListModelContractBridge` centralizes search/current-index/current-note reflection and write calls so QML does
  not need to own all dynamic contract detection.
- Committed active-row state now reads `noteListContractBridge.currentIndex/currentNoteId` **property contracts**
  (not invokable snapshots), so QML binding reactivity tracks selection changes and avoids the first-row-fixed
  active-card regression.
- Delegate active styling uses `isDelegateActive(index, noteId)`:
  - first contract: `selectedNoteIndices` membership (multi-selection highlight)
  - primary contract: committed `currentIndex`
  - fallback contract: committed `currentNoteId` equality
  so the selected row stays visually active even when the model momentarily reorders or defers index stabilization.
- `FocusedNoteDeletionBridge` keeps keyboard delete behavior aligned with whichever note card is visually focused.
- Focused-note sync now reads `resolvedNoteListModel.currentNoteId` first, so keyboard-delete focus
  follows the model-authoritative current note contract.

## Drag And Context Menu

- Desktop uses immediate internal drag for note-to-folder assignment; mobile defers drag pickup to a `1000ms`
  long-press surface.
- The drag preview is reparented into the overlay layer so the carried note card can cross panel boundaries.
- Note-card context menus are centralized at the root through `contextMenuNoteId` and `openNoteContextMenu(...)`,
  which keeps delegates free of per-row popup wiring.
- Pointer-drag hot-spot fallback now uses `noteItemDelegate.width/height` explicitly, so drag-start
  math remains bound to the delegate's own touch target.

## Tests

- Automated test files are not currently present in this repository.
- Modifier-selection regression checklist for this file:
  - `Shift + click` selects contiguous ranges from `noteSelectionAnchorIndex`.
  - `Cmd/Ctrl + click` toggles row membership without collapsing to single-row selection.
  - `Cmd/Ctrl + Shift + click` unions the anchor range with existing selection.
  - Releasing modifier keys near pointer-up must still respect press-time intent.
