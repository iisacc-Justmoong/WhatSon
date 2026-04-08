# `src/app/viewmodel/calendar/YearCalendarViewModel.cpp`

## Implementation Notes
- Year rebuild now observes `ICalendarBoardStore::entriesChanged`.
- `requestYearView(...)` is now hook/log-only; actual year-model rebuilding stays with `setDisplayedYear(...)`,
  `setCalendarSystemByEnum(...)`, `setCalendarBoardStore(...)`, and board-entry mutations.
