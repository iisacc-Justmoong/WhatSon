# `src/app/models/calendar/DayCalendarController.cpp`

## Implementation Notes
- Day-model rebuilding listens to `ICalendarBoardStore::entriesChanged`.
- `requestDayView(...)` is now hook/log-only; actual rebuild ownership stays with `setDisplayedDateIso(...)`,
  `setCalendarBoardStore(...)`, and `entriesChanged`, which removes the duplicate recalculation path after
  `shiftDay(...)` / `jumpToToday()`.
