# `src/app/viewmodel/calendar/WeekCalendarViewModel.cpp`

## Implementation Notes
- Week refresh now observes `ICalendarBoardStore::entriesChanged`.
- The viewmodel still owns the normalized week anchor; `WeekCalendarPage.qml` now composes a continuous horizontal
  date surface by repeatedly calling `entriesForDate(dateIso)` around that anchor.
- The QML layer now synchronizes `displayedWeekStartIso` from the focused middle date in the three-column viewport, so
  `Today` can land on the actual current date while preserving a week-level anchor.
