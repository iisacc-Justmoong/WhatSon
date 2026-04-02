# `src/app/viewmodel/calendar/AgendaViewModel.hpp`

## Role
`AgendaViewModel` exposes the agenda projection for a selected date.

## Interface Alignment
- Calendar board wiring now targets `ICalendarBoardStore`.
- The viewmodel no longer requires the concrete in-memory board type.
