# `src/app/qml/view/panels/ContentViewLayout.qml`

## Responsibility

`ContentViewLayout.qml` switches the center content slot between the note editor, the dedicated resource editor, and
the calendar surfaces.
It owns the platform mode for the unified note host and forwards shared collaborators into the currently active center
surface.

## Current Contract

- Note editor surface: `ContentsDisplayView.qml`
- Resource editor surface: `ContentsResourceEditorView.qml`
- Mobile/desktop host variance: `mobileHost` input forwarded from `isMobilePlatform`
- Calendar surfaces:
  - `AgendaPage.qml`
  - `DayCalendarPage.qml`
  - `WeekCalendarPage.qml`
- `MonthCalendarPage.qml`
- `YearCalendarPage.qml`
- The active non-calendar surface now always fills the full content slot and no longer forwards any bottom-partition
  sizing contract.
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
- Inside the non-calendar slot, a second loader now selects:
  - `ContentsDisplayView.qml` for note-backed list models
  - `ContentsResourceEditorView.qml` for direct resource-backed list models
- The active note-list model still closes any visible calendar surface when note selection changes.
- Entering resource-editor mode also dismisses any active calendar overlay so the center slot cannot show both a
  resource editor request and a calendar overlay at once.
- `resourcesImportController`, `editorViewModeController`, `sidebarHierarchyController`, and the resolved
  note-list/content controllers are forwarded into the unified note editor host when that surface is active.
- The global `noteActiveState` object is also forwarded into the note editor host. The host registers its
  `ContentsEditorSessionController` there, so active-note changes can rebind the editor session before QML's local
  note-list bindings finish a later refresh turn.
- `minimapVisible` is forwarded into `ContentsDisplayView.qml`, where the visible editor/minimap `LV.HStack` is
  mounted.
- That sidebar-hierarchy forwarding is now part of the editor contract so desktop/mobile surfaces can distinguish:
  - direct resource-package browsing inside the Resources hierarchy
  - ordinary notes from other hierarchies whose `.wsnbody` happens to contain inline `<resource ... />` blocks
- `isMobilePlatform` still decides which host mode is activated.
- `requestOpenLibraryNote(noteId)` now uses `libraryHierarchyController.activateNoteById(...)` together with
  `sidebarHierarchyController.setActiveHierarchyIndex(...)` so a calendar note tap can switch the active hierarchy back
  to Library, select the note, and then dismiss the current calendar overlay.
- `YearCalendarPage.qml` can now request a month-overlay open through this file. `ContentViewLayout.qml` applies the
  requested year/month/date to `monthCalendarController` first, then emits `monthCalendarOverlayOpenRequested`.

## Tests

## Tests

- Regression coverage now lives in `test/cpp/suites/*.cpp`.
- Regression checklist:
  - switching between note editor, resource editor, and calendar surfaces must continue to use one shared content slot
  - the active non-calendar surface must fill that slot in both desktop and mobile modes without a reserved bottom partition
  - direct resource-backed list models must choose `ContentsResourceEditorView.qml` without affecting note-backed hierarchies
  - note selection changes must still dismiss any visible calendar surface
  - a year-calendar month/day tap must still propagate into a month-overlay open request with the month controller already synchronized to the requested month/date
  - a note tap from Agenda/day/week/month must reopen that library note and return the content slot to the note editor surface
  - calendar note opening must switch the active hierarchy back to Library before the overlay is dismissed
  - the note editor host must receive the same global `noteActiveState` object that desktop/mobile shells use for
    active-note selection
