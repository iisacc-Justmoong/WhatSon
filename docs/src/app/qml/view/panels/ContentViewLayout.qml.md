# `src/app/qml/view/panels/ContentViewLayout.qml`

## Responsibility

`ContentViewLayout.qml` switches the center content slot between the editor surface and the calendar surfaces.
It owns the platform split between desktop and mobile editor implementations and forwards shared editor collaborators
into the selected editor surface.

## Current Contract

- Desktop editor surface: `ContentsDisplayView.qml`
- Mobile editor surface: `MobileContentsDisplayView.qml`
- Calendar surfaces:
  - `AgendaPage.qml`
  - `DayCalendarPage.qml`
  - `WeekCalendarPage.qml`
- `MonthCalendarPage.qml`
- `YearCalendarPage.qml`
- The editor surface now always fills the full content slot and no longer forwards any bottom-partition sizing
  contract.

## Signals

- `editorTextEdited(string text)`
- `dayCalendarOverlayCloseRequested`
- `agendaOverlayCloseRequested`
- `monthCalendarOverlayCloseRequested`
- `weekCalendarOverlayCloseRequested`
- `viewHookRequested`
- `yearCalendarOverlayCloseRequested`

## Routing Notes

- A `StackLayout` selects either the editor surface or the active calendar surface.
- The active note-list model still closes any visible calendar surface when note selection changes.
- `resourcesImportViewModel`, `editorViewModeViewModel`, and the resolved note-list/content viewmodels are forwarded
  into both editor variants.
- `isMobilePlatform` still decides which editor file is instantiated.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Switching between editor and calendar surfaces must continue to use one shared content slot.
  - Desktop and mobile editor surfaces must both fill that slot without a reserved bottom partition.
  - Note selection changes must still dismiss any visible calendar surface.
