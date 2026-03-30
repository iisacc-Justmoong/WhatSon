# `src/app/viewmodel/calendar/YearCalendarViewModel.cpp`

## Role
Implements year-level calendar model generation, calendar-system switching, and board-store synchronization.

## Behavior Summary
- Initializes to current year in Gregorian mode.
- Rebuilds the full year grid when:
  - year changes,
  - calendar system changes,
  - shared board entries change.
- Emits `yearViewRequested(reason)` for navigation/view hooks.

## Year Grid Generation
`rebuildYearModel()` produces month cards with 42-cell day grids, including adjacent-month spill cells.
Each day model now carries entry counters (`eventCount`, `taskCount`, `entryCount`) resolved from
`CalendarBoardStore::countsForDate(dateIso)`.

## Board Integration
- `setCalendarBoardStore(...)` disconnects old store bindings and connects `entriesChanged()` to year-model rebuild.
- Event/task mutation wrappers delegate directly to the shared store so month and year overlays stay consistent.
- `entriesForDate(...)` provides per-date drill-down for future detail panels or popovers.

## Coverage
- `tests/app/test_year_calendar_viewmodel.cpp` verifies:
  - baseline year rendering behavior,
  - calendar-system and year mutation contracts,
  - board entry counters projected into the year day-grid cells.
