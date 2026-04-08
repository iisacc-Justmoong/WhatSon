# `src/app/qml/view/calendar/MonthCalendarDayCell.qml`

## Role
`MonthCalendarDayCell.qml` is the reusable monthly day-cell component for Figma node `227:9427`.
It supports two variants through one boolean argument:
- `disable: false` for in-month date cells
- `disable: true` for adjacent-month overflow dates shown in the month grid

## Public QML Contract
- `property int dayNumber`
- `property bool disable`
- `property bool selected`
- `property bool today`
- `property int maxVisibleEntries`
- `property var entryCells`
- `signal clicked`

## Render Rules
- Background: `LV.Theme.panelBackground04`
- Disabled variant: `opacity: 0.5`
- Selected state: applies an accent border using `LV.Theme.accent` and `LV.Theme.strokeThin`
- Today state: when the cell is not the selected date, applies a soft border using `LV.Theme.strokeSoft` and
  `LV.Theme.strokeThin`
- Content padding: `LV.Theme.gap8`
- Day number and overflow label typography use `LV.Theme.scaleMetric(12)`.
- Day-label gap, event-row height, and event-row spacing use `LV.Theme.scaleMetric(10/16)` and `LV.Theme.gap2`.
- Entry chips delegate to shared `CalendarEventCell` with `cornerRadius: LV.Theme.radiusSm` in month-grid usage.
- Overflow state: renders `+N more` when visible entry capacity is exceeded
- The parent grid may now pass note/event chip payloads that already originated from `dayModel.entries` inside the
  month projection, so visible calendar items stay aligned with the rebuilt month snapshot.

## Interaction
1. Whole cell is clickable through an internal `MouseArea`.
2. Parent page handles `clicked` and updates selected date in `MonthCalendarViewModel`.

## Collaborators
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - The selected date cell must render with an accent border even when it is not today.
    - When today is also selected, the accent border must win over the soft today border.
    - The day cell mapped from `dayModel.isToday === true` must still render with a visible but low-contrast border
      when it is not the selected date.
    - Adjacent non-today, non-selected overflow dates must continue to render without the added border.
