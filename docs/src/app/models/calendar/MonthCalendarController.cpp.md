# `src/app/models/calendar/MonthCalendarController.cpp`

## Implementation Notes
- Month and selected-date refreshes now observe `ICalendarBoardStore`.
- The shared month-grid builder now serves both the canonical `displayedYear/displayedMonth` state and the persistent
  `pagerMonthModels` property used by the month pager.
- Each month `dayModel` now carries the resolved `entries` array for that ISO date in addition to the aggregate counts,
  so the month grid can render note/event chips directly from the rebuilt projection instead of re-querying only by
  side effect.
- `requestMonthView(...)` now emits the hook/tracing signal without forcing another month rebuild. The month data is
  already owned by setter/store-driven state changes, so opening the month surface no longer incurs an extra
  page-open recomputation from QML.
- The month view therefore now depends on the upstream store/controller signal flow being correct: projected note cache
  updates must arrive through `CalendarBoardStore::entriesChanged`, not through a late UI-triggered rebuild.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - The canonical current-month model and `pagerMonthModels[1]` must describe the same month/year.
    - Opening month view must not require `requestMonthView(...)` to rebuild the projection before the correct month
      grid appears.
    - Month note chips must still appear after startup/deferred library loads even when the user never presses `Today`.
    - Neighbor month pager data must stay synchronized after `shiftMonth(...)` and `focusToday()`.
