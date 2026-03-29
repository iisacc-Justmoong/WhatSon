# `src/app/qml/view/panels/ListBarLayout.qml`

## Responsibility

`ListBarLayout.qml` is the shared note-list surface for desktop and mobile routes. It owns note selection authority,
search propagation, context-menu targeting, internal note drag preview, and the viewport behavior of the note list
itself.

## Source Metadata
- Source path: `src/app/qml/view/panels/ListBarLayout.qml`
- Source kind: QML view/component
- File name: `ListBarLayout.qml`
- Approximate line count: 707

## QML Surface Snapshot
- Root type: `Rectangle`

### Object IDs
- `listBarLayout`
- `noteSelectionState`
- `noteDragPreviewState`
- `noteDeletionBridge`
- `noteContextMenu`
- `noteDragPreview`
- `topToolbar`
- `noteListView`
- `noteItemDelegate`
- `noteCard`
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
- `activateNoteIndex(index, noteId)` is the only immediate note-activation path. It updates `currentIndex`, pushes the
  authoritative selection back into the bound note-list model, captures `focusedNoteId` for keyboard deletion, and
  reasserts the pending user choice after the current event turn.
- `syncCurrentIndexFromModel()` prevents unsolicited `ListView.currentIndex` resets from leaking back into app state.
- `FocusedNoteDeletionBridge` keeps keyboard delete behavior aligned with whichever note card is visually focused.

## Drag And Context Menu

- Desktop uses immediate internal drag for note-to-folder assignment; mobile defers drag pickup to a `1000ms`
  long-press surface.
- The drag preview is reparented into the overlay layer so the carried note card can cross panel boundaries.
- Note-card context menus are centralized at the root through `contextMenuNoteId` and `openNoteContextMenu(...)`,
  which keeps delegates free of per-row popup wiring.

## Tests

- `tests/app/test_qml_binding_syntax_guard.cpp` guards selection authority, drag wiring, delete shortcuts, and the
  non-inertial quantized note-list scroll contract.
