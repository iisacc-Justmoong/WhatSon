# `src/app/models/calendar/AgendaController.hpp`

## Role
`AgendaController` exposes the agenda projection for a selected date.

## Interface Alignment
- Calendar board wiring now targets `ICalendarBoardStore`.
- The controller no longer requires the concrete in-memory board type.
- Agenda projection now exposes only date label, location, section lists, and summary state.
