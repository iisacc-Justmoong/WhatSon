# `src/app/qml/view/calendar/WeekCalendarPage.qml`

## Role
`WeekCalendarPage.qml` renders the week calendar as a three-column inline timeline surface.
The week surface now keeps one continuous time scaffold: the left `Time` header and the 24 hourly labels stay fixed,
while only the day columns on the right move horizontally. The visible viewport is still sized for three day columns,
but horizontal flicking advances across adjacent dates without page snapping.
The page no longer owns a second date-model/cache layer; it only manages viewport movement and delegates timeline data
ownership to `WeekCalendarController`.

## View Contract
- Input: `weekCalendarController`
- Output signal: `noteOpenRequested(string noteId)`
- Hook signal: `viewHookRequested(string reason)`
- Hook forwarder: `requestViewHook(reason)` delegates to `weekCalendarController.requestWeekView(reason)`

## UI Composition
- Surface:
  - root page keeps `LV.Theme.accentTransparent` so the app background shows through.
- Header:
  - shared `CalendarTodayControl` (`Prev/Today/Next`) only.
  - `Prev/Next` still recenters by week from the currently focused middle date, while horizontal flicking handles the day-by-day traversal.
  - `Today` recenters the surface so the actual current date lands in the middle visible day column.
- Body:
  - one fixed left time scaffold (`Time` header + 24 hour labels),
  - one horizontal `Flickable` on the right that owns only the date columns,
  - the visible viewport is sized for three day columns at a time,
  - the date-column model now comes directly from `WeekCalendarController.timelineDayModels`,
  - each day model already includes per-date entry lists and derived metadata, so the page no longer keeps a local
    `ListModel`, `dateEntriesCache`, or `buildDateModel(...)` function,
  - a dedicated day-header row stays aligned with the hourly grid because both live in the same horizontal content surface,
  - generic day-header and hour cells stay transparent through `LV.Theme.accentTransparent` regardless of current-week
    membership, so the week surface does not draw per-column grid fills,
  - current-week emphasis is now limited to text tone and the existing today outline,
  - the scaffold computes `hourRowHeight` from the available viewport height so 24 rows evenly fill the remaining area (`00:00` top slot, final slot at the bottom edge),
  - the 3 day columns divide the remaining width after the hour column and the three inter-column gaps, so the
    timeline width matches the page viewport on both desktop and mobile,
  - horizontal scrolling has no snap behavior.
- The centered-day highlight radius now routes through `LV.Theme.gap20 + LV.Theme.strokeThin` instead of a local pixel constant.

## Interaction/Data Flow
1. `Component.onCompleted` initializes a centered lazy date window and, when opening the current week, uses the actual
   current date as the initial focused middle column instead of the week-start Monday anchor.
2. Horizontal movement updates only the right-side date-column surface; the time scaffold does not move.
3. Near-edge access triggers lazy date growth:
   - left edge: `prependDates(...)` -> `WeekCalendarController.prependTimelineDates(...)`
   - right edge: `appendDates(...)` -> `WeekCalendarController.appendTimelineDates(...)`
4. Adjacent content advances one day at a time, so the user can traverse `1-3`, `2-4`, `3-5`, and so on.
5. Date-window size is bounded (`maxDateWindowSize`) through `WeekCalendarController.trimTimelineWindow(...)` to avoid
   unbounded memory growth.
6. Programmatic recentering (`Prev`, `Next`, `Today`) aligns the target date to the middle column before syncing the week anchor.
7. Horizontal movement end normalizes the currently focused middle date back to its Monday-start week and syncs `displayedWeekStartIso`.
8. Hour-slot labels are derived from the controller-owned day-model `entries` payload and compressed by `slotSummary(...)`.
9. Hour-slot entry chips are rendered by shared `CalendarEventCell` (default/colored background by entry type).
10. Direct note opening is enabled only when a visible slot chip represents exactly one projected note entry. The
    compressed `title +N` chip remains passive because it does not expose a unique note target.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - The weekly page must keep the `Time` header and 24 hour labels fixed while only the day columns move horizontally.
  - Weekly calendar content width must stay within the viewport while showing exactly three day columns at rest.
  - Horizontal scrolling must move continuously without page snapping.
  - The week page must not maintain a second local `ListModel` or per-date entry cache on top of the controller data.
  - Initial page-open for the current week must center the actual current date instead of the week-start Monday.
  - Clicking `Today` must not introduce visible borders on generic current-week header/hour cells.
  - Clicking `Today` must place the actual current date in the middle visible day column.
  - Generic header and hour cells must stay transparent for every date column.
  - The weekly page must render a dedicated day-header row above the hourly grid.
  - Today must remain visually distinguished in the day-header row.
  - A week-slot chip backed by exactly one note entry must emit `noteOpenRequested(...)` on click/tap.
  - A compressed multi-entry week-slot chip must remain non-openable until the slot UI exposes per-entry hit targets.

## Collaborators
- `src/app/models/calendar/WeekCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
