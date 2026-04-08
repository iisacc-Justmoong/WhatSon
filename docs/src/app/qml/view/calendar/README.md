# `src/app/qml/view/calendar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/calendar`
- Child directories: 0
- Child files: 9

## Child Directories
- No child directories.

## Child Files
- `DayCalendarPage.qml`
- `CalendarTodayControl.qml`
- `CalendarEventCell.qml`
- `MonthCalendarDayCell.qml`
- `MonthCalendarGridSurface.qml`
- `MonthCalendarPage.qml`
- `AgendaPage.qml`
- `WeekCalendarPage.qml`
- `YearCalendarPage.qml`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- Day/week/month/year pages now consume the shared calendar backend through dedicated calendar viewmodels.
- `AgendaPage.qml` consumes `AgendaViewModel` and renders date header, location summary, all-day events,
  timed events, and agenda-item completion rows inside the content-surface calendar route.
- `AgendaPage.qml` no longer includes any weather card; the header is limited to date navigation and location context.
- Shared Figma-aligned calendar navigation control (`Prev/Today/Next`) is centralized in `CalendarTodayControl.qml`;
  it now follows node `238:7843` as a three-icon button group.
- Day/week pages keep only `CalendarTodayControl` in the top band and distribute 24 hourly slots across the remaining content height.
- Week view now uses one continuous scaffold: the left time rail stays fixed, the right date columns flick horizontally
  without snap, the viewport is sized for three visible day columns, generic header/hour cells stay transparent
  instead of drawing per-column grid fills, and the day-column model now comes directly from
  `WeekCalendarViewModel.timelineDayModels` instead of a QML-owned `ListModel` plus entry cache. Current-week
  initialization and `Today` both recenter the surface so the real current date occupies the middle visible column.
- Monthly page mirrors Figma node `228:9666` with fixed header (`54`) and a reusable month-grid surface; mobile adds a
  snap-paged horizontal month swiper while desktop keeps non-interactive paging.
- Year view now acts as a routing surface into month view: month-title/day taps preconfigure the month overlay before
  the host switches overlays.
- Year view adjacent-month overflow days now dim by deriving a color from `LV.Theme.titleHeaderColor` instead of
  relying on the label fallback color.
- Month view now distinguishes the selected date with an accent border while preserving the softer today-only border for
  non-selected current-day cells.
- Shared calendar chrome now routes visible chip/header/control metrics through LVRS `gap`, `radius`, `stroke`, and
  `scaleMetric(...)` helpers instead of page-local pixel literals.
- Projected note chips are now interactive across Agenda/day/week/month surfaces. When a chip resolves to a concrete
  library note id, the host can reopen that note in the editor and dismiss the active calendar overlay.
- Week view keeps one intentional limitation: the compressed `title +N` slot-summary chip cannot open a note directly
  because the current weekly slot UI still represents multiple entries with one shared hit target.
- Month view now resolves pager pages and day cells by numeric index against the latest projection arrays, so the
  initial calendar bootstrap does not leave current-month notes attached to adjacent disabled cells from an older
  snapshot.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - Agenda/day/month note chips must emit a note-open request on both mouse click and touch tap.
    - The host content surface must be able to close the active calendar overlay and reveal the editor after note
      activation.
    - Week view must only allow direct note open when one visible chip maps to exactly one note entry.
    - Initial month-view bootstrap must not render a note for `2026-04-01` or another current-month date inside a
      future/past disabled adjacent-month cell.
