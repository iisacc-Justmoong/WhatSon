# `src/app/calendar/CalendarBoardStore.cpp`

## Implementation Notes
- Constructor now initializes the `ICalendarBoardStore` base.
- Manual event/task entries still remain mutable through `addEvent(...)`, `addTask(...)`, `removeEntry(...)`, and
  `setTaskCompleted(...)`.
- The store now keeps a second, read-only projection for library notes indexed from the current `.wshub` package.
- Each note can contribute up to two calendar entries:
  - one for `createdAt`
  - one for `lastModifiedAt`
- When creation and modification fall on the same calendar day, the projection collapses those into one
  `Created/modified note` entry for that date instead of duplicating the chip.
- Projected note entries stay on the shared `entriesForDate(...)` / `countsForDate(...)` path, so day/week/month/year
  viewmodels and Agenda all receive note lifecycle items alongside manual calendar events.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
  - a note whose `lastModifiedAt` is `2026-04-08-...` must appear in `entriesForDate("2026-04-08")`
  - manual event/task entries must remain queryable after note projection reloads
  - deleting or completing a manual entry must not mutate read-only projected note entries
  - a note with identical creation/modification dates must contribute only one projected entry for that day
