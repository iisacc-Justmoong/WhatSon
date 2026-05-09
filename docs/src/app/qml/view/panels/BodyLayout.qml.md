# `src/app/qml/view/panels/BodyLayout.qml`

## Responsibility

`BodyLayout.qml` owns the desktop body shell that composes the hierarchy sidebar, list bar, content surface, and
right detail panel.

## Signals

- `listViewWidthDragRequested(int value)`
- `noteActivated(int index, string noteId)`
- `rightPanelWidthDragRequested(int value)`
- `sidebarWidthDragRequested(int value)`
- `viewHookRequested`
- `agendaOverlayDismissRequested`
- `dayCalendarOverlayDismissRequested`
- `monthCalendarOverlayOpenRequested`
- `monthCalendarOverlayDismissRequested`
- `weekCalendarOverlayDismissRequested`
- `yearCalendarOverlayDismissRequested`

## Current Routing Notes

- `ListBarLayout.noteActivated(...)` is re-emitted as `BodyLayout.noteActivated(...)`.
- Calendar overlay visibility and controller handles can still be forwarded to `ContentViewLayout.qml` as shell
  compatibility inputs, but the content surface does not mount calendar pages.
- Year-calendar month/day routing is also re-emitted upward as `monthCalendarOverlayOpenRequested`, so the desktop app
  shell can swap from year view to month view without breaking the overlay ownership contract.
- `ContentViewLayout.qml` accepts restored shell inputs for compatibility, while editor-session/import/editor-mode
  backends remain removed from the TextEditor surface.
- The desktop shell now resolves the effective deletion target for `ListBarLayout` from the active
  hierarchy first. If the active hierarchy publishes its own delete/clear-folder contract
  (`deleteNoteById`, `deleteNotesByIds`, `clearNoteFoldersById`, or `clearNoteFoldersByIds`), that
  contract is used; otherwise the shell falls back to the shared library-note mutation controller.
- The desktop shell still snapshots the active hierarchy index and hierarchy controller together when
  `SidebarHierarchyController.activeBindingsChanged()` fires, but it now also forwards the already-resolved
  `activeNoteListModel` into `ListBarLayout.qml`.
- `ContentViewLayout.qml` no longer consumes note-list objects during hierarchy-domain switches.
- The desktop shell now prefers the global `noteActiveState.activeNoteListModel` for list/content binding and only
  falls back to `sidebarHierarchyController.activeNoteListModel` when the global state object is not supplied.
- The center editor surface now mounts only the LVRS `TextEditor` path and does not attach an editor-session backend.
- The contents surface now fills the center panel directly without an additional bottom-partition contract.
- Sidebar, list, and right-panel splitters continue to own the desktop width-resize flow.
- Desktop default/min right-panel widths and sidebar horizontal inset now come from named `LV.Theme` width/gap/stroke
  tokens, so shell sizing follows the same LVRS density policy as the owned panels.

## Tests

- The maintained C++ regression suite now locks the hierarchy-driven note-list rebinding contract used by this shell.
- Regression checklist:
- The center content surface may receive active hierarchy controller, note-list model, and import controller as shell
    compatibility inputs, but it must not consume them as a TextEditor backend or receive an editor view-mode controller.
  - Sidebar/list/right-panel splitters must keep their existing resize behavior.
  - Calendar overlay routing must not be mounted by the center content surface.
  - `Delete` / `Backspace` from `ListBarLayout` must still route to the active resources-domain
    deletion contract when the resources hierarchy owns the current list.
  - Switching between library and resources must keep list bindings coherent without relying on the content surface.
  - Switching from Library to Resources (or any other domain) must also replace the mounted content/list controller
    objects themselves, not only the highlighted toolbar index.
  - Switching from Resources back to Library must restore library-row metadata such as folder and tag chips from the
    same active note-list model that the editor surface uses.
  - Desktop content routing may pass `noteActiveState` into the editor host as route context, but it must not reintroduce
    editor-session sync through a removed backend object.
