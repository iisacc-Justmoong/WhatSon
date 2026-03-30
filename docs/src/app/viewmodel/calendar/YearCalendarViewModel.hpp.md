# `src/app/viewmodel/calendar/YearCalendarViewModel.hpp`

## Role
`YearCalendarViewModel` exposes the year-grid calendar surface used by the content overlay and now supports board-style
event/task integration through a shared calendar backend.

## Core Surface
- `displayedYear`
- `calendarSystem`, `calendarSystemName`
- `weekdayLabels`
- `monthModels`
- `calendarSystemOptions`

## Mutation and Hooks
- `setDisplayedYear(int)`
- `setCalendarSystemByEnum(CalendarSystem)`
- `setCalendarSystemByValue(int)`
- `shiftYear(int delta)`
- `requestYearView(reason)`

## Board API
- `setCalendarBoardStore(CalendarBoardStore*)`
- `addEvent(dateIso, timeText, title, detail)`
- `addTask(dateIso, timeText, title, detail)`
- `entriesForDate(dateIso)`
- `removeEntry(entryId)`
- `setTaskCompleted(entryId, completed)`

## Day Cell Contract
Each day cell in `monthModels[*].days[*]` includes:
- `day`, `month`, `year`
- `dateIso`
- `inCurrentMonth`
- `isToday`
- `eventCount`, `taskCount`, `entryCount`

This allows year cards to act as board summaries, not only passive date viewers.

## Signals
- `displayedYearChanged()`
- `calendarSystemChanged()`
- `yearViewChanged()`
- `yearViewRequested(QString reason)`
