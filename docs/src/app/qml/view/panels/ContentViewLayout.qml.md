# `src/app/qml/view/panels/ContentViewLayout.qml`

## Responsibility

`ContentViewLayout.qml` switches the center content slot between the note editor, the dedicated resource editor, and
the calendar surfaces.
It owns the platform mode for the unified note host and forwards shared collaborators into the currently active center
surface.

## Current Contract

- Note editor surface: direct `ContentViewLayout.qml` chrome plus `ContentsEditorDisplayBackend`
- Resource editor surface: `ContentsResourceEditorView.qml`
- Mobile/desktop route context: `isMobilePlatform` is retained at this boundary, but the note editor no longer mounts a
  separate QML host-mode wrapper.
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
  - direct note-editor chrome for note-backed list models
  - `ContentsResourceEditorView.qml` for direct resource-backed list models
- `ContentsEditorSurfaceModeSupport` owns that note-vs-resource surface policy in C++; this QML file only binds to
  `resourceEditorVisible` and `currentResourceEntry`.
- The active note-list model still closes any visible calendar surface when note selection changes.
- Entering resource-editor mode also dismisses any active calendar overlay so the center slot cannot show both a
  resource editor request and a calendar overlay at once.
- The resolved note-list/content controllers and active-note tracker are forwarded into `ContentsEditorDisplayBackend`
  when the note editor surface is active.
- The global `noteActiveState` object is also forwarded into the note editor host. The host registers its
  `ContentsEditorSessionController` there, so active-note changes can rebind the editor session before QML's local
  note-list bindings finish a later refresh turn.
- `minimapVisible` is forwarded into `ContentsEditorDisplayBackend`; this QML file renders the visible editor/minimap
  `LV.HStack` and applies minimap drag deltas to the editor `Flickable`.
- The editor document viewport keeps a `LV.Theme.gap16` bottom inset in the scrollable content height. This is document
  body breathing room, not text top padding; the first line remains top-flush while the final line can scroll above the
  bottom chrome.
- The editor `Flickable` resets to the top only when `ContentsEditorDisplayBackend` emits
  `editorViewportResetRequested()`. That signal is a note-identity transition contract, not a generic text/projection
  refresh hook; same-note typing, save reconcile, and current-entry refreshes must preserve `contentY`.
- The note editor flow receives both the session RAW `sourceText` and the projection-owned `projectionSourceText`, so
  `ContentsStructuredDocumentFlow.qml` can hold the last ready logical projection while parser/render publication
  catches up instead of falling back to RAW tag text.
- `ContentViewLayout.qml` does not forward `presentationProjection.logicalCursorPosition` into
  `ContentsStructuredDocumentFlow.qml`; the live `ContentsInlineFormatEditor.qml` native editor owns the visible
  logical cursor and exposes the mapped RAW cursor back upward through `editorCursorPosition`.
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
