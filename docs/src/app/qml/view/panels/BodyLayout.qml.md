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
- Calendar overlay visibility and viewmodel handles are forwarded to `ContentViewLayout.qml`.
- Year-calendar month/day routing is also re-emitted upward as `monthCalendarOverlayOpenRequested`, so the desktop app
  shell can swap from year view to month view without breaking the overlay ownership contract.
- `resourcesImportViewModel`, `editorViewModeViewModel`, and `isMobilePlatform` are forwarded to the central content
  surface.
- `sidebarHierarchyViewModel` is also forwarded to the central content surface so calendar note taps can switch the
  active domain back to `Library` before the editor becomes visible again.
- The contents surface now fills the center panel directly without an additional bottom-partition contract.
- Sidebar, list, and right-panel splitters continue to own the desktop width-resize flow.
- Desktop default/min right-panel widths and sidebar horizontal inset now come from `LV.Theme.scaleMetric(...)` /
  `LV.Theme.gap2`, so shell sizing follows the same LVRS density policy as the owned panels.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - The center content surface must still receive the active hierarchy viewmodel and note-list model.
  - Sidebar/list/right-panel splitters must keep their existing resize behavior.
  - Calendar dismiss routing must still return the body shell to the editor surface.
  - A year-calendar month/day tap from the center content surface must still bubble up as a month-overlay open request.
  - A calendar note tap from the center content surface must still be able to switch the active hierarchy back to
    `Library`.
