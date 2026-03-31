# `src/app/qml/view/calendar/WeekCalendarPage.qml`

## Role
`WeekCalendarPage.qml` renders the week calendar as an inline content page.
The week surface is horizontally pageable and supports bidirectional infinite scrolling with lazy week-window expansion.

## View Contract
- Input: `weekCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `weekCalendarViewModel.requestWeekView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) maps to horizontal week paging,
  - active week-range label mirrors the snapped page.
- Body:
  - horizontal `ListView` with `ListView.SnapOneItem`,
  - each page hosts one week timeline (24-hour rows x 7 day columns),
  - each week page has its own vertical `Flickable` for the hour grid.
  - mobile mode (`LV.Theme.mobileTarget`) enables horizontal date scrolling inside the week timeline by enforcing a
    minimum day-column width (`mobileMinimumDayColumnWidth`), so dates are browsed with left/right gestures instead of
    squeezing 7 day columns into one viewport.

## Interaction/Data Flow
1. `Component.onCompleted` initializes a centered week window (`initialWeekRadius`) and requests `page-open`.
2. Horizontal page changes update `displayedWeekStartIso`.
3. Near-edge access triggers lazy window growth:
   - left edge: `prependWeeks(...)`
   - right edge: `appendWeeks(...)`
4. Window size is bounded (`maxWeekWindowSize`) to avoid unbounded memory growth.
5. Hour-slot labels are derived from `entriesForDate(dateIso)` and compressed by `slotSummary(...)`.
6. On mobile, horizontal gestures prioritize date-column scrolling inside the active week page (`Flickable.HorizontalAndVerticalFlick`).

## Collaborators
- `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
