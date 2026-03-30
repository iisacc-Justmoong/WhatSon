# `src/app/viewmodel/calendar/DayCalendarViewModel.cpp`

## Role
Implements date cursor mutation, day-entry projection, and 24-hour timeline derivation for
`DayCalendarViewModel`.

## Behavior Summary
- Initializes to `QDate::currentDate()` in ISO format.
- Rebuilds day payload when:
  - displayed date changes,
  - board store pointer changes,
  - board emits `entriesChanged()`,
  - `requestDayView(...)` is invoked.
- Emits structured debug traces for view requests.

## Timeline Generation
`rebuildDayModel()` builds a fixed 24-hour slot list where each slot includes:
- `hour`
- `timeLabel` (`HH:mm`)
- `entries` (entries with matching hour)
- `entryCount`

This keeps QML simple by avoiding per-frame hour filtering in the view.

## Board Flow
- `setCalendarBoardStore(...)` reconnects signals safely and refreshes model state.
- Mutation wrappers (`addEvent`, `addTask`, `removeEntry`, `setTaskCompleted`) delegate to the shared board store.
- `entriesForDate(...)` returns date-filtered entries directly from the board.

## Coverage
- `tests/app/test_day_calendar_viewmodel.cpp` validates defaults, cursor shifting, slot population, and request
  signal emission.
