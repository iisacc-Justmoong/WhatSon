# `src/app/qml/view/panels/ContentViewLayout.qml`

## Responsibility

`ContentViewLayout.qml` switches the center content slot between the editor surface and the calendar surfaces.
It owns the platform mode for the unified editor host and forwards shared editor collaborators into that one surface.

## Current Contract

- Shared editor surface: `ContentsDisplayView.qml`
- Mobile/desktop host variance: `mobileHost` input forwarded from `isMobilePlatform`
- Calendar surfaces:
  - `AgendaPage.qml`
  - `DayCalendarPage.qml`
  - `WeekCalendarPage.qml`
- `MonthCalendarPage.qml`
- `YearCalendarPage.qml`
- The editor surface now always fills the full content slot and no longer forwards any bottom-partition sizing
  contract.
- Calendar note activation is also centralized here so every calendar surface can reuse one shared "open note in
  editor" bridge.

## Signals

- `editorTextEdited(string text)`
- `dayCalendarOverlayCloseRequested`
- `agendaOverlayCloseRequested`
- `monthCalendarOverlayOpenRequested`
- `monthCalendarOverlayCloseRequested`
- `weekCalendarOverlayCloseRequested`
- `viewHookRequested`
- `yearCalendarOverlayCloseRequested`

## Routing Notes

- A `StackLayout` selects either the editor surface or the active calendar surface.
- The active note-list model still closes any visible calendar surface when note selection changes.
- `resourcesImportViewModel`, `editorViewModeViewModel`, `sidebarHierarchyViewModel`, and the resolved
  note-list/content viewmodels are forwarded into the unified editor host.
- That sidebar-hierarchy forwarding is now part of the editor contract so desktop/mobile surfaces can distinguish:
  - direct resource-package browsing inside the Resources hierarchy
  - ordinary notes from other hierarchies whose `.wsnbody` happens to contain inline `<resource ... />` blocks
- `isMobilePlatform` still decides which host mode is activated.
- `requestOpenLibraryNote(noteId)` now uses `libraryHierarchyViewModel.activateNoteById(...)` together with
  `sidebarHierarchyViewModel.setActiveHierarchyIndex(...)` so a calendar note tap can switch the active hierarchy back
  to Library, select the note, and then dismiss the current calendar overlay.
- `YearCalendarPage.qml` can now request a month-overlay open through this file. `ContentViewLayout.qml` applies the
  requested year/month/date to `monthCalendarViewModel` first, then emits `monthCalendarOverlayOpenRequested`.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Switching between editor and calendar surfaces must continue to use one shared content slot.
  - The unified editor host must fill that slot in both desktop and mobile modes without a reserved bottom partition.
  - Note selection changes must still dismiss any visible calendar surface.
  - A year-calendar month/day tap must still propagate into a month-overlay open request with the month viewmodel
    already synchronized to the requested month/date.
  - A note tap from Agenda/day/week/month must reopen that library note and return the content slot to the editor
    surface.
  - Calendar note opening must switch the active hierarchy back to Library before the overlay is dismissed.
