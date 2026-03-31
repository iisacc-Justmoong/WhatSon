# `src/app/qml/view/calendar/MonthCalendarPage.qml`

## Role
`MonthCalendarPage.qml` renders a month-grid overlay surface for the editor area. It consumes
`monthCalendarViewModel`, requests month navigation hooks, and renders a conventional 7-column month calendar
with weekday headers plus per-day schedule previews.
The page is fully non-scrollable and fills the `ContentsView` slot responsively.
Layout and sizing now align to Figma node `228:9666` at cell-level granularity.
All calendar surfaces keep transparent fills so the app background color is visible through the month page.

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
  - fixed height `54` (`monthHeaderHeight`)
  - left: active month title (`monthLabel, displayedYear`)
  - right: shared `CalendarTodayControl` (`Prev/Today/Next`) aligned to Figma node `227:8807`.
- Body row:
  - Weekday header band (`weekdayHeaderBand`) with fixed height `39` (`weekdayHeaderHeight`).
    - Weekday labels use body typography (`12 / Medium`) with left inset (`weekdayCellHorizontalPadding = 12`) and 3-letter uppercase.
    - Header background/border uses `LV.Theme.panelBackground06`.
  - Fixed day grid (`visibleDayModels`) with:
    - explicit cell sizing:
      - `dayCellWidth = floor(monthDayGrid.width / 7)`
      - `dayCellHeight = floor(monthDayGrid.height / visibleWeekRowCount)`
    - each day delegate binds `Layout.preferredWidth/Height` to those computed values so the
      date grid always fills the remaining area without row collapse,
    - delegates composed by reusable `MonthCalendarDayCell`:
      - `disable: false` for in-month cells,
      - `disable: true` for adjacent-month spillover cells,
    - per-day entry models are transformed into reusable shared `CalendarEventCell` chips
      (`label` + `backgroundType` with two modes),
    - overflow marker (`+N more`) when cell entries exceed in-cell capacity (inside day-cell component).
  - Root container remains transparent (`color: "transparent"`), while day cells keep their own dark tile fill.
  - No `Flickable`/`ListView` usage in the page; both axes are constrained to the available
    `ContentsView` frame.
  - Day grid is anchored from `weekdayHeaderBand.bottom` to `parent.bottom`, so the date area always fills the
    remaining calendar body.

## Interaction and Data Flow
1. On `Component.onCompleted`, page emits `requestViewHook("page-open")`.
2. Header control actions call `shiftMonth(-1|1)` and `focusToday()` (`current-month` hook).
3. Day click sets `calendarVm.selectedDateIso` and requests hook (`select-date`).
4. Day entry maps are converted via `buildEntryCellModels(...)` and delegated into reusable day/event cell modules.
5. Recomputed labels/day models are reflected through QML bindings without local caching.

## Collaborators
- `src/app/calendar/CalendarBoardStore.hpp/.cpp`
- `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/MonthCalendarDayCell.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml` (overlay host via `CalendarView.MonthCalendarPage`)

## Testing
- `tests/app/test_month_calendar_viewmodel.cpp` verifies core month-model behavior.
- `tests/app/test_navigation_qml_frames.cpp` and
  `tests/python/test_navigation_panel_toggles.py` verify navigation-to-overlay routing.
