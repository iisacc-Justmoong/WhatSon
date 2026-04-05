# `src/app/qml/view/calendar/MonthCalendarPage.qml`

## Role

`MonthCalendarPage.qml` renders the month calendar overlay shell for the editor area.
It owns the shared month header (`CalendarTodayControl` + title) and, on mobile, exposes a horizontal paged month
surface so left/right swipes move to the previous or next month.

## View Contract

- Input: `monthCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `monthCalendarViewModel.requestMonthView(...)`

## Pager Model

- The page keeps a 3-slot projection window:
    - previous month
    - current displayed month
    - next month
- Those projections come from `MonthCalendarViewModel.monthProjectionFor(year, month)`.
- After a swipe completes, `commitMonthSwipeDelta(...)` shifts the canonical displayed month in the viewmodel and then
  recenters the local pager back to the middle slot.
- Desktop keeps the same shell but disables interactive swipe paging; header controls still drive month changes.

## UI Composition

- Header:
  - header height resolves through `LV.Theme.scaleMetric(54)` (`monthHeaderHeight`)
  - left: active month title (`monthLabel, displayedYear`)
  - right: shared `CalendarTodayControl`
- Body:
    - horizontal `ListView` with `ListView.SnapOneItem`
    - each page fills the calendar body viewport
    - each page renders one `MonthCalendarGridSurface.qml`
- Header/body typography and weekday/header paddings now route through `LV.Theme.scaleMetric(...)` and `LV.Theme.gap8/12`
  instead of raw `12/39/54px` literals.

## Interaction/Data Flow

1. `Component.onCompleted` rebuilds the 3-slot pager and requests `page-open`.
2. Header prev/next still call `shiftMonth(-1|1)`.
3. Mobile swipe completion also resolves to `shiftMonth(-1|1)`.
4. `monthViewChanged` rebuilds pager projections so entry counts and month labels stay in sync with the shared
   `MonthCalendarViewModel`.
5. Day selection is still delegated back into `monthCalendarViewModel.setSelectedDateIso(...)`.
6. When opened from `YearCalendarPage.qml`, the host now preloads `displayedYear`, `displayedMonth`, and
   `selectedDateIso` before this page becomes visible, so the routed month opens on the requested month/date.

## Collaborators
- `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/MonthCalendarGridSurface.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - On mobile, monthly view must accept left/right swipe navigation between adjacent months.
    - Swipe completion must update the canonical displayed month, not leave the pager parked on a side slot.
    - Desktop month view must remain width-fitted without enabling unintended horizontal scrolling.
    - Day selection inside any visible month page must still update `selectedDateIso`.
    - Opening the page from a year-calendar month/day tap must show the requested month immediately instead of the
      previously displayed month.
