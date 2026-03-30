# `src/app/calendar/CalendarBoardStore.cpp`

## Role
Implements validation, normalization, mutation, and query behavior for the shared calendar board store.

## Validation Rules
- `dateIso` must be parseable by `QDate::fromString(..., Qt::ISODate)`.
- `timeText` must match supported time formats (`HH:mm`, `HH:mm:ss`, 12-hour variants, or ISO time).
- `title` must be non-empty after trimming.

Invalid payloads are rejected and traced through `WhatSon::Debug`.

## Data Behavior
- New entries receive UUID-based `id`.
- Entries are sorted by date/time/type/title for stable list rendering.
- Day-level counters are derived on demand by `countsForDate(...)`.
- Task completion is mutable through `setTaskCompleted(...)`; events remain immutable in completion semantics.

## Integration Points
- `DayCalendarViewModel`, `WeekCalendarViewModel`, `MonthCalendarViewModel`, and `YearCalendarViewModel` connect
  `entriesChanged()` to rebuild timeline/day-cell models.
- `main.cpp` instantiates one shared store and injects it into all four calendar viewmodels.

## Coverage
- `tests/app/test_calendar_board_store.cpp` verifies:
  - date/time/title validation,
  - sorted retrieval and per-day counts,
  - task completion and entry removal mutations.
