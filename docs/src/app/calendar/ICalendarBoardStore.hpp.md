# `src/app/calendar/ICalendarBoardStore.hpp`

## Role
`ICalendarBoardStore` defines the shared calendar board contract used by calendar-facing viewmodels.

## Contract
- Mutations: `addEvent`, `addTask`, `removeEntry`, `setTaskCompleted`
- Queries: `entriesForDate`, `countsForDate`
- Signals: `entriesChanged`, `entryAdded`, `entryRemoved`, `entryUpdated`

## Notes
- Viewmodels now depend on this interface instead of the concrete `CalendarBoardStore` implementation.
- The interface keeps object-to-object communication on the calendar path independent from the in-memory store class.
