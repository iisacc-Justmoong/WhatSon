# `src/app/qml/view/calendar/YearCalendarPage.qml`

## Role
`YearCalendarPage.qml` renders the annual calendar content surface and now surfaces board-style per-day entry counts so
the year view acts as a high-level event/task heatmap.

## View Contract
- Input: `yearCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Navigation signal: `monthCalendarOpenRequested(int year, int month, string selectedDateIso)`
- Hook forwarder: `requestViewHook(reason)` delegates to `yearCalendarViewModel.requestYearView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) is always visible and drives year shift/focus actions.
  - calendar-system switching is no longer exposed in this page; that control belongs to settings UI.
- Body:
  - desktop: fixed 4x3 month-card grid (`desktopYearGridColumnCount = 4`),
  - mobile: vertical scroll year list (`mobileYearGridColumnCount = 1`),
  - month title color uses accent token (`monthTitleColor = LV.Theme.accent`) instead of fixed red,
  - desktop month cards scale responsively using `desktopResponsiveScale`, so grid gap/card padding/day-cell sizes
    change with window size (`yearGridSpacing`, `monthCardPadding`, `monthSectionSpacing`, `monthTitlePixelSize`),
  - weekday/day numeric labels are standardized to body typography (`monthWeekdayPixelSize = 12`,
    `monthDayPixelSize = 12`, both `Font.Medium`),
  - weekday header row per card,
  - 42-cell day grid per month.

## LVRS/QML Standard Alignment
- Enables `pragma ComponentBehavior: Bound` so nested delegates access outer IDs through explicit bound scope.
- Repeater delegates use `required property var modelData` and ID-qualified mapping
  (`monthCard.modelData`, `dayCell.modelData`) instead of unqualified context reads.
- This keeps the calendar page compatible with the stricter LVRS/QML lint contract used by the workspace.

## Board Extensions
- Each day cell uses lightweight text rendering with `inCurrentMonth` dimming and a circular today highlight.
- The year view remains a high-density navigation surface for month/day context while keeping the board data contract
  from `YearCalendarViewModel`.
- `YearCalendarViewModel::focusToday()` aligns the displayed year with the active calendar system.
- Month-title taps now request the matching month overlay using that card's first in-month date as the selected date.
- Day taps now request the month overlay for the tapped date, so adjacent overflow days route into their real month.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - Year calendar view must not render a calendar-system segmented control on mobile.
    - Header prev/today/next actions must continue to work after removing the selector row.
    - Tapping a month title must switch the content surface from year view to the corresponding month view.
    - Tapping an in-month or adjacent-month day must open the month view for that tapped date and preserve its selected
      ISO date.

## Collaborators
- `src/app/calendar/CalendarBoardStore.hpp/.cpp`
- `src/app/viewmodel/calendar/YearCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
