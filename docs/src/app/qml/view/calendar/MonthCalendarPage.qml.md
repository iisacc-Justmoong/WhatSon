# `src/app/qml/view/calendar/MonthCalendarPage.qml`

## Role

`MonthCalendarPage.qml` renders the month calendar overlay shell for the editor area.
It owns the shared month header (`CalendarTodayControl` + title) and, on mobile, exposes a horizontal paged month
surface so left/right swipes move to the previous or next month.

## View Contract

- Input: `monthCalendarViewModel`
- Output signal: `noteOpenRequested(string noteId)`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `monthCalendarViewModel.requestMonthView(...)`

## Pager Model

- The page keeps a 3-slot projection window:
    - previous month
    - current displayed month
    - next month
- Those projections now come from `MonthCalendarViewModel.pagerMonthModels`.
- The `ListView` pager now binds delegates by numeric index and resolves the live projection through
  `monthProjectionForIndex(...)`.
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

1. `Component.onCompleted` recenters the already-precomputed pager and requests `page-open`.
2. Header prev/next still call `shiftMonth(-1|1)`.
3. Mobile swipe completion also resolves to `shiftMonth(-1|1)`.
4. `monthViewChanged` now only recenters the pager because the shared `MonthCalendarViewModel` already owns the updated
   `pagerMonthModels`.
5. Day selection is still delegated back into `monthCalendarViewModel.setSelectedDateIso(...)`.
6. When opened from `YearCalendarPage.qml`, the host now preloads `displayedYear`, `displayedMonth`, and
   `selectedDateIso` before this page becomes visible, so the routed month opens on the requested month/date.
7. Note-chip taps from `MonthCalendarGridSurface.qml` bubble upward as `noteOpenRequested(noteId)` so the host can
   reopen the selected note in the editor.
8. `visible` re-entry also recenters the pager, so reopening the month overlay cannot leave the grid parked on the
   previous-month page while the header already points at the current month.

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
    - Initial month-page open must not show the previous-month grid while the header already displays the current
      month title.
    - Clicking or tapping a visible month note chip must bubble `noteOpenRequested(...)` out of the page.
