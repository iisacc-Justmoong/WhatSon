# `src/app/viewmodel/calendar/WeekCalendarViewModel.cpp`

## Role
Implements week cursor normalization, locale-based week labeling, and day-model projection for
`WeekCalendarViewModel`.

## Behavior Summary
- Initializes week cursor to locale `startOfWeek(currentDate)`.
- Accepts arbitrary date input in `setDisplayedWeekStartIso(...)`, then normalizes to week-start.
- Supports week paging via `shiftWeek(...)`.
- Emits explicit request signal (`weekViewRequested`) with normalized reason text.

## Week Model Generation
`rebuildWeekModel()` derives:
- locale-first weekday labels,
- seven day models from week start to +6 days,
- per-day board entries and counters (`eventCount`, `taskCount`, `entryCount`),
- `isToday` marker for quick UI highlighting.

## Board Flow
- `setCalendarBoardStore(...)` reconnects store signals and refreshes state immediately.
- Mutation wrappers (`addEvent`, `addTask`, `removeEntry`, `setTaskCompleted`) delegate to `CalendarBoardStore`.

## Coverage
  request signal emission.
