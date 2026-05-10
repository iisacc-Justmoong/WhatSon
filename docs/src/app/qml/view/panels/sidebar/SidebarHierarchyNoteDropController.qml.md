# `src/app/qml/view/panels/sidebar/SidebarHierarchyNoteDropController.qml`

## Responsibility

`SidebarHierarchyNoteDropController.qml` centralizes note-to-folder drop decoding for the hierarchy sidebar.

## Behavior

- Resolves the hovered hierarchy row from raw pointer coordinates and exposes that as a normalized drop target.
- `noteDropIndexAtPosition(...)` must delegate to the same target resolver and return the resolved target index, so
  callers never receive `undefined` from an otherwise valid note-drop hit test.
- Normalizes drag payloads into unique note-id arrays, including multi-selection payloads exported from
  `ListBarLayout.qml`.
- `normalizeNoteIds(...)` must always return the deduplicated array; a missing return collapses folder-drop acceptance
  to an empty payload and blocks note assignment entirely.
- `collectHierarchyItems()` must always return the discovered hierarchy row list because sibling sidebar helpers reuse
  the same item locator contract for hover, palette, and drop-adjacent behaviors.
- Uses `HierarchyDragDropBridge.canAcceptNoteDropList(...)` / `assignNotesToFolder(...)` when available, with
  single-note fallback for older capability surfaces.
- Clears hover preview state whenever the payload is empty or the hovered folder cannot accept any of the dragged notes.
  The controller, not the outer `DropArea.enabled` binding, owns this target-level accept/reject decision.
- The outer hierarchy drop surface is still restricted to `whatson.library.note` drag keys. Hierarchy item drags must
  remain outside the note-drop surface so LVRS can perform tree reorder through its own `listItemMoved` signal.

## Regression Notes

- `test/cpp/suites/sidebar_hierarchy_rename_controller_tests.cpp` locks the note-drop surface contract so the `DropArea`
  remains open for note drags, rejects empty payloads, and does not intercept hierarchy item reorder drags.
- Regression checklist:
    - JSON `application/x-whatson-note-ids` payloads must decode into a unique ordered note-id list.
    - Plain-text newline-separated payloads must still decode into the same note-id set.
    - A multi-selected note-list drag should highlight a folder when at least one dragged note can be assigned there.
    - Dropping that payload should route every assignable dragged note through the hierarchy drag/drop bridge.
    - `noteDropIndexAtPosition(...)` must return the resolved target index, not fall through to `undefined`.
    - `normalizeNoteIds(...)` and `collectHierarchyItems()` must keep explicit `return normalized;` /
      `return items;` statements so QML does not silently produce `undefined`.
