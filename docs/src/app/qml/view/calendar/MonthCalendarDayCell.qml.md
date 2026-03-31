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
- Content padding: `8`
- Day number label style: `12 / Medium` (Body typography)
- Entry chips: delegated to shared `CalendarEventCell` with `cornerRadius: 4` in month-grid usage
- Overflow state: renders `+N more` when visible entry capacity is exceeded

## Interaction
1. Whole cell is clickable through an internal `MouseArea`.
2. Parent page handles `clicked` and updates selected date in `MonthCalendarViewModel`.

## Collaborators
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
