# `src/app/viewmodel/calendar/MonthCalendarViewModel.cpp`

## Implementation Notes
- Month and selected-date refreshes now observe `ICalendarBoardStore`.
- The shared month-grid builder now serves both the canonical `displayedYear/displayedMonth` state and ad-hoc neighbor
  projections requested by the mobile month pager.
