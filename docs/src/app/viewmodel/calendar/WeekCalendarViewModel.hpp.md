# `src/app/viewmodel/calendar/WeekCalendarViewModel.hpp`

## Role
`WeekCalendarViewModel` exposes both the canonical 7-day week projection and the lazy horizontal timeline date window
used by the week page.

## Interface Alignment
- Calendar board injection now depends on `ICalendarBoardStore`.
- `displayedWeekStartIso` remains the canonical week anchor even though the QML surface now renders three visible day
  columns from one continuous horizontal scaffold.
- The viewmodel now owns that scaffold's timeline-day model window through `timelineDayModels` plus incremental
  window-mutation methods (`initializeTimelineWindow`, `prependTimelineDates`, `appendTimelineDates`,
  `trimTimelineWindow`), so QML no longer maintains a second ad-hoc date model hierarchy.
- The weekly anchor is now synchronized from the focused middle date in the viewport, not from the left edge of the
  three-column surface.
- The current-week visual emphasis in QML is computed against a Monday-start week, matching the same weekly anchoring
  expectation used by the weekly route.
