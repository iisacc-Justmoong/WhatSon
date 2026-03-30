# `src/app/viewmodel/calendar/MonthCalendarViewModel.hpp`

## Role
`MonthCalendarViewModel` exposes the month-level calendar grid state used by the editor overlay page. It is a
QObject-backed viewmodel designed for direct QML binding.

## Core Surface
- `displayedYear`, `displayedMonth`: active month cursor.
- `calendarSystem`, `calendarSystemName`: selected calendar backend and display label.
- `monthLabel`, `weekdayLabels`, `dayModels`: precomputed UI-ready month-grid payload.
- `selectedDateIso`, `selectedDateEntries`: active board selection and entries bound to that date.
- `calendarSystemOptions`: enum/label list for selector buttons.

## Calendar System Enum
- `Gregorian`
- `Julian`
- `IslamicCivil`
- `Custom`

The enum is exported with `Q_ENUM` so QML and meta-object reflection can consume stable values.

## Mutation and Hooks
- Slots:
  - `setDisplayedYear(int)`
  - `setDisplayedMonth(int)`
  - `setCalendarSystemByEnum(CalendarSystem)`
- Invokables:
  - `setCalendarSystemByValue(int)`
  - `shiftMonth(int delta)`
  - `requestMonthView(const QString& reason = QString())`
  - `addEvent(dateIso, timeText, title, detail)`
  - `addTask(dateIso, timeText, title, detail)`
  - `entriesForDate(dateIso)`
  - `removeEntry(entryId)`
  - `setTaskCompleted(entryId, completed)`

## Board Integration
- `setCalendarBoardStore(CalendarBoardStore*)` binds the shared board backend.
- `dayModels` now include board metadata per date cell:
  - `dateIso`
  - `eventCount`
  - `taskCount`
  - `entryCount`
- If `dateIso` is omitted for add APIs, the viewmodel falls back to `selectedDateIso`.

## Signals
- `displayedYearChanged()`
- `displayedMonthChanged()`
- `calendarSystemChanged()`
- `monthViewChanged()`
- `monthViewRequested(QString reason)`
- `selectedDateIsoChanged()`
- `selectedDateEntriesChanged()`

These signals form the contract for `MonthCalendarPage.qml` and overlay hosts.

## Internal Invariants
- Year range is clamped to `1..9999`.
- Month is clamped to the selected calendar system's available month count in the active year.
- `dayModels` is rebuilt as a fixed-size month grid suitable for deterministic UI layout.
