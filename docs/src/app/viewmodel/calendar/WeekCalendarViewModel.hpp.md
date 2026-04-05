# `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp`

## Role
`WeekCalendarViewModel` exposes a week projection grouped by day.

## Interface Alignment
- Calendar board injection now depends on `ICalendarBoardStore`.
- `displayedWeekStartIso` remains the canonical week anchor even though the QML surface now renders three visible day
  columns from one continuous horizontal scaffold built from per-date entry queries.
- The weekly anchor is now synchronized from the focused middle date in the viewport, not from the left edge of the
  three-column surface.
- The current-week visual emphasis in QML is computed against a Monday-start week, matching the same weekly anchoring
  expectation used by the weekly route.
