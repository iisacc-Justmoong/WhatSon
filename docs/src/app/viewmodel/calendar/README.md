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
- `WeekCalendarViewModel` continues to anchor the weekly route while the QML week surface now presents that data as
  one continuous horizontal date surface sized for three visible day columns.
- `AgendaViewModel` projects one date into Agenda-focused section models (`allDayEvents`, `timedEvents`, `agendaItems`) and
  exposes the date label, location caption, and summary counts needed by the Agenda route layout.
