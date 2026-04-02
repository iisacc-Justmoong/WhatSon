# `src/app/calendar/CalendarBoardStore.hpp`

## Role
`CalendarBoardStore` is the default in-memory implementation of `ICalendarBoardStore`.

## Interface Alignment
- Inherits `ICalendarBoardStore`.
- Keeps the same mutation/query surface while moving collaboration to the interface type.
- Emits the shared board signals declared on the interface.
