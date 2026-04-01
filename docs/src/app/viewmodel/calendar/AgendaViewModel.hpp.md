# `src/app/viewmodel/calendar/AgendaViewModel.hpp`

## Role
`AgendaViewModel` exposes an Agenda-focused day projection for the calendar surface. It is a QObject-backed viewmodel
that keeps Agenda sections and weather/location cards in one MVVM contract.

## Core Surface
- `displayedDateIso`: active date cursor (`yyyy-MM-dd`).
- `dateLabel`: localized Agenda header label.
- `location`: location object (`cityName`, `regionName`, `countryCode`, `timeZoneId`, `displayName`).
- `weather`: weather object (`conditionText`, `temperatureText`, `highLowText`, `precipitationText`, numeric fields).
- `allDayEvents`: event list where time is `00:00`.
- `timedEvents`: event list with explicit time slots.
- `agendaItems`: agenda-item list with completion/status metadata.
- `summary`: aggregate counters for all sections.

## Mutation and Hooks
- Slot:
  - `setDisplayedDateIso(const QString&)`
- Invokables:
  - `shiftDay(int deltaDays)`
  - `requestAgendaView(const QString& reason = QString())`
  - `addEvent(dateIso, timeText, title, detail)`
  - `addAgendaItem(dateIso, timeText, title, detail)`
  - `entriesForDate(dateIso)`
  - `removeEntry(entryId)`
  - `setAgendaItemCompleted(entryId, completed)`
  - `toggleAgendaItemCompleted(entryId)`
  - `setLocation(cityName, regionName, countryCode, timeZoneId)`
  - `setWeather(conditionText, temperatureCelsius, highCelsius, lowCelsius, precipitationPercent)`

## Board Integration
- `setCalendarBoardStore(CalendarBoardStore*)` binds the shared in-memory board backend.
- Board updates (`entriesChanged`) trigger Agenda section rebuild.
- `addEvent` / `addAgendaItem` use `displayedDateIso` when date input is empty.

## Signals
- `displayedDateIsoChanged()`
- `agendaChanged()`
- `agendaRequested(QString reason)`

These signals form the contract consumed by `AgendaPage.qml` and overlay hosts.
