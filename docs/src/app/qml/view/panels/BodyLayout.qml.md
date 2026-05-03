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
- Calendar overlay visibility and controller handles are forwarded to `ContentViewLayout.qml`.
- Year-calendar month/day routing is also re-emitted upward as `monthCalendarOverlayOpenRequested`, so the desktop app
  shell can swap from year view to month view without breaking the overlay ownership contract.
- `resourcesImportController`, `editorViewModeController`, and `isMobilePlatform` are forwarded to the central content
  surface.
- The central content surface no longer receives a gutter color; the note editor's left gutter rail inherits the editor
  background instead of painting a separate panel.
- `sidebarHierarchyController` is also forwarded to the central content surface so calendar note taps can switch the
  active domain back to `Library` before the editor becomes visible again.
- The desktop shell now resolves the effective deletion target for `ListBarLayout` from the active
  hierarchy first. If the active hierarchy publishes its own delete/clear-folder contract
  (`deleteNoteById`, `deleteNotesByIds`, `clearNoteFoldersById`, or `clearNoteFoldersByIds`), that
  contract is used; otherwise the shell falls back to the shared library-note mutation controller.
- The desktop shell still snapshots the active hierarchy index and hierarchy controller together when
  `SidebarHierarchyController.activeBindingsChanged()` fires, but it now also forwards the already-resolved
  `activeNoteListModel` into `ListBarLayout.qml`.
- `ListBarLayout.qml` and `ContentViewLayout.qml` therefore consume the same note-list object during hierarchy-domain
  switches. This removes one remaining split path where the list bar could keep resolving rows from a stale
  hierarchy-owned model while the center surface had already switched to the new domain note-list model.
- The desktop shell now prefers the global `noteActiveState.activeNoteListModel` for list/content binding and only
  falls back to `sidebarHierarchyController.activeNoteListModel` when the global state object is not supplied.
- The same `noteActiveState` object is forwarded through `ContentViewLayout.qml` into `ContentsDisplayView.qml`, where
  the visible editor session is attached back to the global active-note tracker. This keeps desktop note selection and
  editor-session rebinding in one active-state path.
- The contents surface now fills the center panel directly without an additional bottom-partition contract.
- Sidebar, list, and right-panel splitters continue to own the desktop width-resize flow.
- Desktop default/min right-panel widths and sidebar horizontal inset now come from named `LV.Theme` width/gap/stroke
  tokens, so shell sizing follows the same LVRS density policy as the owned panels.

## Tests

- The maintained C++ regression suite now locks the hierarchy-driven note-list rebinding contract used by this shell.
- Regression checklist:
  - The center content surface must still receive the active hierarchy controller and note-list model.
  - Sidebar/list/right-panel splitters must keep their existing resize behavior.
  - Calendar dismiss routing must still return the body shell to the editor surface.
  - A year-calendar month/day tap from the center content surface must still bubble up as a month-overlay open request.
  - A calendar note tap from the center content surface must still be able to switch the active hierarchy back to
    `Library`.
  - `Delete` / `Backspace` from `ListBarLayout` must still route to the active resources-domain
    deletion contract when the resources hierarchy owns the current list.
  - Switching between library and resources must swap the list/content bindings from one shared snapshot, so the
    previous hierarchy's note-list rows do not remain visible during the transition turn.
  - Switching from Library to Resources (or any other domain) must also replace the mounted content/list controller
    objects themselves, not only the highlighted toolbar index.
  - Switching from Resources back to Library must restore library-row metadata such as folder and tag chips from the
    same active note-list model that the editor surface uses.
  - Desktop content routing must pass `noteActiveState` into the editor host so active-note session sync is not delayed
    behind local QML note-list binding updates.
