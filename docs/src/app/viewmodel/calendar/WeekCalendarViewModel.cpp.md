# `src/app/viewmodel/calendar/WeekCalendarViewModel.cpp`

## Implementation Notes
- Week refresh now observes `ICalendarBoardStore::entriesChanged`.
- The viewmodel still owns the normalized week anchor.
- The viewmodel now also owns the lazy horizontal timeline window (`timelineDayModels`) and can initialize, prepend,
  append, trim, and refresh that window without a second QML-side `ListModel`.
- Timeline day models now already carry `entries`, `eventCount`, `taskCount`, `weekdayLabel`, and `dateLabel`, so the
  week page does not rebuild date metadata or cache entry lists on its own.
- The QML layer now synchronizes `displayedWeekStartIso` from the focused middle date in the three-column viewport, so
  `Today` can land on the actual current date while preserving a week-level anchor.
- `requestWeekView(...)` is now hook/log-only; the real week rebuild stays behind `setDisplayedWeekStartIso(...)`,
  `setCalendarBoardStore(...)`, and board-entry mutations so page-open and prev/next navigation do not rebuild twice.
- `trimTimelineWindow(...)` normalizes `QVariantList::size()` into explicit `int` window counts before calling
  `std::min(...)`, which keeps the Qt 6 `qsizetype` container API from breaking the timeline trim compile path.

## Regression Checks
- `trimTimelineWindow(...)` should keep compiling when `QVariantList::size()` resolves to `qsizetype` on Qt 6.
