# `src/app/calendar/CalendarBoardStore.cpp`

## Implementation Notes
- Constructor now initializes the `ICalendarBoardStore` base.
- Storage, sorting, and event/task mutation behavior remain unchanged.
- The change is structural: consumers bind to the interface while this file stays the concrete backing store.
