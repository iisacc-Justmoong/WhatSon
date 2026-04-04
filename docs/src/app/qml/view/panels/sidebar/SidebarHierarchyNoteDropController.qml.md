# `src/app/qml/view/panels/sidebar/SidebarHierarchyNoteDropController.qml`

## Responsibility

`SidebarHierarchyNoteDropController.qml` centralizes note-to-folder drop decoding for the hierarchy sidebar.

## Behavior

- Resolves the hovered hierarchy row from raw pointer coordinates and exposes that as a normalized drop target.
- Normalizes drag payloads into unique note-id arrays, including multi-selection payloads exported from
  `ListBarLayout.qml`.
- Uses `HierarchyDragDropBridge.canAcceptNoteDropList(...)` / `assignNotesToFolder(...)` when available, with
  single-note fallback for older capability surfaces.
- Clears hover preview state whenever the payload is empty or the hovered folder cannot accept any of the dragged notes.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - JSON `application/x-whatson-note-ids` payloads must decode into a unique ordered note-id list.
    - Plain-text newline-separated payloads must still decode into the same note-id set.
    - A multi-selected note-list drag should highlight a folder when at least one dragged note can be assigned there.
    - Dropping that payload should route every assignable dragged note through the hierarchy drag/drop bridge.
