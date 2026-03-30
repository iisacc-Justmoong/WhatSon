# `src/app/viewmodel/calendar/MonthCalendarViewModel.cpp`

## Role
Implements month-grid state derivation, calendar-system resolution, cursor mutation, and request tracing for
`MonthCalendarViewModel`.

## Behavior Summary
- Initializes to current system date month/year in Gregorian mode.
- Rebuilds month payload whenever year/month/system changes.
- Supports calendar-system switching by enum or integer value.
- Supports cross-year month shifting (`shiftMonth`) with bounded year range.
- Emits explicit view-request signal (`monthViewRequested`) with normalized reason text.
- Syncs with `CalendarBoardStore` updates and keeps selected-date entries current.

## Month Grid Generation
`rebuildMonthModel()` computes:
- Locale-aware weekday headers ordered from the locale first day of week.
- Current month day count and leading/trailing days from adjacent months.
- A fixed 42-cell grid payload (`dayModels`) containing:
  - `day`, `month`, `year`
  - `dateIso`
  - `inCurrentMonth`
  - `isToday`
  - `eventCount`, `taskCount`, `entryCount`

If the selected calendar cannot produce a valid first date, the model resets to empty labels/grid and emits
`monthViewChanged()`.

## Calendar-System Resolution
`resolveCalendarSystem()` maps enum selections to `QCalendar` instances:
- Gregorian uses built-in `QCalendar::System::Gregorian`.
- Julian / Islamic Civil / Custom try named calendar systems first, then fall back to Gregorian if invalid.

## Diagnostics
- Rejecting unsupported `setCalendarSystemByValue(...)` values logs a structured debug trace.
- `requestMonthView(...)` also emits structured trace entries containing year/month/system/reason.

## Board Flow
- `setCalendarBoardStore(...)` connects store `entriesChanged()` to:
  - `rebuildMonthModel()`
  - `refreshSelectedDateEntries()`
- Mutation wrappers (`addEvent`, `addTask`, `removeEntry`, `setTaskCompleted`) delegate to the shared board store.
- `setSelectedDateIso(...)` validates ISO date text and refreshes `selectedDateEntries`.

## Coverage
- `tests/app/test_month_calendar_viewmodel.cpp` validates defaults, calendar-system mutation, month cursor boundaries,
  request signal emission, and board entry count/selection synchronization.
