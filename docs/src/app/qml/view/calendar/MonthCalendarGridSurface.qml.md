# `src/app/qml/view/calendar/MonthCalendarGridSurface.qml`

## Role

`MonthCalendarGridSurface.qml` is the reusable month-grid renderer used by `MonthCalendarPage.qml`.

## Responsibilities

- Render the weekday header band and the 7-column day grid for one month projection.
- Trim trailing all-adjacent-month rows from a 42-cell projection before computing visible row count.
- Resolve per-day entry chips through `calendarVm.entriesForDate(...)`.
- Forward `dayModel.isToday` into each `MonthCalendarDayCell` so the current date can render its soft outline state.
- Emit `dateSelected(dateIso)` when a day cell is tapped.

## Public Contract

- Inputs:
    - `calendarVm`
    - `monthProjection`
    - `selectedDateIso`
    - sizing/style props such as `weekdayHeaderHeight`, `weekdayCellHorizontalPadding`, `bodyLabelPixelSize`
- Signals:
    - `dateSelected(string dateIso)`
    - `viewHookRequested(string reason)`
- Hook forwarder:
    - `requestViewHook(reason)`

## Notes

- This component intentionally owns only month-grid rendering and day-cell interaction.
- Month-level navigation and horizontal paging stay in `MonthCalendarPage.qml`.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - A 42-cell projection must collapse trailing out-of-month rows down to a 5-row grid when the sixth row is empty of
      current-month dates.
    - Day taps must continue to resolve entry chips and emit the selected ISO date.
    - The visible cell for today's ISO date must keep its soft border when month projections are rebuilt.
