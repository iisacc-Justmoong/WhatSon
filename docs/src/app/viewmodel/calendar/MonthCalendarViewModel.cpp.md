# `src/app/viewmodel/calendar/MonthCalendarViewModel.cpp`

## Implementation Notes
- Month and selected-date refreshes now observe `ICalendarBoardStore`.
- The shared month-grid builder now serves both the canonical `displayedYear/displayedMonth` state and ad-hoc neighbor
  projections requested by the mobile month pager.
- Each month `dayModel` now carries the resolved `entries` array for that ISO date in addition to the aggregate counts,
  so the month grid can render note/event chips directly from the rebuilt projection instead of re-querying only by
  side effect.
