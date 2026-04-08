# `src/app/qml/view/calendar/MonthCalendarGridSurface.qml`

## Role

`MonthCalendarGridSurface.qml` is the reusable month-grid renderer used by `MonthCalendarPage.qml`.

## Responsibilities

- Render the weekday header band and the 7-column day grid for one month projection.
- Trim trailing all-adjacent-month rows from a 42-cell projection before computing visible row count.
- Resolve per-day entry chips through `calendarVm.entriesForDate(...)`.
- Keep manual calendar events and projected note lifecycle chips on the same day-cell surface.
- Forward `selectedDateIso` into each `MonthCalendarDayCell` so the chosen date can render its accent-border state.
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
- Weekday header height, left padding, and label size now resolve through `LV.Theme.scaleMetric(...)` / `LV.Theme.gap12`
  so the month grid follows LVRS density scaling instead of fixed `12/39px` values.
- Projected note chips now use the calendar accent color while manual event chips keep the primary event styling.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - A 42-cell projection must collapse trailing out-of-month rows down to a 5-row grid when the sixth row is empty of
      current-month dates.
    - Day taps must continue to resolve entry chips and emit the selected ISO date.
    - `buildEntryCellModels(...)` must return visible chip payloads for both manual events and projected notes.
    - Projected note entries marked `allDay` must not append `00:00` to the chip label.
    - The visible cell for `selectedDateIso` must render the selected accent border after selection changes.
    - The visible cell for today's ISO date must keep its soft border when month projections are rebuilt.
