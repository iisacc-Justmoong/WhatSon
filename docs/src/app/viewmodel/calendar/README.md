# `src/app/viewmodel/calendar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/calendar`
- Child directories: 0
- Child files: 10

## Child Directories
- No child directories.

## Child Files
- `DayCalendarViewModel.cpp`
- `DayCalendarViewModel.hpp`
- `MonthCalendarViewModel.cpp`
- `MonthCalendarViewModel.hpp`
- `AgendaViewModel.cpp`
- `AgendaViewModel.hpp`
- `WeekCalendarViewModel.cpp`
- `WeekCalendarViewModel.hpp`
- `YearCalendarViewModel.cpp`
- `YearCalendarViewModel.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- `DayCalendarViewModel`, `WeekCalendarViewModel`, `MonthCalendarViewModel`, and `YearCalendarViewModel` are wired to
  the shared `src/app/calendar/CalendarBoardStore.*` backend for date/time-based event/task board semantics.
- That shared board now merges manual calendar entries with note lifecycle projections keyed by note creation and
  modification timestamps, so all calendar routes see the same note/event surface.
- `MonthCalendarViewModel` now owns the `previous/current/next` month pager projections directly instead of asking QML
  to assemble them ad hoc at render time, which keeps month bootstrap aligned with MVVM ownership and reduces
  page-open churn.
- Calendar note projection refresh is now driven by the library runtime viewmodel's note-snapshot signal instead of a
  month-page open side effect, so note mounting stays deterministic even during deferred startup loads.
- `WeekCalendarViewModel` now owns both the canonical week anchor and the lazy horizontal timeline day window, while
  the QML week surface is reduced to viewport math and hit interaction for that data.
- `WeekCalendarViewModel::trimTimelineWindow(...)` explicitly normalizes Qt container sizes into integer window counts
  before chunk trimming so timeline maintenance stays compatible with Qt 6 `qsizetype` APIs.
- `AgendaViewModel` projects one date into Agenda-focused section models (`allDayEvents`, `timedEvents`, `agendaItems`) and
  exposes the date label, location caption, and summary counts needed by the Agenda route layout.
- `requestDayView(...)`, `requestAgendaView(...)`, `requestWeekView(...)`, and `requestYearView(...)` are now
  hook/log-only entry points. Rebuild ownership stays with actual cursor/system/store state changes so QML navigation
  actions do not trigger the same calendar recomputation twice.
