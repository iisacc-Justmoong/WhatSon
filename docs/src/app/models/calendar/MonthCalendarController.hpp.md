# `src/app/models/calendar/MonthCalendarController.hpp`

## Role
`MonthCalendarController` exposes a calendar-grid month projection.

## Interface Alignment
- Calendar board injection now depends on `ICalendarBoardStore`.
- `monthProjectionFor(year, month)` remains available for arbitrary month-grid queries.
- The controller now also exposes a precomputed `pagerMonthModels` property for the canonical `previous/current/next`
  month pages, so `MonthCalendarPage.qml` no longer has to assemble neighboring month projections inside QML.
