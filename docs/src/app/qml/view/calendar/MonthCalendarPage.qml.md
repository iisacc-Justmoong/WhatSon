# `src/app/qml/view/calendar/MonthCalendarPage.qml`

## Role
`MonthCalendarPage.qml` renders a month-grid overlay surface for the editor area. It consumes
`monthCalendarViewModel`, requests month navigation hooks, and renders a fixed 7x6 day grid with weekday labels.

## Source Metadata
- Source path: `src/app/qml/view/calendar/MonthCalendarPage.qml`
- Source kind: QML view/component
- Root type: `Rectangle`
- Approximate line count: 170+

## Public QML Contract
- `property var monthCalendarViewModel`: backend calendar source exposed from `main.cpp`.
- `signal viewHookRequested(string reason)`: emitted for navigation/telemetry hooks.
- `function requestViewHook(reason)`: forwards the same reason into `monthCalendarViewModel.requestMonthView(...)`
  and then emits `viewHookRequested`.

## Render Structure
- Header row:
  - Previous/next month controls (`shiftMonth(-1)` / `shiftMonth(1)`).
  - Active month/year/system label.
  - Calendar-system selector buttons bound to `calendarSystemOptions`.
- Body row:
  - Weekday label row (`weekdayLabels`).
  - Day cell grid (`dayModels`) with current-month/today visual states.
  - Day cell entry-count badge sourced from `dayModel.entryCount`.

## Interaction and Data Flow
1. On `Component.onCompleted`, page emits `requestViewHook("page-open")`.
2. Prev/next buttons mutate the viewmodel month, then request hook (`previous-month`, `next-month`).
3. System buttons call `setCalendarSystemByValue(...)`, then request hook (`calendar-system`).
4. Day click sets `calendarVm.selectedDateIso` and requests hook (`select-date`).
5. Recomputed labels/day models are reflected through QML bindings without local caching.

## Collaborators
- `src/app/calendar/CalendarBoardStore.hpp/.cpp`
- `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/panels/ContentViewLayout.qml` (overlay host via `CalendarView.MonthCalendarPage`)

## Testing
- `tests/app/test_month_calendar_viewmodel.cpp` verifies core month-model behavior.
- `tests/app/test_navigation_qml_frames.cpp` and
  `tests/python/test_navigation_panel_toggles.py` verify navigation-to-overlay routing.
