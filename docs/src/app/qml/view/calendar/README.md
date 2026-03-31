# `src/app/qml/view/calendar`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/calendar`
- Child directories: 0
- Child files: 7

## Child Directories
- No child directories.

## Child Files
- `DayCalendarPage.qml`
- `CalendarTodayControl.qml`
- `CalendarEventCell.qml`
- `MonthCalendarDayCell.qml`
- `MonthCalendarPage.qml`
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
- Shared Figma-aligned calendar navigation control (`Prev/Today/Next`) is centralized in `CalendarTodayControl.qml`.
- Day/week pages keep only `CalendarTodayControl` in the top band and distribute 24 hourly slots across the remaining content height.
- Monthly page mirrors Figma node `228:9666` with fixed header (`54`), weekday band (`39`), and a non-scrollable fill grid.
