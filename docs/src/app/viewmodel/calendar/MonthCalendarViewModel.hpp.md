# `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp`

## Role
`MonthCalendarViewModel` exposes a calendar-grid month projection.

## Interface Alignment
- Calendar board injection now depends on `ICalendarBoardStore`.
- `monthProjectionFor(year, month)` exposes an arbitrary month-grid projection so QML can render neighboring month pages
  without mutating the canonical displayed month first.
