# `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`

## Role
This is the main visual host for the hierarchy sidebar.

It renders the hierarchy tree, search/header/footer affordances, toolbar switching, rename overlay placement, note-drop hover feedback, and bookmark-specific palette visuals. It is the most reusable sidebar surface in the QML tree.

## Composition Model
The file now delegates several responsibilities to sibling helper controllers.
- `SidebarHierarchyRenameController`: rename label normalization and rename transaction handling.
- `SidebarHierarchyNoteDropController`: hierarchy hit testing, drag payload decoding, note-drop preview, and drop commit.
- `SidebarHierarchyBookmarkPaletteController`: bookmark color token lookup and canvas glyph drawing.

The root file still exposes wrapper functions for these helpers so callers and tests can use a stable interface.

## Important Inputs
- `hierarchyViewModel`: the active hierarchy state provider.
- `hierarchyInteractionBridge`: rename, create, delete, and expansion bridge.
- `hierarchyDragDropBridge`: reorder and note-drop bridge.
- appearance controls such as panel color, insets, toolbar icon names, and search configuration.

## Important Outputs
- `searchSubmitted(...)`
- `searchTextEdited(...)`
- `hierarchyItemActivated(...)`
- `toolbarIndexChangeRequested(...)`
- `viewHookRequested`

These signals make the file a reusable visual surface instead of a hard-coded one-off sidebar.

## Expansion Routing Guard

- Chevron-driven expansion now arms a one-turn suppression fence before the LVRS expansion callback
  is forwarded to the interaction bridge.
- While that fence is active, `onListItemActivated` does not re-emit `hierarchyItemActivated(...)`.
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
