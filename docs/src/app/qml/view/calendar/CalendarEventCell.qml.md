# `src/app/qml/view/calendar/CalendarEventCell.qml`

## Role
`CalendarEventCell.qml` is the shared reusable event chip used by monthly, weekly, and daily calendar views.
It is derived from the monthly event chip design reference (Figma node `227:9429`) and generalized for
cross-view usage.

## Public QML Contract
- `property string label`
- `property int backgroundType` (`backgroundDefault` / `backgroundColored`)
- `property color defaultBackgroundColor`
- `property color coloredBackgroundColor`
- `property color textColor`
- `property int cornerRadius`
- `property int horizontalInset`
- `property int verticalInset`
- `property int labelPixelSize`
- `property int labelWeight`

## Render Rules
- Base size: `height: 16` (callers may override).
- Radius: `cornerRadius` (`4` default).
- Insets: `horizontalInset` (`8` default), `verticalInset` (`2` default).
- Font: `labelPixelSize` (`12`) and `labelWeight` (`Font.Medium`) by default.
- Background:
  - default: `defaultBackgroundColor` (`LV.Theme.panelBackground08`)
  - colored: `coloredBackgroundColor` (`LV.Theme.primary`)

## Collaborators
- `src/app/qml/view/calendar/MonthCalendarDayCell.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/DayCalendarPage.qml`
