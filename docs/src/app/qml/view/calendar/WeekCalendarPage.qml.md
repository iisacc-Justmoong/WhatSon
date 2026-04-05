# `src/app/qml/view/calendar/WeekCalendarPage.qml`

## Role
`WeekCalendarPage.qml` renders the week calendar as a three-column inline timeline surface.
The week surface now keeps one continuous time scaffold: the left `Time` header and the 24 hourly labels stay fixed,
while only the day columns on the right move horizontally. The visible viewport is still sized for three day columns,
but horizontal flicking advances across adjacent dates without page snapping.

## View Contract
- Input: `weekCalendarViewModel`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `weekCalendarViewModel.requestWeekView(reason)`

## UI Composition
- Surface:
  - root page keeps `color: "transparent"` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) only.
  - `Prev/Next` still recenters by week from the currently focused middle date, while horizontal flicking handles the day-by-day traversal.
  - `Today` recenters the surface so the actual current date lands in the middle visible day column.
- Body:
  - one fixed left time scaffold (`Time` header + 24 hour labels),
  - one horizontal `Flickable` on the right that owns only the date columns,
  - the visible viewport is sized for three day columns at a time,
  - the `ListModel` stores only per-date metadata; per-date entry lists are resolved through `entriesForDate(dateIso)` so
    the surface does not push list-valued roles through the model,
  - a dedicated day-header row stays aligned with the hourly grid because both live in the same horizontal content surface,
  - generic day-header and hour cells stay transparent regardless of current-week membership, so the week surface does
    not draw per-column grid fills,
  - current-week emphasis is now limited to text tone and the existing today outline,
  - the scaffold computes `hourRowHeight` from the available viewport height so 24 rows evenly fill the remaining area (`00:00` top slot, final slot at the bottom edge),
  - the 3 day columns divide the remaining width after the hour column and the three inter-column gaps, so the
    timeline width matches the page viewport on both desktop and mobile,
  - horizontal scrolling has no snap behavior.

## Interaction/Data Flow
1. `Component.onCompleted` initializes a centered lazy date window and requests `page-open`.
2. Horizontal movement updates only the right-side date-column surface; the time scaffold does not move.
3. Near-edge access triggers lazy date growth:
   - left edge: `prependDates(...)`
   - right edge: `appendDates(...)`
4. Adjacent content advances one day at a time, so the user can traverse `1-3`, `2-4`, `3-5`, and so on.
5. Date-window size is bounded (`maxDateWindowSize`) to avoid unbounded memory growth.
6. Programmatic recentering (`Prev`, `Next`, `Today`) aligns the target date to the middle column before syncing the week anchor.
7. Horizontal movement end normalizes the currently focused middle date back to its Monday-start week and syncs `displayedWeekStartIso`.
8. Per-date entry lookup is cached by ISO date and invalidated when the week view refreshes.
9. Hour-slot labels are derived from `entriesForDate(dateIso)` and compressed by `slotSummary(...)`.
10. Hour-slot entry chips are rendered by shared `CalendarEventCell` (default/colored background by entry type).

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
    - The weekly page must keep the `Time` header and 24 hour labels fixed while only the day columns move horizontally.
    - Weekly calendar content width must stay within the viewport while showing exactly three day columns at rest.
    - Horizontal scrolling must move continuously without page snapping.
    - Initial page-open and week refresh must not emit `ListModel` role-type warnings for `entries`.
    - Clicking `Today` must not introduce visible borders on generic current-week header/hour cells.
    - Clicking `Today` must place the actual current date in the middle visible day column.
    - Generic header and hour cells must stay transparent for every date column.
    - The weekly page must render a dedicated day-header row above the hourly grid.
    - Today must remain visually distinguished in the day-header row.

## Collaborators
- `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
