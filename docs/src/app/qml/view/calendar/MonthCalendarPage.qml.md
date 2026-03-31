# `src/app/qml/view/calendar/MonthCalendarPage.qml`

## Role
`MonthCalendarPage.qml` renders a month-grid overlay surface for the editor area. It consumes
`monthCalendarViewModel`, requests month navigation hooks, and renders a conventional 7-column month calendar
with weekday headers plus per-day schedule previews.
The page is fully non-scrollable and fills the `ContentsView` slot responsively.
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
  - shared `CalendarTodayControl` (`Prev/Today/Next`) aligned to Figma node `227:8807`.
  - Active month title (`monthLabel, displayedYear`).
- Body row:
  - Weekday header grid (`weekdayLabels`) with fixed height (`weekdayHeaderHeight`).
    - Header labels are center-aligned per column for even width distribution.
  - Fixed day grid (`visibleDayModels`) with:
    - explicit cell sizing:
      - `dayCellWidth = floor(monthDayGrid.width / 7)`
      - `dayCellHeight = floor(monthDayGrid.height / visibleWeekRowCount)`
    - each day delegate binds `Layout.preferredWidth/Height` to those computed values so the
      date grid always fills the remaining area without row collapse,
    - border emphasis for `isToday` and selected date,
    - dimmed text opacity for out-of-month cells (`inCurrentMonth === false`),
    - right-aligned day number label (`dayNumberPixelSize = 16`, `dayNumberHeight = LV.Theme.gap16`),
    - per-day entry list preview (`entriesForDate(dateIso)`),
    - overflow marker (`+N more`) when cell entries exceed in-cell capacity.
  - Root, weekday cells, and day cells do not paint background fills (`color: "transparent"`).
  - No `Flickable`/`ListView` usage in the page; both axes are constrained to the available
    `ContentsView` frame.
  - Day grid is anchored from `weekdayHeaderGrid.bottom` to `parent.bottom`, so the date area always fills the
    remaining calendar body.

## Interaction and Data Flow
1. On `Component.onCompleted`, page emits `requestViewHook("page-open")`.
2. Header control actions call `shiftMonth(-1|1)` and `focusToday()` (`current-month` hook).
3. Day click sets `calendarVm.selectedDateIso` and requests hook (`select-date`).
4. Each day cell computes `entryCapacity` from actual rendered cell height and clips entry rows accordingly.
5. When rows exceed capacity, `+N more` is shown instead of adding scroll surfaces.
6. Recomputed labels/day models are reflected through QML bindings without local caching.

## Collaborators
- `src/app/calendar/CalendarBoardStore.hpp/.cpp`
- `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml` (overlay host via `CalendarView.MonthCalendarPage`)

## Testing
- `tests/app/test_month_calendar_viewmodel.cpp` verifies core month-model behavior.
- `tests/app/test_navigation_qml_frames.cpp` and
  `tests/python/test_navigation_panel_toggles.py` verify navigation-to-overlay routing.
