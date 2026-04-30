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
- `property bool interactive`
- `signal activated`

## Render Rules
- Base size: `height` defaults through `LV.Theme.iconSm` (callers may override).
- Radius: `cornerRadius` defaults to `LV.Theme.radiusSm`.
- Insets: `horizontalInset` / `verticalInset` default to `LV.Theme.gap8` / `LV.Theme.gap2`.
- Font: `labelPixelSize` defaults through `LV.Theme.textBody` and `labelWeight` remains `Font.Medium`.
- Text color defaults to `LV.Theme.bodyColor`.
- Background:
  - default: `defaultBackgroundColor` (`LV.Theme.panelBackground08`)
  - colored: `coloredBackgroundColor` (`LV.Theme.primary`)
- Interaction:
  - an internal `TapHandler` emits `activated()` only when `interactive == true`
  - the default contract stays passive so non-note chips do not become clickable accidentally

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - Passive event chips must remain non-clickable by default.
    - An interactive note chip must emit `activated()` on both mouse click and touch tap.

## Collaborators
- `src/app/qml/view/calendar/MonthCalendarDayCell.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/DayCalendarPage.qml`
