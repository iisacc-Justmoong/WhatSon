# `src/app/calendar/CalendarBoardStore.hpp`

## Role
`CalendarBoardStore` is the shared calendar board backend for date/time-bound events and tasks. It is designed as the
single in-memory source consumed by day, week, month, and year calendar viewmodels.

## Public API
- `addEvent(dateIso, timeText, title, detail)`
- `addTask(dateIso, timeText, title, detail)`
- `entriesForDate(dateIso)`
- `countsForDate(dateIso)`
- `removeEntry(entryId)`
- `setTaskCompleted(entryId, completed)`

All write APIs require explicit date and time arguments and are intended for future reminder/event assignment flows.

## Entry Payload Contract
`entriesForDate(...)` returns maps with:
- `id`
- `type` (`event` or `task`)
- `date` (ISO date)
- `time` (`HH:mm`)
- `title`
- `detail`
- `completed` (task completion flag)

`countsForDate(...)` returns:
- `eventCount`
- `taskCount`
- `entryCount`

## Signals
- `entriesChanged()`
- `entryAdded(entryId, entryType, dateIso, timeText)`
- `entryRemoved(entryId)`
- `entryUpdated(entryId)`

These signals are consumed by calendar viewmodels to rebuild visible day/week/month/year timeline metadata.
