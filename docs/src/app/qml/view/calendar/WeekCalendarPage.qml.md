# `src/app/qml/view/calendar/WeekCalendarPage.qml`

## Role
`WeekCalendarPage.qml` renders the week calendar as an inline content page.
The week surface is horizontally pageable and supports bidirectional infinite scrolling with lazy week-window expansion.
Inside each page, the 7-day timeline is width-fitted to the viewport instead of exposing a second horizontal scroll
surface.

## View Contract
- Input: `weekCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `weekCalendarViewModel.requestWeekView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) only.
- Body:
  - horizontal `ListView` with `ListView.SnapOneItem`,
  - each page hosts one week timeline (24-hour rows x 7 day columns),
  - each page computes `hourRowHeight` from the available viewport height so 24 rows evenly fill the remaining area (`00:00` top slot, final slot at the bottom edge),
  - the 7 day columns now divide the remaining width after the hour column and the seven inter-column gaps, so the
    timeline width matches the page viewport on both desktop and mobile,
  - there is no inner horizontal `Flickable` for day-column panning.

## Interaction/Data Flow
1. `Component.onCompleted` initializes a centered week window (`initialWeekRadius`) and requests `page-open`.
2. Horizontal page changes update `displayedWeekStartIso`.
3. Near-edge access triggers lazy window growth:
   - left edge: `prependWeeks(...)`
   - right edge: `appendWeeks(...)`
4. Window size is bounded (`maxWeekWindowSize`) to avoid unbounded memory growth.
5. Hour-slot labels are derived from `entriesForDate(dateIso)` and compressed by `slotSummary(...)`.
6. Hour-slot entry chips are rendered by shared `CalendarEventCell` (default/colored background by entry type).
7. On mobile, horizontal swipe gestures now belong only to the outer week pager, so a left/right swipe always means
   previous/next week navigation rather than inner timeline panning.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - Weekly calendar content width must stay within the mobile viewport without horizontal overflow.
    - Left/right swipe in the weekly page must change the visible week only.
    - The weekly page must keep the same full-width content contract as `DayCalendarPage.qml`.

## Collaborators
- `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
